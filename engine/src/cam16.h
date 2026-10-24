/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_CAM16_H
#define SEURAT_CAM16_H

#include <stdint.h>

struct seurat_viewing_conditions {
    double n;
    double aw;
    double nbb;
    double ncb;
    double c;
    double nc;
    double fl;
    double fl_root;
    double z;
    double rgb_d[3];
};

struct seurat_cam16 {
    double hue;
    double chroma;
    double j;
    double q;
    double m;
    double s;
};

const struct seurat_viewing_conditions *seurat_viewing_conditions_default(void);

struct seurat_cam16 seurat_cam16_from_argb(uint32_t argb);

struct seurat_cam16 seurat_cam16_from_jch(double j, double chroma, double hue);

uint32_t seurat_cam16_to_argb(const struct seurat_cam16 *cam);

double seurat_cam16_distance(const struct seurat_cam16 *a,
                             const struct seurat_cam16 *b);

#endif
