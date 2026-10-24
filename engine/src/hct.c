/* SPDX-License-Identifier: GPL-3.0-only */
#include "seurat/hct.h"

#include <math.h>

#include "cam16.h"
#include "seurat/color.h"

static double sanitize_degrees(double degrees)
{
    degrees = fmod(degrees, 360.0);
    if (degrees < 0.0)
        degrees += 360.0;
    return degrees;
}

static int find_cam_by_j(double hue, double chroma, double lstar,
                         struct seurat_cam16 *out)
{
    double low = 0.0;
    double high = 100.0;
    double best_dl = 1000.0;
    double best_de = 1000.0;
    int found = 0;

    while (fabs(low - high) > 0.01) {
        double mid = low + (high - low) / 2.0;
        struct seurat_cam16 before = seurat_cam16_from_jch(mid, chroma, hue);
        uint32_t clipped = seurat_cam16_to_argb(&before);
        double clipped_lstar = seurat_lstar_from_argb(clipped);
        double dl = fabs(lstar - clipped_lstar);

        if (dl < 0.2) {
            struct seurat_cam16 cam = seurat_cam16_from_argb(clipped);
            struct seurat_cam16 ideal =
                seurat_cam16_from_jch(cam.j, cam.chroma, hue);
            double de = seurat_cam16_distance(&cam, &ideal);

            if (de <= 1.0) {
                best_dl = dl;
                best_de = de;
                *out = cam;
                found = 1;
            }
        }

        if (best_dl == 0.0 && best_de == 0.0)
            break;

        if (clipped_lstar < lstar)
            low = mid;
        else
            high = mid;
    }
    return found;
}

struct seurat_hct seurat_hct_from_argb(uint32_t argb)
{
    struct seurat_cam16 cam = seurat_cam16_from_argb(argb);
    struct seurat_hct hct = {
        .hue = cam.hue,
        .chroma = cam.chroma,
        .tone = seurat_lstar_from_argb(argb),
    };

    return hct;
}

uint32_t seurat_hct_to_argb(struct seurat_hct hct)
{
    double high = hct.chroma;
    double mid = hct.chroma;
    double low = 0.0;
    int first_loop = 1;
    int have_answer = 0;
    struct seurat_cam16 answer;
    double hue;

    if (hct.chroma < 1.0 || round(hct.tone) <= 0.0 || round(hct.tone) >= 100.0)
        return seurat_argb_from_lstar(hct.tone);

    hue = sanitize_degrees(hct.hue);

    while (fabs(low - high) >= 0.4) {
        struct seurat_cam16 possible;
        int ok = find_cam_by_j(hue, mid, hct.tone, &possible);

        if (first_loop) {
            if (ok)
                return seurat_cam16_to_argb(&possible);
            first_loop = 0;
            mid = low + (high - low) / 2.0;
            continue;
        }

        if (!ok) {
            high = mid;
        } else {
            answer = possible;
            have_answer = 1;
            low = mid;
        }
        mid = low + (high - low) / 2.0;
    }

    if (!have_answer)
        return seurat_argb_from_lstar(hct.tone);
    return seurat_cam16_to_argb(&answer);
}
