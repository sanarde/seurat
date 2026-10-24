/* SPDX-License-Identifier: GPL-3.0-only */
#include "seurat/source.h"

#include "image.h"
#include "quantize.h"
#include "score.h"

#define MAX_COLORS 128

int seurat_source_color_from_file(const char *path, uint32_t *argb)
{
    struct seurat_image image;
    struct seurat_quantized_color colors[MAX_COLORS];
    size_t n;

    if (seurat_image_load(path, &image) != 0)
        return -1;

    n = seurat_quantize_wu(image.pixels, (size_t)image.width * image.height,
                           MAX_COLORS, colors);
    seurat_image_free(&image);

    *argb = seurat_score_source_color(colors, n);
    return 0;
}
