/* SPDX-License-Identifier: GPL-3.0-only */
#include "cam16.h"
#include "check.h"

static void expect(uint32_t argb, double hue, double chroma, double j)
{
    struct seurat_cam16 cam = seurat_cam16_from_argb(argb);

    CHECK_NEAR(cam.hue, hue, 0.05);
    CHECK_NEAR(cam.chroma, chroma, 0.05);
    CHECK_NEAR(cam.j, j, 0.05);
}

int main(void)
{
    expect(0xffff0000, 27.408, 113.357, 46.445);
    expect(0xff00ff00, 142.139, 108.410, 79.331);
    expect(0xff0000ff, 282.788, 87.230, 25.465);
    expect(0xffffffff, 209.492, 2.869, 100.0);
    expect(0xff000000, 0.0, 0.0, 0.0);

    for (uint32_t c = 0; c < 0x1000000; c += 0x010203) {
        uint32_t argb = 0xff000000u | c;
        struct seurat_cam16 cam = seurat_cam16_from_argb(argb);
        struct seurat_cam16 jch =
            seurat_cam16_from_jch(cam.j, cam.chroma, cam.hue);

        CHECK(seurat_cam16_to_argb(&jch) == argb);
    }

    return check_failures ? 1 : 0;
}
