/* SPDX-License-Identifier: GPL-3.0-only */
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "config.h"

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

static void reload_config(const char *path, struct seuratd_config *config)
{
    struct seuratd_config loaded;

    if (seuratd_config_load(path, &loaded) != 0) {
        fprintf(stderr, "seuratd: keeping previous configuration\n");
        return;
    }
    *config = loaded;
}

static int add_epoll_fd(int epfd, int fd)
{
    struct epoll_event ev = {.events = EPOLLIN, .data.fd = fd};

    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int main(int argc, char **argv)
{
    char config_path[PATH_MAX];
    char config_name[NAME_MAX + 1];
    struct seuratd_config config;
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
    if (epfd < 0 || add_epoll_fd(epfd, sig_fd) != 0 ||
        (watch_fd >= 0 && add_epoll_fd(epfd, watch_fd) != 0)) {
        fprintf(stderr, "seuratd: epoll: %s\n", strerror(errno));
        return 1;
    }

    for (;;) {
        struct epoll_event events[2];
        int n = epoll_wait(epfd, events, 2, -1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "seuratd: epoll_wait: %s\n", strerror(errno));
            return 1;
        }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == sig_fd) {
                struct signalfd_siginfo info;

                if (read(sig_fd, &info, sizeof(info)) != sizeof(info))
                    continue;
                if (info.ssi_signo == SIGHUP)
                    reload_config(config_path, &config);
                else
                    return 0;
            } else if (config_changed(watch_fd, config_name)) {
                reload_config(config_path, &config);
            }
        }

        if (!config.enabled) {
            fprintf(stderr, "seuratd: disabled in configuration\n");
            return 0;
        }
    }
}
