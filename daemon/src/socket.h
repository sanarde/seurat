/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_SOCKET_H
#define SEURATD_SOCKET_H

#include <limits.h>
#include <stdint.h>

#include "loop.h"
#include "state.h"

#define SEURATD_MAX_CLIENTS 16

struct seuratd_client {
    struct seuratd_source source;
    int fd;
};

struct seuratd_socket {
    struct seuratd_source source;
    int fd;
    int epfd;
    char path[PATH_MAX];
    struct seuratd_client clients[SEURATD_MAX_CLIENTS];
    char text[SEURATD_TEXT_MAX];
    int text_len;
};

int seuratd_socket_open(struct seuratd_socket *sock, int epfd,
                        const char *path);

void seuratd_socket_close(struct seuratd_socket *sock);

void seuratd_socket_accept(struct seuratd_socket *sock);

void seuratd_socket_client_event(struct seuratd_socket *sock,
                                 struct seuratd_client *client,
                                 uint32_t events);

void seuratd_socket_publish(struct seuratd_socket *sock,
                            const struct seuratd_state *state);

#endif
