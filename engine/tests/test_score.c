/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "score.h"

int main(void)
{
    struct seurat_quantized_color colors[3];

    colors[0] = (struct seurat_quantized_color){0xff808080, 700};
    colors[1] = (struct seurat_quantized_color){0xff2850c8, 280};
    colors[2] = (struct seurat_quantized_color){0xffc82828, 20};
    CHECK(seurat_score_source_color(colors, 3) == 0xff2850c8);

    colors[1].population = 20;
    colors[2].population = 280;
    CHECK(seurat_score_source_color(colors, 3) == 0xffc82828);

    CHECK(seurat_score_source_color(colors, 1) == SEURAT_FALLBACK_SOURCE_COLOR);
    CHECK(seurat_score_source_color(colors, 0) == SEURAT_FALLBACK_SOURCE_COLOR);

    return check_failures ? 1 : 0;
}
