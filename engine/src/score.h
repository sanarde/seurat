/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_SCORE_H
#define SEURAT_SCORE_H

#include <stddef.h>
#include <stdint.h>

#include "quantize.h"

#define SEURAT_FALLBACK_SOURCE_COLOR 0xff4285f4u

uint32_t seurat_score_source_color(const struct seurat_quantized_color *colors,
                                   size_t count);

#endif
