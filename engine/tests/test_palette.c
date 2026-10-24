/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdlib.h>

#include "check.h"
#include "seurat/color.h"
#include "seurat/hct.h"
#include "seurat/palette.h"

static int channel_diff(uint32_t a, uint32_t b)
{
    int dr = abs((int)((a >> 16) & 0xff) - (int)((b >> 16) & 0xff));
    int dg = abs((int)((a >> 8) & 0xff) - (int)((b >> 8) & 0xff));
    int db = abs((int)(a & 0xff) - (int)(b & 0xff));

    return dr > dg ? (dr > db ? dr : db) : (dg > db ? dg : db);
}

int main(void)
{
    struct seurat_tonal_palette palette = {270.0, 36.0};
    struct seurat_tonal_palette baseline = {
        seurat_hct_from_argb(0xff6750a4).hue, 48.0};

    CHECK(seurat_tonal_palette_tone(&palette, 0.0) == 0xff000000);
    CHECK(seurat_tonal_palette_tone(&palette, 100.0) == 0xffffffff);

    for (double tone = 30.0; tone <= 60.0; tone += 10.0) {
        uint32_t argb = seurat_tonal_palette_tone(&palette, tone);
        struct seurat_hct hct = seurat_hct_from_argb(argb);

        CHECK_NEAR(hct.tone, tone, 1.0);
        CHECK_NEAR(hct.hue, 270.0, 3.0);
        CHECK_NEAR(hct.chroma, 36.0, 3.0);
    }

    for (double tone = 10.0; tone <= 90.0; tone += 10.0) {
        uint32_t argb = seurat_tonal_palette_tone(&palette, tone);

        CHECK_NEAR(seurat_lstar_from_argb(argb), tone, 1.0);
    }

    CHECK(channel_diff(seurat_tonal_palette_tone(&baseline, 40.0), 0xff6750a4) <= 1);
    CHECK(channel_diff(seurat_tonal_palette_tone(&baseline, 80.0), 0xffd0bcff) <= 1);
    CHECK(channel_diff(seurat_tonal_palette_tone(&baseline, 30.0), 0xff4f378b) <= 1);
    CHECK(channel_diff(seurat_tonal_palette_tone(&baseline, 90.0), 0xffeaddff) <= 1);

    return check_failures ? 1 : 0;
}
