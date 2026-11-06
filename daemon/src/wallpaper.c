/* SPDX-License-Identifier: GPL-3.0-only */
#include "wallpaper.h"

#include <stdio.h>
#include <string.h>

#include "seurat/source.h"

uint32_t seuratd_wallpaper_color(struct seuratd_wallpaper *wallpaper)
{
    if (wallpaper->valid)
        return wallpaper->color;

    wallpaper->color = SEURAT_FALLBACK_SOURCE_COLOR;
    if (wallpaper->path[0] != '\0' &&
        seurat_source_color_from_file(wallpaper->path, &wallpaper->color) != 0) {
        fprintf(stderr, "seuratd: cannot read wallpaper %s\n", wallpaper->path);
        wallpaper->color = SEURAT_FALLBACK_SOURCE_COLOR;
    }
    wallpaper->valid = 1;
    return wallpaper->color;
}

void seuratd_wallpaper_invalidate(struct seuratd_wallpaper *wallpaper)
{
    wallpaper->valid = 0;
}

int seuratd_wallpaper_set(struct seuratd_wallpaper *wallpaper, const char *path)
{
    if (strlen(path) >= sizeof(wallpaper->storage))
        return -1;
    strcpy(wallpaper->storage, path);
    wallpaper->path = wallpaper->storage;
    wallpaper->valid = 0;
    return 0;
}
