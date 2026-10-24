/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_IMAGE_H
#define SEURAT_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#define SEURAT_IMAGE_MAX_SIDE 128

struct seurat_image {
    uint32_t *pixels;
    int width;
    int height;
};

int seurat_image_load(const char *path, struct seurat_image *image);

void seurat_image_free(struct seurat_image *image);

#endif
