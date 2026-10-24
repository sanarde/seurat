/* SPDX-License-Identifier: GPL-3.0-only */
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "bus.h"
#include "config.h"
#include "loop.h"
#include "seurat/source.h"
#include "socket.h"
#include "state.h"

static struct seuratd_socket socket_server;

static int open_signal_fd(void)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) != 0)
        return -1;
    return signalfd(-1, &mask, SFD_CLOEXEC);
}

static int open_config_watch(const char *path, char *name, size_t name_size)
{
    char dir_copy[PATH_MAX];
    char name_copy[PATH_MAX];
    int fd;

    if (snprintf(dir_copy, sizeof(dir_copy), "%s", path) >= (int)sizeof(dir_copy))
        return -1;
    strcpy(name_copy, dir_copy);
    if (snprintf(name, name_size, "%s", basename(name_copy)) >= (int)name_size)
        return -1;

    fd = inotify_init1(IN_CLOEXEC);
    if (fd < 0)
        return -1;
    if (inotify_add_watch(fd, dirname(dir_copy),
                          IN_CLOSE_WRITE | IN_MOVED_TO | IN_MOVED_FROM |
                              IN_DELETE) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static int config_changed(int fd, const char *name)
{
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len = read(fd, buf, sizeof(buf));
    int changed = 0;

    for (char *p = buf; len > 0 && p < buf + len;) {
        const struct inotify_event *ev = (const struct inotify_event *)p;

        if (ev->len > 0 && strcmp(ev->name, name) == 0)
            changed = 1;
        p += sizeof(*ev) + ev->len;
    }
    return changed;
}

static void apply_config(const struct seuratd_config *config,
                         struct seuratd_state *state, struct seuratd_bus *bus,
                         int publish_always)
{
    struct seuratd_state next;
    uint32_t source =
        config->has_color ? config->color : SEURAT_FALLBACK_SOURCE_COLOR;

    seuratd_state_compute(&next, source, config->color_scheme);
    if (!publish_always && next.source == state->source &&
        next.dark == state->dark)
        return;

    *state = next;
    if (socket_server.fd >= 0)
        seuratd_socket_publish(&socket_server, state);
    seuratd_bus_emit_changed(bus);
}

static void reload_config(const char *path, struct seuratd_config *config,
                          struct seuratd_state *state, struct seuratd_bus *bus)
{
    struct seuratd_config loaded;

    if (seuratd_config_load(path, &loaded) != 0) {
        fprintf(stderr, "seuratd: keeping previous configuration\n");
        return;
    }
    *config = loaded;
    apply_config(config, state, bus, 0);
}

static int add_epoll_source(int epfd, int fd, struct seuratd_source *source)
{
    struct epoll_event ev = {.events = EPOLLIN, .data.ptr = source};

    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

static int socket_path(char *buf, size_t size)
{
    const char *runtime = getenv("XDG_RUNTIME_DIR");

    if (!runtime || !*runtime)
        return -1;
    return snprintf(buf, size, "%s/seurat.sock", runtime) >= (int)size ? -1 : 0;
}

int main(int argc, char **argv)
{
    char config_path[PATH_MAX];
    char config_name[NAME_MAX + 1];
    char sock_path[PATH_MAX];
    struct seuratd_source signal_source = {SEURATD_SRC_SIGNAL};
    struct seuratd_source config_source = {SEURATD_SRC_CONFIG};
    struct seuratd_config config;
    struct seuratd_state state;
    struct seuratd_bus bus = {0};
    int sig_fd;
    int watch_fd;
    int epfd;

    if (argc == 3 && strcmp(argv[1], "-c") == 0) {
        if (snprintf(config_path, sizeof(config_path), "%s", argv[2]) >=
            (int)sizeof(config_path)) {
            fprintf(stderr, "seuratd: configuration path is too long\n");
            return 1;
        }
    } else if (argc == 1) {
        if (seuratd_config_default_path(config_path, sizeof(config_path)) != 0) {
            fprintf(stderr, "seuratd: cannot determine configuration path\n");
            return 1;
        }
    } else {
        fprintf(stderr, "usage: seuratd [-c config]\n");
        return 2;
    }

    if (seuratd_config_load(config_path, &config) != 0)
        return 1;
    if (!config.enabled) {
        fprintf(stderr, "seuratd: disabled in configuration\n");
        return 0;
    }

    sig_fd = open_signal_fd();
    if (sig_fd < 0) {
        fprintf(stderr, "seuratd: signalfd: %s\n", strerror(errno));
        return 1;
    }

    watch_fd = open_config_watch(config_path, config_name, sizeof(config_name));
    if (watch_fd < 0)
        fprintf(stderr,
                "seuratd: cannot watch %s, reload only on SIGHUP\n", config_path);

    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd < 0 || add_epoll_source(epfd, sig_fd, &signal_source) != 0 ||
        (watch_fd >= 0 &&
         add_epoll_source(epfd, watch_fd, &config_source) != 0)) {
        fprintf(stderr, "seuratd: epoll: %s\n", strerror(errno));
        return 1;
    }

    socket_server.fd = -1;
    seuratd_state_compute(&state,
                          config.has_color ? config.color
                                           : SEURAT_FALLBACK_SOURCE_COLOR,
                          config.color_scheme);

    if (socket_path(sock_path, sizeof(sock_path)) != 0) {
        fprintf(stderr, "seuratd: XDG_RUNTIME_DIR is not usable, no socket\n");
    } else if (seuratd_socket_open(&socket_server, epfd, sock_path) != 0) {
        fprintf(stderr, "seuratd: socket %s: %s\n", sock_path, strerror(errno));
    } else {
        seuratd_socket_publish(&socket_server, &state);
    }

    if (seuratd_bus_open(&bus, epfd, &state) != 0 && socket_server.fd < 0) {
        fprintf(stderr, "seuratd: no distribution channel available\n");
        return 1;
    }

    for (;;) {
        struct epoll_event event;
        int n = epoll_wait(epfd, &event, 1, -1);
        struct seuratd_source *source = event.data.ptr;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "seuratd: epoll_wait: %s\n", strerror(errno));
            break;
        }

        if (source->kind == SEURATD_SRC_SIGNAL) {
            struct signalfd_siginfo info;

            if (read(sig_fd, &info, sizeof(info)) != sizeof(info))
                continue;
            if (info.ssi_signo != SIGHUP)
                break;
            reload_config(config_path, &config, &state, &bus);
        } else if (source->kind == SEURATD_SRC_CONFIG) {
            if (config_changed(watch_fd, config_name))
                reload_config(config_path, &config, &state, &bus);
        } else if (source->kind == SEURATD_SRC_LISTEN) {
            seuratd_socket_accept(&socket_server);
        } else if (source->kind == SEURATD_SRC_CLIENT) {
            seuratd_socket_client_event(&socket_server,
                                        (struct seuratd_client *)source,
                                        event.events);
        } else if (source->kind == SEURATD_SRC_BUS) {
            seuratd_bus_event(&bus, (struct seuratd_bus_watch *)source,
                              event.events);
        }

        if (!config.enabled) {
            fprintf(stderr, "seuratd: disabled in configuration\n");
            break;
        }
    }

    seuratd_bus_close(&bus);
    seuratd_socket_close(&socket_server);
    return 0;
}
