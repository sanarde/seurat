/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdlib.h>

#include "check.h"
#include "seurat/color.h"
#include "seurat/hct.h"

static int channel_diff(uint32_t a, uint32_t b)
{
    int dr = abs((int)((a >> 16) & 0xff) - (int)((b >> 16) & 0xff));
    int dg = abs((int)((a >> 8) & 0xff) - (int)((b >> 8) & 0xff));
    int db = abs((int)(a & 0xff) - (int)(b & 0xff));

    return dr > dg ? (dr > db ? dr : db) : (dg > db ? dg : db);
}

int main(void)
{
    struct seurat_hct red = seurat_hct_from_argb(0xffff0000);
    struct seurat_hct blue = seurat_hct_from_argb(0xff0000ff);

    CHECK_NEAR(red.hue, 27.408, 0.05);
    CHECK_NEAR(red.chroma, 113.357, 0.05);
    CHECK_NEAR(red.tone, 53.233, 0.05);
    CHECK_NEAR(blue.hue, 282.788, 0.05);
    CHECK_NEAR(blue.chroma, 87.230, 0.05);
    CHECK_NEAR(blue.tone, 32.302, 0.05);

    for (uint32_t c = 0; c < 0x1000000; c += 0x010203) {
        uint32_t argb = 0xff000000u | c;
        struct seurat_hct hct = seurat_hct_from_argb(argb);
        uint32_t back = seurat_hct_to_argb(hct);

        if (hct.tone < 99.5)
            CHECK(channel_diff(argb, back) <= 2);
    }

    for (double hue = 0.0; hue < 360.0; hue += 15.0) {
        for (double tone = 5.0; tone < 100.0; tone += 10.0) {
            struct seurat_hct req = {hue, 200.0, tone};
            uint32_t argb = seurat_hct_to_argb(req);

            CHECK_NEAR(seurat_lstar_from_argb(argb), tone, 1.0);
        }
    }

    {
        struct seurat_hct req = {120.0, 0.0, 50.0};

        CHECK(seurat_hct_to_argb(req) == seurat_argb_from_lstar(50.0));
        req.tone = 0.0;
        req.chroma = 50.0;
        CHECK(seurat_hct_to_argb(req) == 0xff000000);
        req.tone = 100.0;
        CHECK(seurat_hct_to_argb(req) == 0xffffffff);
    }

    return check_failures ? 1 : 0;
}
