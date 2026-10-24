/* SPDX-License-Identifier: GPL-3.0-only */
#include "watch.h"

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

static int add_dir_watch(int fd, const char *path, char *name)
{
    char dir_copy[PATH_MAX];
    char name_copy[PATH_MAX];

    if (path[0] == '\0' ||
        snprintf(dir_copy, sizeof(dir_copy), "%s", path) >= (int)sizeof(dir_copy))
        return -1;
    strcpy(name_copy, dir_copy);
    if (snprintf(name, NAME_MAX + 1, "%s", basename(name_copy)) > NAME_MAX)
        return -1;

    return inotify_add_watch(fd, dirname(dir_copy),
                             IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
}

int seuratd_watch_open(struct seuratd_watch *watch, const char *config_path,
                       const char *wallpaper_path)
{
    memset(watch, 0, sizeof(*watch));
    watch->fd = inotify_init1(IN_CLOEXEC);
    if (watch->fd < 0)
        return -1;

    watch->config_wd = add_dir_watch(watch->fd, config_path, watch->config_name);
    watch->wallpaper_wd =
        add_dir_watch(watch->fd, wallpaper_path, watch->wallpaper_name);
    if (watch->config_wd < 0 && watch->wallpaper_wd < 0) {
        close(watch->fd);
        watch->fd = -1;
        return -1;
    }
    return 0;
}

void seuratd_watch_read(struct seuratd_watch *watch, int *config_changed,
                        int *wallpaper_changed)
{
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len = read(watch->fd, buf, sizeof(buf));

    *config_changed = 0;
    *wallpaper_changed = 0;
    for (char *p = buf; len > 0 && p < buf + len;) {
        const struct inotify_event *ev = (const struct inotify_event *)p;

        if (ev->mask & IN_Q_OVERFLOW) {
            *config_changed = 1;
            *wallpaper_changed = 1;
        } else if (ev->len > 0) {
            if (ev->wd == watch->config_wd &&
                strcmp(ev->name, watch->config_name) == 0)
                *config_changed = 1;
            if (ev->wd == watch->wallpaper_wd &&
                strcmp(ev->name, watch->wallpaper_name) == 0)
                *wallpaper_changed = 1;
        }
        p += sizeof(*ev) + ev->len;
    }
}
