/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_HCT_H
#define SEURAT_HCT_H

#include <stdint.h>

/**
 * @file hct.h
 * HCT color space: CAM16 hue and chroma combined with CIE L* as tone.
 */

/** A color in HCT. Hue is in degrees (0..360), tone is L* (0..100). */
struct seurat_hct {
    double hue;
    double chroma;
    double tone;
};

/** Converts ARGB to HCT. */
struct seurat_hct seurat_hct_from_argb(uint32_t argb);

/**
 * Converts HCT to opaque ARGB. The tone is preserved. If the requested chroma
 * is outside the sRGB gamut for the hue and tone, the highest reachable chroma
 * is used.
 */
uint32_t seurat_hct_to_argb(struct seurat_hct hct);

#endif
