/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_PALETTE_H
#define SEURAT_PALETTE_H

#include <stdint.h>

/**
 * @file palette.h
 * Tonal palettes: all tones of one hue and chroma.
 */

/** A tonal palette defined by an HCT hue and chroma. */
struct seurat_tonal_palette {
    double hue;
    double chroma;
};

/** Returns the opaque ARGB color of the palette at the given tone (0..100). */
uint32_t seurat_tonal_palette_tone(const struct seurat_tonal_palette *palette,
                                   double tone);

#endif
