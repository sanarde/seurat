/* SPDX-License-Identifier: GPL-3.0-only */
#include <limits.h>
#include <string.h>

#include "check.h"
#include "seurat/source.h"
#include "wallpaper.h"

#ifndef TEST_DATA_DIR
#error TEST_DATA_DIR is not defined
#endif

int main(void)
{
    struct seuratd_wallpaper wallpaper = {.path = TEST_DATA_DIR "/blue_red.png"};
    struct seuratd_wallpaper empty = {.path = ""};
    struct seuratd_wallpaper missing = {.path = TEST_DATA_DIR "/missing.png"};

    CHECK(seuratd_wallpaper_color(&wallpaper) == 0xff2850c8);
    wallpaper.path = TEST_DATA_DIR "/gray.png";
    CHECK(seuratd_wallpaper_color(&wallpaper) == 0xff2850c8);
    seuratd_wallpaper_invalidate(&wallpaper);
    CHECK(seuratd_wallpaper_color(&wallpaper) == SEURAT_FALLBACK_SOURCE_COLOR);

    CHECK(seuratd_wallpaper_color(&empty) == SEURAT_FALLBACK_SOURCE_COLOR);
    CHECK(seuratd_wallpaper_color(&missing) == SEURAT_FALLBACK_SOURCE_COLOR);

    CHECK(seuratd_wallpaper_set(&wallpaper, TEST_DATA_DIR "/gray.png") == 0);
    CHECK(seuratd_wallpaper_color(&wallpaper) != 0xff2850c8);
    CHECK(seuratd_wallpaper_set(&wallpaper, TEST_DATA_DIR "/blue_red.png") == 0);
    CHECK(seuratd_wallpaper_color(&wallpaper) == 0xff2850c8);
    CHECK(seuratd_wallpaper_set(&wallpaper, "") == 0);
    CHECK(seuratd_wallpaper_color(&wallpaper) == SEURAT_FALLBACK_SOURCE_COLOR);
    {
        char too_long[PATH_MAX + 1];

        memset(too_long, 'a', sizeof(too_long) - 1);
        too_long[sizeof(too_long) - 1] = '\0';
        CHECK(seuratd_wallpaper_set(&wallpaper, too_long) != 0);
    }

    return check_failures ? 1 : 0;
}
