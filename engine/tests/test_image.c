/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "image.h"

#ifndef TEST_DATA_DIR
#error TEST_DATA_DIR is not defined
#endif

int main(void)
{
    struct seurat_image image;

    CHECK(seurat_image_load(TEST_DATA_DIR "/halves.png", &image) == 0);
    CHECK(image.width == 128);
    CHECK(image.height == 85);
    CHECK(image.pixels[0] == 0xffff0000);
    CHECK(image.pixels[image.width - 1] == 0xff0000ff);
    CHECK(image.pixels[(image.height - 1) * image.width] == 0xffff0000);
    seurat_image_free(&image);
    CHECK(image.pixels == NULL);

    CHECK(seurat_image_load(TEST_DATA_DIR "/gray.png", &image) == 0);
    CHECK(image.width == 64);
    CHECK(image.height == 64);
    CHECK(image.pixels[100] == 0xff808080);
    seurat_image_free(&image);

    CHECK(seurat_image_load(TEST_DATA_DIR "/missing.png", &image) != 0);

    return check_failures ? 1 : 0;
}
