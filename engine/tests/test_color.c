/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "seurat/color.h"

int main(void)
{
    double xyz[3];

    CHECK_NEAR(seurat_lstar_from_argb(0xff000000), 0.0, 0.001);
    CHECK_NEAR(seurat_lstar_from_argb(0xffffffff), 100.0, 0.001);
    CHECK_NEAR(seurat_lstar_from_argb(0xff777777), 50.0, 0.1);
    CHECK(seurat_argb_from_lstar(50.0) == 0xff777777);
    CHECK(seurat_argb_from_lstar(0.0) == 0xff000000);
    CHECK(seurat_argb_from_lstar(100.0) == 0xffffffff);

    seurat_xyz_from_argb(0xffffffff, xyz);
    CHECK_NEAR(xyz[0], 95.047, 0.01);
    CHECK_NEAR(xyz[1], 100.0, 0.01);
    CHECK_NEAR(xyz[2], 108.883, 0.01);

    for (uint32_t v = 0; v < 256; v++) {
        CHECK(seurat_delinearized(seurat_linearized((uint8_t)v)) == v);
    }

    for (uint32_t c = 0; c < 0x1000000; c += 0x010203) {
        uint32_t argb = 0xff000000u | c;

        seurat_xyz_from_argb(argb, xyz);
        CHECK(seurat_argb_from_xyz(xyz) == argb);
    }

    for (double l = 0.0; l <= 100.0; l += 1.0) {
        double back = seurat_lstar_from_y(seurat_y_from_lstar(l));

        CHECK_NEAR(back, l, 1e-9);
    }

    return check_failures ? 1 : 0;
}
