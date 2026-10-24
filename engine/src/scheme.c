/* SPDX-License-Identifier: GPL-3.0-only */
#include "seurat/scheme.h"

#include <math.h>

#include "seurat/hct.h"
#include "seurat/palette.h"

enum palette_id {
    PRIMARY_PALETTE,
    SECONDARY_PALETTE,
    TERTIARY_PALETTE,
    NEUTRAL_PALETTE,
    NEUTRAL_VARIANT_PALETTE,
    ERROR_PALETTE,
    PALETTE_COUNT
};

struct role_spec {
    const char *name;
    enum palette_id palette;
    double light_tone;
    double dark_tone;
};

static const struct role_spec roles[SEURAT_COLOR_ROLE_COUNT] = {
    [SEURAT_PRIMARY] = {"primary", PRIMARY_PALETTE, 40, 80},
    [SEURAT_ON_PRIMARY] = {"on_primary", PRIMARY_PALETTE, 100, 20},
    [SEURAT_PRIMARY_CONTAINER] = {"primary_container", PRIMARY_PALETTE, 90, 30},
    [SEURAT_ON_PRIMARY_CONTAINER] = {"on_primary_container", PRIMARY_PALETTE, 10, 90},
    [SEURAT_SECONDARY] = {"secondary", SECONDARY_PALETTE, 40, 80},
    [SEURAT_ON_SECONDARY] = {"on_secondary", SECONDARY_PALETTE, 100, 20},
    [SEURAT_SECONDARY_CONTAINER] = {"secondary_container", SECONDARY_PALETTE, 90, 30},
    [SEURAT_ON_SECONDARY_CONTAINER] = {"on_secondary_container", SECONDARY_PALETTE, 10, 90},
    [SEURAT_TERTIARY] = {"tertiary", TERTIARY_PALETTE, 40, 80},
    [SEURAT_ON_TERTIARY] = {"on_tertiary", TERTIARY_PALETTE, 100, 20},
    [SEURAT_TERTIARY_CONTAINER] = {"tertiary_container", TERTIARY_PALETTE, 90, 30},
    [SEURAT_ON_TERTIARY_CONTAINER] = {"on_tertiary_container", TERTIARY_PALETTE, 10, 90},
    [SEURAT_ERROR] = {"error", ERROR_PALETTE, 40, 80},
    [SEURAT_ON_ERROR] = {"on_error", ERROR_PALETTE, 100, 20},
    [SEURAT_ERROR_CONTAINER] = {"error_container", ERROR_PALETTE, 90, 30},
    [SEURAT_ON_ERROR_CONTAINER] = {"on_error_container", ERROR_PALETTE, 10, 90},
    [SEURAT_BACKGROUND] = {"background", NEUTRAL_PALETTE, 99, 10},
    [SEURAT_ON_BACKGROUND] = {"on_background", NEUTRAL_PALETTE, 10, 90},
    [SEURAT_SURFACE] = {"surface", NEUTRAL_PALETTE, 99, 10},
    [SEURAT_ON_SURFACE] = {"on_surface", NEUTRAL_PALETTE, 10, 90},
    [SEURAT_SURFACE_VARIANT] = {"surface_variant", NEUTRAL_VARIANT_PALETTE, 90, 30},
    [SEURAT_ON_SURFACE_VARIANT] = {"on_surface_variant", NEUTRAL_VARIANT_PALETTE, 30, 80},
    [SEURAT_SURFACE_DIM] = {"surface_dim", NEUTRAL_PALETTE, 87, 6},
    [SEURAT_SURFACE_BRIGHT] = {"surface_bright", NEUTRAL_PALETTE, 98, 24},
    [SEURAT_SURFACE_CONTAINER_LOWEST] = {"surface_container_lowest", NEUTRAL_PALETTE, 100, 4},
    [SEURAT_SURFACE_CONTAINER_LOW] = {"surface_container_low", NEUTRAL_PALETTE, 96, 10},
    [SEURAT_SURFACE_CONTAINER] = {"surface_container", NEUTRAL_PALETTE, 94, 12},
    [SEURAT_SURFACE_CONTAINER_HIGH] = {"surface_container_high", NEUTRAL_PALETTE, 92, 17},
    [SEURAT_SURFACE_CONTAINER_HIGHEST] = {"surface_container_highest", NEUTRAL_PALETTE, 90, 22},
    [SEURAT_OUTLINE] = {"outline", NEUTRAL_VARIANT_PALETTE, 50, 60},
    [SEURAT_OUTLINE_VARIANT] = {"outline_variant", NEUTRAL_VARIANT_PALETTE, 80, 30},
    [SEURAT_SHADOW] = {"shadow", NEUTRAL_PALETTE, 0, 0},
    [SEURAT_SCRIM] = {"scrim", NEUTRAL_PALETTE, 0, 0},
    [SEURAT_INVERSE_SURFACE] = {"inverse_surface", NEUTRAL_PALETTE, 20, 90},
    [SEURAT_INVERSE_ON_SURFACE] = {"inverse_on_surface", NEUTRAL_PALETTE, 95, 20},
    [SEURAT_INVERSE_PRIMARY] = {"inverse_primary", PRIMARY_PALETTE, 80, 40},
    [SEURAT_PRIMARY_FIXED] = {"primary_fixed", PRIMARY_PALETTE, 90, 90},
    [SEURAT_PRIMARY_FIXED_DIM] = {"primary_fixed_dim", PRIMARY_PALETTE, 80, 80},
    [SEURAT_ON_PRIMARY_FIXED] = {"on_primary_fixed", PRIMARY_PALETTE, 10, 10},
    [SEURAT_ON_PRIMARY_FIXED_VARIANT] = {"on_primary_fixed_variant", PRIMARY_PALETTE, 30, 30},
    [SEURAT_SECONDARY_FIXED] = {"secondary_fixed", SECONDARY_PALETTE, 90, 90},
    [SEURAT_SECONDARY_FIXED_DIM] = {"secondary_fixed_dim", SECONDARY_PALETTE, 80, 80},
    [SEURAT_ON_SECONDARY_FIXED] = {"on_secondary_fixed", SECONDARY_PALETTE, 10, 10},
    [SEURAT_ON_SECONDARY_FIXED_VARIANT] = {"on_secondary_fixed_variant", SECONDARY_PALETTE, 30, 30},
    [SEURAT_TERTIARY_FIXED] = {"tertiary_fixed", TERTIARY_PALETTE, 90, 90},
    [SEURAT_TERTIARY_FIXED_DIM] = {"tertiary_fixed_dim", TERTIARY_PALETTE, 80, 80},
    [SEURAT_ON_TERTIARY_FIXED] = {"on_tertiary_fixed", TERTIARY_PALETTE, 10, 10},
    [SEURAT_ON_TERTIARY_FIXED_VARIANT] = {"on_tertiary_fixed_variant", TERTIARY_PALETTE, 30, 30},
};

void seurat_scheme_from_argb(uint32_t source, bool dark,
                             struct seurat_scheme *scheme)
{
    double hue = seurat_hct_from_argb(source).hue;
    struct seurat_tonal_palette palettes[PALETTE_COUNT] = {
        [PRIMARY_PALETTE] = {hue, 36.0},
        [SECONDARY_PALETTE] = {hue, 16.0},
        [TERTIARY_PALETTE] = {fmod(hue + 60.0, 360.0), 24.0},
        [NEUTRAL_PALETTE] = {hue, 6.0},
        [NEUTRAL_VARIANT_PALETTE] = {hue, 8.0},
        [ERROR_PALETTE] = {25.0, 84.0},
    };

    for (int i = 0; i < SEURAT_COLOR_ROLE_COUNT; i++) {
        double tone = dark ? roles[i].dark_tone : roles[i].light_tone;

        scheme->colors[i] =
            seurat_tonal_palette_tone(&palettes[roles[i].palette], tone);
    }
}

const char *seurat_color_role_name(enum seurat_color_role role)
{
    return roles[role].name;
}
