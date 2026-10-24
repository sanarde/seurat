/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_SOURCE_H
#define SEURAT_SOURCE_H

#include <stdint.h>

/**
 * @file source.h
 * Source color selection from an image.
 */

/** Source color used when an image has no suitable color. */
#define SEURAT_FALLBACK_SOURCE_COLOR 0xff4285f4u

/**
 * Decodes a PNG or JPEG file and picks a source color from it. The image is
 * sampled down to at most 128 pixels per side before analysis. If no color in
 * the image is suitable (for example a grayscale image), SEURAT_FALLBACK_SOURCE_COLOR
 * is returned.
 *
 * @param path Path to the image file.
 * @param argb Receives the opaque ARGB source color.
 * @return 0 on success, -1 if the file cannot be read or decoded.
 */
int seurat_source_color_from_file(const char *path, uint32_t *argb);

#endif
