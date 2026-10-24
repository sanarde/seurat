/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_WATCH_H
#define SEURATD_WATCH_H

#include <limits.h>

struct seuratd_watch {
    int fd;
    int config_wd;
    int wallpaper_wd;
    char config_name[NAME_MAX + 1];
    char wallpaper_name[NAME_MAX + 1];
};

int seuratd_watch_open(struct seuratd_watch *watch, const char *config_path,
                       const char *wallpaper_path);

void seuratd_watch_read(struct seuratd_watch *watch, int *config_changed,
                        int *wallpaper_changed);

#endif
