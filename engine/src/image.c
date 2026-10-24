/* SPDX-License-Identifier: GPL-3.0-only */
#include "image.h"

#include <stdlib.h>

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int seurat_image_load(const char *path, struct seurat_image *image)
{
    int src_w, src_h, channels;
    unsigned char *src = stbi_load(path, &src_w, &src_h, &channels, 3);
    int w, h;

    if (!src)
        return -1;

    w = src_w < SEURAT_IMAGE_MAX_SIDE ? src_w : SEURAT_IMAGE_MAX_SIDE;
    h = src_h < SEURAT_IMAGE_MAX_SIDE ? src_h : SEURAT_IMAGE_MAX_SIDE;
    if (src_w >= src_h && src_w > SEURAT_IMAGE_MAX_SIDE)
        h = (int)((long long)src_h * SEURAT_IMAGE_MAX_SIDE / src_w);
    else if (src_h > src_w && src_h > SEURAT_IMAGE_MAX_SIDE)
        w = (int)((long long)src_w * SEURAT_IMAGE_MAX_SIDE / src_h);
    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;

    image->pixels = malloc((size_t)w * h * sizeof(uint32_t));
    if (!image->pixels) {
        stbi_image_free(src);
        return -1;
    }
    image->width = w;
    image->height = h;

    for (int y = 0; y < h; y++) {
        int sy = (int)((long long)y * src_h / h);

        for (int x = 0; x < w; x++) {
            int sx = (int)((long long)x * src_w / w);
            const unsigned char *p = src + ((size_t)sy * src_w + sx) * 3;

            image->pixels[(size_t)y * w + x] =
                0xff000000u | (uint32_t)p[0] << 16 | (uint32_t)p[1] << 8 | p[2];
        }
    }

    stbi_image_free(src);
    return 0;
}

void seurat_image_free(struct seurat_image *image)
{
    free(image->pixels);
    image->pixels = NULL;
    image->width = 0;
    image->height = 0;
}
