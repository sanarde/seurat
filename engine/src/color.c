/* SPDX-License-Identifier: GPL-3.0-only */
#include "seurat/color.h"

#include <math.h>

static const double rgb_to_xyz[3][3] = {
    {0.41233895, 0.35762064, 0.18051042},
    {0.2126, 0.7152, 0.0722},
    {0.01932141, 0.11916382, 0.95034478},
};

static const double xyz_to_rgb[3][3] = {
    {3.2413774792388685, -1.5376652402851851, -0.49885366846268053},
    {-0.9691452513005321, 1.8758853451067872, 0.04156585616912061},
    {0.05562093689691305, -0.20395524564742123, 1.0571799111220335},
};

static const double lab_epsilon = 216.0 / 24389.0;
static const double lab_kappa = 24389.0 / 27.0;

static double lab_f(double t)
{
    if (t > lab_epsilon)
        return cbrt(t);
    return (lab_kappa * t + 16.0) / 116.0;
}

double seurat_linearized(uint8_t channel)
{
    double n = channel / 255.0;

    if (n <= 0.040449936)
        return n / 12.92 * 100.0;
    return pow((n + 0.055) / 1.055, 2.4) * 100.0;
}

uint8_t seurat_delinearized(double linear)
{
    double n = linear / 100.0;
    double d;

    if (n <= 0.0031308)
        d = n * 12.92;
    else
        d = 1.055 * pow(n, 1.0 / 2.4) - 0.055;

    d = round(d * 255.0);
    if (d < 0.0)
        return 0;
    if (d > 255.0)
        return 255;
    return (uint8_t)d;
}

uint32_t seurat_argb_from_xyz(const double xyz[3])
{
    uint8_t c[3];

    for (int i = 0; i < 3; i++) {
        double v = xyz_to_rgb[i][0] * xyz[0] + xyz_to_rgb[i][1] * xyz[1] +
                   xyz_to_rgb[i][2] * xyz[2];
        c[i] = seurat_delinearized(v);
    }
    return 0xff000000u | (uint32_t)c[0] << 16 | (uint32_t)c[1] << 8 | c[2];
}

void seurat_xyz_from_argb(uint32_t argb, double xyz[3])
{
    double rgb[3] = {
        seurat_linearized((argb >> 16) & 0xff),
        seurat_linearized((argb >> 8) & 0xff),
        seurat_linearized(argb & 0xff),
    };

    for (int i = 0; i < 3; i++)
        xyz[i] = rgb_to_xyz[i][0] * rgb[0] + rgb_to_xyz[i][1] * rgb[1] +
                 rgb_to_xyz[i][2] * rgb[2];
}

double seurat_lstar_from_y(double y)
{
    return 116.0 * lab_f(y / 100.0) - 16.0;
}

double seurat_y_from_lstar(double lstar)
{
    if (lstar > 8.0)
        return pow((lstar + 16.0) / 116.0, 3.0) * 100.0;
    return lstar / lab_kappa * 100.0;
}

double seurat_lstar_from_argb(uint32_t argb)
{
    double xyz[3];

    seurat_xyz_from_argb(argb, xyz);
    return seurat_lstar_from_y(xyz[1]);
}

uint32_t seurat_argb_from_lstar(double lstar)
{
    uint8_t c = seurat_delinearized(seurat_y_from_lstar(lstar));

    return 0xff000000u | (uint32_t)c << 16 | (uint32_t)c << 8 | c;
}
