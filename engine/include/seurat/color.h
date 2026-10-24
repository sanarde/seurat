/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_COLOR_H
#define SEURAT_COLOR_H

#include <stdint.h>

/**
 * @file color.h
 * Conversions between 32-bit ARGB (0xAARRGGBB, sRGB), CIE XYZ (D65, Y in
 * 0..100) and CIE L*.
 */

/** Converts an sRGB channel (0..255) to linear light in 0..100. */
double seurat_linearized(uint8_t channel);

/** Converts linear light in 0..100 to an sRGB channel, clamped to 0..255. */
uint8_t seurat_delinearized(double linear);

/** Converts XYZ (0..100) to opaque ARGB. Out-of-gamut values are clamped. */
uint32_t seurat_argb_from_xyz(const double xyz[3]);

/** Converts ARGB to XYZ (0..100). */
void seurat_xyz_from_argb(uint32_t argb, double xyz[3]);

/** Returns the CIE L* (0..100) of an ARGB color. */
double seurat_lstar_from_argb(uint32_t argb);

/** Returns the opaque gray ARGB color with the given L*. */
uint32_t seurat_argb_from_lstar(double lstar);

/** Returns luminance Y (0..100) for a given L*. */
double seurat_y_from_lstar(double lstar);

/** Returns L* for a given luminance Y (0..100). */
double seurat_lstar_from_y(double y);

#endif
