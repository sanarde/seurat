/* SPDX-License-Identifier: GPL-3.0-only */
#include <math.h>
#include <string.h>

#include "check.h"
#include "seurat/color.h"
#include "seurat/hct.h"
#include "seurat/scheme.h"

int main(void)
{
    struct seurat_scheme light;
    struct seurat_scheme dark;
    double hue = seurat_hct_from_argb(0xff6750a4).hue;
    struct seurat_hct primary;
    struct seurat_hct tertiary;
    struct seurat_hct error;

    seurat_scheme_from_argb(0xff6750a4, false, &light);
    seurat_scheme_from_argb(0xff6750a4, true, &dark);

    primary = seurat_hct_from_argb(light.colors[SEURAT_PRIMARY]);
    CHECK_NEAR(primary.tone, 40.0, 1.0);
    CHECK_NEAR(primary.hue, hue, 3.0);
    CHECK_NEAR(primary.chroma, 36.0, 2.0);

    tertiary = seurat_hct_from_argb(light.colors[SEURAT_TERTIARY]);
    CHECK_NEAR(tertiary.hue, fmod(hue + 60.0, 360.0), 3.0);
    CHECK_NEAR(tertiary.chroma, 24.0, 2.0);

    error = seurat_hct_from_argb(dark.colors[SEURAT_ERROR]);
    CHECK_NEAR(error.tone, 80.0, 1.0);
    CHECK_NEAR(error.hue, 25.0, 3.0);

    CHECK(light.colors[SEURAT_ON_PRIMARY] == 0xffffffff);
    CHECK_NEAR(seurat_lstar_from_argb(dark.colors[SEURAT_SURFACE]), 10.0, 1.0);
    CHECK_NEAR(seurat_lstar_from_argb(light.colors[SEURAT_SURFACE]), 99.0, 1.0);

    CHECK(light.colors[SEURAT_SHADOW] == 0xff000000);
    CHECK(dark.colors[SEURAT_SCRIM] == 0xff000000);

    for (int i = 0; i < SEURAT_COLOR_ROLE_COUNT; i++) {
        const char *name = seurat_color_role_name((enum seurat_color_role)i);

        CHECK(name != NULL && strlen(name) > 0);
        CHECK((light.colors[i] >> 24) == 0xff);
        CHECK((dark.colors[i] >> 24) == 0xff);
    }

    return check_failures ? 1 : 0;
}
