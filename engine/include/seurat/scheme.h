/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_SCHEME_H
#define SEURAT_SCHEME_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file scheme.h
 * Material You color schemes (tonal spot) derived from a source color.
 */

/** Color roles of a scheme. */
enum seurat_color_role {
    SEURAT_PRIMARY,
    SEURAT_ON_PRIMARY,
    SEURAT_PRIMARY_CONTAINER,
    SEURAT_ON_PRIMARY_CONTAINER,
    SEURAT_SECONDARY,
    SEURAT_ON_SECONDARY,
    SEURAT_SECONDARY_CONTAINER,
    SEURAT_ON_SECONDARY_CONTAINER,
    SEURAT_TERTIARY,
    SEURAT_ON_TERTIARY,
    SEURAT_TERTIARY_CONTAINER,
    SEURAT_ON_TERTIARY_CONTAINER,
    SEURAT_ERROR,
    SEURAT_ON_ERROR,
    SEURAT_ERROR_CONTAINER,
    SEURAT_ON_ERROR_CONTAINER,
    SEURAT_BACKGROUND,
    SEURAT_ON_BACKGROUND,
    SEURAT_SURFACE,
    SEURAT_ON_SURFACE,
    SEURAT_SURFACE_VARIANT,
    SEURAT_ON_SURFACE_VARIANT,
    SEURAT_SURFACE_DIM,
    SEURAT_SURFACE_BRIGHT,
    SEURAT_SURFACE_CONTAINER_LOWEST,
    SEURAT_SURFACE_CONTAINER_LOW,
    SEURAT_SURFACE_CONTAINER,
    SEURAT_SURFACE_CONTAINER_HIGH,
    SEURAT_SURFACE_CONTAINER_HIGHEST,
    SEURAT_OUTLINE,
    SEURAT_OUTLINE_VARIANT,
    SEURAT_SHADOW,
    SEURAT_SCRIM,
    SEURAT_INVERSE_SURFACE,
    SEURAT_INVERSE_ON_SURFACE,
    SEURAT_INVERSE_PRIMARY,
    SEURAT_PRIMARY_FIXED,
    SEURAT_PRIMARY_FIXED_DIM,
    SEURAT_ON_PRIMARY_FIXED,
    SEURAT_ON_PRIMARY_FIXED_VARIANT,
    SEURAT_SECONDARY_FIXED,
    SEURAT_SECONDARY_FIXED_DIM,
    SEURAT_ON_SECONDARY_FIXED,
    SEURAT_ON_SECONDARY_FIXED_VARIANT,
    SEURAT_TERTIARY_FIXED,
    SEURAT_TERTIARY_FIXED_DIM,
    SEURAT_ON_TERTIARY_FIXED,
    SEURAT_ON_TERTIARY_FIXED_VARIANT,
    SEURAT_COLOR_ROLE_COUNT
};

/** A complete scheme: one opaque ARGB color per role. */
struct seurat_scheme {
    uint32_t colors[SEURAT_COLOR_ROLE_COUNT];
};

/**
 * Builds a tonal spot scheme from a source color. Only the hue of the source
 * is used.
 */
void seurat_scheme_from_argb(uint32_t source, bool dark,
                             struct seurat_scheme *scheme);

/** Returns the snake_case name of a role, for example "on_primary". */
const char *seurat_color_role_name(enum seurat_color_role role);

#endif
