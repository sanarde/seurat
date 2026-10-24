/* SPDX-License-Identifier: GPL-3.0-only */
#include "check.h"
#include "quantize.h"

static uint32_t population_of(const struct seurat_quantized_color *colors,
                              size_t n, uint32_t argb)
{
    for (size_t i = 0; i < n; i++) {
        if (colors[i].argb == argb)
            return colors[i].population;
    }
    return 0;
}

int main(void)
{
    uint32_t pixels[1000];
    struct seurat_quantized_color colors[128];
    size_t n;
    uint32_t total = 0;

    for (int i = 0; i < 1000; i++) {
        if (i < 600)
            pixels[i] = 0xffff0000;
        else if (i < 900)
            pixels[i] = 0xff00ff00;
        else
            pixels[i] = 0xff0000ff;
    }

    n = seurat_quantize_wu(pixels, 1000, 128, colors);
    CHECK(n == 3);
    CHECK(population_of(colors, n, 0xffff0000) == 600);
    CHECK(population_of(colors, n, 0xff00ff00) == 300);
    CHECK(population_of(colors, n, 0xff0000ff) == 100);

    for (int i = 0; i < 1000; i++)
        pixels[i] = 0xff000000u | (uint32_t)(i * 255 / 999) << 16 |
                    (uint32_t)((i * 7) % 256) << 8 | (uint32_t)((i * 13) % 256);

    n = seurat_quantize_wu(pixels, 1000, 16, colors);
    CHECK(n > 1 && n <= 16);
    for (size_t i = 0; i < n; i++)
        total += colors[i].population;
    CHECK(total == 1000);

    n = seurat_quantize_wu(pixels, 1000, 1, colors);
    CHECK(n == 1);
    CHECK(colors[0].population == 1000);

    return check_failures ? 1 : 0;
}
