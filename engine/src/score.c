/* SPDX-License-Identifier: GPL-3.0-only */
#include "score.h"

#include <math.h>

#include "cam16.h"

#define TARGET_CHROMA 48.0
#define WEIGHT_PROPORTION 0.7
#define WEIGHT_CHROMA_ABOVE 0.3
#define WEIGHT_CHROMA_BELOW 0.1
#define CUTOFF_CHROMA 5.0
#define CUTOFF_EXCITED_PROPORTION 0.01
#define MAX_COLORS 128

static int sanitize_hue(int degrees)
{
    degrees %= 360;
    if (degrees < 0)
        degrees += 360;
    return degrees;
}

uint32_t seurat_score_source_color(const struct seurat_quantized_color *colors,
                                   size_t count)
{
    struct seurat_cam16 cams[MAX_COLORS];
    double hue_population[360] = {0};
    double excited[360] = {0};
    double total = 0.0;
    double best_score = -INFINITY;
    uint32_t best = SEURAT_FALLBACK_SOURCE_COLOR;

    if (count > MAX_COLORS)
        count = MAX_COLORS;

    for (size_t i = 0; i < count; i++) {
        cams[i] = seurat_cam16_from_argb(colors[i].argb);
        hue_population[sanitize_hue((int)floor(cams[i].hue))] +=
            colors[i].population;
        total += colors[i].population;
    }
    if (total == 0.0)
        return best;

    for (int hue = 0; hue < 360; hue++) {
        double proportion = hue_population[hue] / total;

        for (int i = hue - 14; i < hue + 16; i++)
            excited[sanitize_hue(i)] += proportion;
    }

    for (size_t i = 0; i < count; i++) {
        double proportion = excited[sanitize_hue((int)round(cams[i].hue))];
        double chroma_weight;
        double score;

        if (cams[i].chroma < CUTOFF_CHROMA ||
            proportion <= CUTOFF_EXCITED_PROPORTION)
            continue;

        chroma_weight = cams[i].chroma < TARGET_CHROMA ? WEIGHT_CHROMA_BELOW
                                                       : WEIGHT_CHROMA_ABOVE;
        score = proportion * 100.0 * WEIGHT_PROPORTION +
                (cams[i].chroma - TARGET_CHROMA) * chroma_weight;

        if (score > best_score) {
            best_score = score;
            best = colors[i].argb;
        }
    }
    return best;
}
