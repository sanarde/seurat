/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "seurat/source.h"
#include "wallpaper.h"

#ifndef TEST_DATA_DIR
#error TEST_DATA_DIR is not defined
#endif

int main(void)
{
    struct seuratd_wallpaper wallpaper = {TEST_DATA_DIR "/blue_red.png", 0, 0};
    struct seuratd_wallpaper empty = {"", 0, 0};
    struct seuratd_wallpaper missing = {TEST_DATA_DIR "/missing.png", 0, 0};

    CHECK(seuratd_wallpaper_color(&wallpaper) == 0xff2850c8);
    wallpaper.path = TEST_DATA_DIR "/gray.png";
    CHECK(seuratd_wallpaper_color(&wallpaper) == 0xff2850c8);
    seuratd_wallpaper_invalidate(&wallpaper);
    CHECK(seuratd_wallpaper_color(&wallpaper) == SEURAT_FALLBACK_SOURCE_COLOR);

    CHECK(seuratd_wallpaper_color(&empty) == SEURAT_FALLBACK_SOURCE_COLOR);
    CHECK(seuratd_wallpaper_color(&missing) == SEURAT_FALLBACK_SOURCE_COLOR);

    return check_failures ? 1 : 0;
}
