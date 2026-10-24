/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "check.h"
#include "socket.h"

static int connect_to(const char *path)
{
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    strcpy(addr.sun_path, path);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static int read_block(int fd, char *buf, size_t size)
{
    size_t len = 0;

    while (len + 1 < size) {
        ssize_t n = read(fd, buf + len, size - 1 - len);

        if (n <= 0)
            return -1;
        len += (size_t)n;
        buf[len] = '\0';
        if (len >= 4 && strcmp(buf + len - 4, "end\n") == 0)
            return (int)len;
    }
    return -1;
}

int main(void)
{
    static struct seuratd_socket sock;
    static struct seuratd_socket second;
    struct seuratd_state state;
    char dir[] = "/tmp/seurat-test-XXXXXX";
    char path[128];
    char buf[SEURATD_TEXT_MAX];
    int epfd = epoll_create1(EPOLL_CLOEXEC);
    int a, b;

    CHECK(mkdtemp(dir) != NULL);
    snprintf(path, sizeof(path), "%s/seurat.sock", dir);

    seuratd_state_compute(&state, 0xff6750a4, SEURATD_SCHEME_DEFAULT);
    CHECK(seuratd_socket_open(&sock, epfd, path) == 0);
    seuratd_socket_publish(&sock, &state);

    CHECK(seuratd_socket_open(&second, epfd, path) != 0);
    seuratd_socket_accept(&sock);
    CHECK(sock.clients[0].fd < 0);

    a = connect_to(path);
    CHECK(a >= 0);
    seuratd_socket_accept(&sock);
    CHECK(read_block(a, buf, sizeof(buf)) > 0);
    CHECK(strncmp(buf, "scheme source=#6750a4 mode=light\n", 33) == 0);

    b = connect_to(path);
    CHECK(b >= 0);
    seuratd_socket_accept(&sock);
    CHECK(read_block(b, buf, sizeof(buf)) > 0);

    seuratd_state_compute(&state, 0xff00aa00, SEURATD_SCHEME_DARK);
    seuratd_socket_publish(&sock, &state);
    CHECK(read_block(a, buf, sizeof(buf)) > 0);
    CHECK(strncmp(buf, "scheme source=#00aa00 mode=dark\n", 32) == 0);
    CHECK(read_block(b, buf, sizeof(buf)) > 0);

    close(a);
    seuratd_socket_client_event(&sock, &sock.clients[0], EPOLLRDHUP);
    CHECK(sock.clients[0].fd < 0);
    CHECK(sock.clients[1].fd >= 0);

    close(b);
    seuratd_socket_close(&sock);
    CHECK(connect_to(path) < 0);

    rmdir(dir);
    close(epfd);
    return check_failures ? 1 : 0;
}
