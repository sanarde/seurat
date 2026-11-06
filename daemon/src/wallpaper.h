/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_WALLPAPER_H
#define SEURATD_WALLPAPER_H

#include <limits.h>
#include <stdint.h>

struct seuratd_wallpaper {
    const char *path;
    uint32_t color;
    int valid;
    char storage[PATH_MAX];
};

uint32_t seuratd_wallpaper_color(struct seuratd_wallpaper *wallpaper);

void seuratd_wallpaper_invalidate(struct seuratd_wallpaper *wallpaper);

int seuratd_wallpaper_set(struct seuratd_wallpaper *wallpaper, const char *path);

#endif
