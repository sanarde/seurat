/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "seurat/source.h"

#ifndef TEST_DATA_DIR
#error TEST_DATA_DIR is not defined
#endif

int main(void)
{
    uint32_t argb = 0;

    CHECK(seurat_source_color_from_file(TEST_DATA_DIR "/blue_red.png", &argb) == 0);
    CHECK(argb == 0xff2850c8);

    CHECK(seurat_source_color_from_file(TEST_DATA_DIR "/gray.png", &argb) == 0);
    CHECK(argb == 0xff4285f4);

    CHECK(seurat_source_color_from_file(TEST_DATA_DIR "/missing.png", &argb) != 0);

    return check_failures ? 1 : 0;
}
