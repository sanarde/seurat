/* SPDX-License-Identifier: GPL-3.0-only */
#include "socket.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int bind_socket(int fd, const struct sockaddr_un *addr)
{
    if (bind(fd, (const struct sockaddr *)addr, sizeof(*addr)) == 0)
        return 0;
    if (errno != EADDRINUSE)
        return -1;

    int probe = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    int alive;

    if (probe < 0)
        return -1;
    alive = connect(probe, (const struct sockaddr *)addr, sizeof(*addr)) == 0;
    close(probe);
    if (alive) {
        errno = EADDRINUSE;
        return -1;
    }

    unlink(addr->sun_path);
    return bind(fd, (const struct sockaddr *)addr, sizeof(*addr));
}

static void drop_client(struct seuratd_socket *sock,
                        struct seuratd_client *client)
{
    epoll_ctl(sock->epfd, EPOLL_CTL_DEL, client->fd, NULL);
    close(client->fd);
    client->fd = -1;
}

static int send_text(struct seuratd_socket *sock, struct seuratd_client *client)
{
    ssize_t n = send(client->fd, sock->text, (size_t)sock->text_len,
                     MSG_NOSIGNAL);

    return n == sock->text_len ? 0 : -1;
}

int seuratd_socket_open(struct seuratd_socket *sock, int epfd,
                        const char *path)
{
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    struct epoll_event ev = {.events = EPOLLIN, .data.ptr = &sock->source};

    if (strlen(path) >= sizeof(addr.sun_path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(addr.sun_path, path);

    sock->source.kind = SEURATD_SRC_LISTEN;
    sock->epfd = epfd;
    sock->text_len = 0;
    snprintf(sock->path, sizeof(sock->path), "%s", path);
    for (int i = 0; i < SEURATD_MAX_CLIENTS; i++) {
        sock->clients[i].source.kind = SEURATD_SRC_CLIENT;
        sock->clients[i].fd = -1;
    }

    sock->fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sock->fd < 0)
        return -1;
    if (bind_socket(sock->fd, &addr) != 0 || listen(sock->fd, 8) != 0 ||
        epoll_ctl(epfd, EPOLL_CTL_ADD, sock->fd, &ev) != 0) {
        int saved = errno;

        close(sock->fd);
        sock->fd = -1;
        errno = saved;
        return -1;
    }
    return 0;
}

void seuratd_socket_close(struct seuratd_socket *sock)
{
    if (sock->fd < 0)
        return;
    for (int i = 0; i < SEURATD_MAX_CLIENTS; i++) {
        if (sock->clients[i].fd >= 0)
            drop_client(sock, &sock->clients[i]);
    }
    close(sock->fd);
    sock->fd = -1;
    unlink(sock->path);
}

void seuratd_socket_accept(struct seuratd_socket *sock)
{
    int fd = accept4(sock->fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
    struct seuratd_client *client = NULL;
    struct epoll_event ev;

    if (fd < 0)
        return;

    for (int i = 0; i < SEURATD_MAX_CLIENTS; i++) {
        if (sock->clients[i].fd < 0) {
            client = &sock->clients[i];
            break;
        }
    }
    if (!client) {
        close(fd);
        return;
    }

    client->fd = fd;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = &client->source;
    if (epoll_ctl(sock->epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        close(fd);
        client->fd = -1;
        return;
    }
    if (sock->text_len > 0 && send_text(sock, client) != 0)
        drop_client(sock, client);
}

void seuratd_socket_client_event(struct seuratd_socket *sock,
                                 struct seuratd_client *client,
                                 uint32_t events)
{
    char scratch[64];

    if (client->fd < 0)
        return;
    if (events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
        drop_client(sock, client);
        return;
    }
    if (read(client->fd, scratch, sizeof(scratch)) == 0)
        drop_client(sock, client);
}

void seuratd_socket_publish(struct seuratd_socket *sock,
                            const struct seuratd_state *state)
{
    int len = seuratd_state_format(state, sock->text, sizeof(sock->text));

    sock->text_len = len > 0 ? len : 0;
    if (sock->text_len == 0)
        return;

    for (int i = 0; i < SEURATD_MAX_CLIENTS; i++) {
        if (sock->clients[i].fd >= 0 && send_text(sock, &sock->clients[i]) != 0)
            drop_client(sock, &sock->clients[i]);
    }
}
