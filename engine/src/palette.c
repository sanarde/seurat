/* SPDX-License-Identifier: GPL-3.0-only */
#include "seurat/palette.h"

#include "seurat/hct.h"

uint32_t seurat_tonal_palette_tone(const struct seurat_tonal_palette *palette,
                                   double tone)
{
    struct seurat_hct hct = {palette->hue, palette->chroma, tone};

    return seurat_hct_to_argb(hct);
}
