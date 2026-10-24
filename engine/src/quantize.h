/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_QUANTIZE_H
#define SEURAT_QUANTIZE_H

#include <stddef.h>
#include <stdint.h>

struct seurat_quantized_color {
    uint32_t argb;
    uint32_t population;
};

size_t seurat_quantize_wu(const uint32_t *pixels, size_t count,
                          size_t max_colors,
                          struct seurat_quantized_color *out);

#endif
