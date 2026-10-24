/* SPDX-License-Identifier: GPL-3.0-only */
#include "quantize.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_BITS 5
#define SIDE 33
#define TOTAL (SIDE * SIDE * SIDE)

enum direction { RED, GREEN, BLUE };

struct box {
    int r0, r1;
    int g0, g1;
    int b0, b1;
    int vol;
};

struct moments {
    int64_t *weights;
    int64_t *r;
    int64_t *g;
    int64_t *b;
    int64_t *sq;
};

static int idx(int r, int g, int b)
{
    return (r * SIDE + g) * SIDE + b;
}

static int64_t volume(const struct box *c, const int64_t *m)
{
    return m[idx(c->r1, c->g1, c->b1)] - m[idx(c->r1, c->g1, c->b0)] -
           m[idx(c->r1, c->g0, c->b1)] + m[idx(c->r1, c->g0, c->b0)] -
           m[idx(c->r0, c->g1, c->b1)] + m[idx(c->r0, c->g1, c->b0)] +
           m[idx(c->r0, c->g0, c->b1)] - m[idx(c->r0, c->g0, c->b0)];
}

static int64_t bottom(const struct box *c, enum direction d, const int64_t *m)
{
    switch (d) {
    case RED:
        return -m[idx(c->r0, c->g1, c->b1)] + m[idx(c->r0, c->g1, c->b0)] +
               m[idx(c->r0, c->g0, c->b1)] - m[idx(c->r0, c->g0, c->b0)];
    case GREEN:
        return -m[idx(c->r1, c->g0, c->b1)] + m[idx(c->r1, c->g0, c->b0)] +
               m[idx(c->r0, c->g0, c->b1)] - m[idx(c->r0, c->g0, c->b0)];
    default:
        return -m[idx(c->r1, c->g1, c->b0)] + m[idx(c->r1, c->g0, c->b0)] +
               m[idx(c->r0, c->g1, c->b0)] - m[idx(c->r0, c->g0, c->b0)];
    }
}

static int64_t top(const struct box *c, enum direction d, int pos,
                   const int64_t *m)
{
    switch (d) {
    case RED:
        return m[idx(pos, c->g1, c->b1)] - m[idx(pos, c->g1, c->b0)] -
               m[idx(pos, c->g0, c->b1)] + m[idx(pos, c->g0, c->b0)];
    case GREEN:
        return m[idx(c->r1, pos, c->b1)] - m[idx(c->r1, pos, c->b0)] -
               m[idx(c->r0, pos, c->b1)] + m[idx(c->r0, pos, c->b0)];
    default:
        return m[idx(c->r1, c->g1, pos)] - m[idx(c->r1, c->g0, pos)] -
               m[idx(c->r0, c->g1, pos)] + m[idx(c->r0, c->g0, pos)];
    }
}

static double variance(const struct box *c, const struct moments *m)
{
    double dr = (double)volume(c, m->r);
    double dg = (double)volume(c, m->g);
    double db = (double)volume(c, m->b);
    double xx = (double)(m->sq[idx(c->r1, c->g1, c->b1)] -
                         m->sq[idx(c->r1, c->g1, c->b0)] -
                         m->sq[idx(c->r1, c->g0, c->b1)] +
                         m->sq[idx(c->r1, c->g0, c->b0)] -
                         m->sq[idx(c->r0, c->g1, c->b1)] +
                         m->sq[idx(c->r0, c->g1, c->b0)] +
                         m->sq[idx(c->r0, c->g0, c->b1)] -
                         m->sq[idx(c->r0, c->g0, c->b0)]);
    double w = (double)volume(c, m->weights);

    return xx - (dr * dr + dg * dg + db * db) / w;
}

static double maximize(const struct box *c, enum direction d, int first,
                       int last, int *cut, const struct moments *m,
                       const int64_t whole[4])
{
    int64_t bottom_r = bottom(c, d, m->r);
    int64_t bottom_g = bottom(c, d, m->g);
    int64_t bottom_b = bottom(c, d, m->b);
    int64_t bottom_w = bottom(c, d, m->weights);
    double best = 0.0;

    *cut = -1;
    for (int i = first; i < last; i++) {
        double hr = (double)(bottom_r + top(c, d, i, m->r));
        double hg = (double)(bottom_g + top(c, d, i, m->g));
        double hb = (double)(bottom_b + top(c, d, i, m->b));
        double hw = (double)(bottom_w + top(c, d, i, m->weights));
        double temp;

        if (hw == 0.0)
            continue;
        temp = (hr * hr + hg * hg + hb * hb) / hw;

        hr = (double)whole[0] - hr;
        hg = (double)whole[1] - hg;
        hb = (double)whole[2] - hb;
        hw = (double)whole[3] - hw;
        if (hw == 0.0)
            continue;
        temp += (hr * hr + hg * hg + hb * hb) / hw;

        if (temp > best) {
            best = temp;
            *cut = i;
        }
    }
    return best;
}

static int cut_box(struct box *one, struct box *two, const struct moments *m)
{
    int64_t whole[4] = {
        volume(one, m->r),
        volume(one, m->g),
        volume(one, m->b),
        volume(one, m->weights),
    };
    int cut_r, cut_g, cut_b;
    double max_r = maximize(one, RED, one->r0 + 1, one->r1, &cut_r, m, whole);
    double max_g = maximize(one, GREEN, one->g0 + 1, one->g1, &cut_g, m, whole);
    double max_b = maximize(one, BLUE, one->b0 + 1, one->b1, &cut_b, m, whole);
    enum direction dir;

    if (max_r >= max_g && max_r >= max_b) {
        if (cut_r < 0)
            return 0;
        dir = RED;
    } else if (max_g >= max_r && max_g >= max_b) {
        dir = GREEN;
    } else {
        dir = BLUE;
    }

    two->r1 = one->r1;
    two->g1 = one->g1;
    two->b1 = one->b1;

    switch (dir) {
    case RED:
        two->r0 = one->r1 = cut_r;
        two->g0 = one->g0;
        two->b0 = one->b0;
        break;
    case GREEN:
        two->g0 = one->g1 = cut_g;
        two->r0 = one->r0;
        two->b0 = one->b0;
        break;
    default:
        two->b0 = one->b1 = cut_b;
        two->r0 = one->r0;
        two->g0 = one->g0;
        break;
    }

    one->vol = (one->r1 - one->r0) * (one->g1 - one->g0) * (one->b1 - one->b0);
    two->vol = (two->r1 - two->r0) * (two->g1 - two->g0) * (two->b1 - two->b0);
    return 1;
}

static void build_histogram(const uint32_t *pixels, size_t count,
                            struct moments *m)
{
    for (size_t i = 0; i < count; i++) {
        int r = (pixels[i] >> 16) & 0xff;
        int g = (pixels[i] >> 8) & 0xff;
        int b = pixels[i] & 0xff;
        int k = idx((r >> (8 - INDEX_BITS)) + 1, (g >> (8 - INDEX_BITS)) + 1,
                    (b >> (8 - INDEX_BITS)) + 1);

        m->weights[k]++;
        m->r[k] += r;
        m->g[k] += g;
        m->b[k] += b;
        m->sq[k] += r * r + g * g + b * b;
    }
}

static void build_moments(struct moments *m)
{
    for (int r = 1; r < SIDE; r++) {
        int64_t area_w[SIDE] = {0};
        int64_t area_r[SIDE] = {0};
        int64_t area_g[SIDE] = {0};
        int64_t area_b[SIDE] = {0};
        int64_t area_sq[SIDE] = {0};

        for (int g = 1; g < SIDE; g++) {
            int64_t line_w = 0, line_r = 0, line_g = 0, line_b = 0, line_sq = 0;

            for (int b = 1; b < SIDE; b++) {
                int k = idx(r, g, b);
                int prev = idx(r - 1, g, b);

                line_w += m->weights[k];
                line_r += m->r[k];
                line_g += m->g[k];
                line_b += m->b[k];
                line_sq += m->sq[k];

                area_w[b] += line_w;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area_sq[b] += line_sq;

                m->weights[k] = m->weights[prev] + area_w[b];
                m->r[k] = m->r[prev] + area_r[b];
                m->g[k] = m->g[prev] + area_g[b];
                m->b[k] = m->b[prev] + area_b[b];
                m->sq[k] = m->sq[prev] + area_sq[b];
            }
        }
    }
}

size_t seurat_quantize_wu(const uint32_t *pixels, size_t count,
                          size_t max_colors,
                          struct seurat_quantized_color *out)
{
    struct moments m;
    int64_t *block = calloc(5 * (size_t)TOTAL, sizeof(int64_t));
    struct box *boxes = calloc(max_colors, sizeof(struct box));
    double *vv = calloc(max_colors, sizeof(double));
    size_t generated = max_colors;
    size_t n = 0;
    size_t next = 0;

    if (!block || !boxes || !vv) {
        free(block);
        free(boxes);
        free(vv);
        return 0;
    }

    m.weights = block;
    m.r = block + TOTAL;
    m.g = block + 2 * TOTAL;
    m.b = block + 3 * TOTAL;
    m.sq = block + 4 * TOTAL;

    build_histogram(pixels, count, &m);
    build_moments(&m);

    boxes[0] = (struct box){0, SIDE - 1, 0, SIDE - 1, 0, SIDE - 1, 0};

    for (size_t i = 1; i < max_colors; i++) {
        if (cut_box(&boxes[next], &boxes[i], &m)) {
            vv[next] = boxes[next].vol > 1 ? variance(&boxes[next], &m) : 0.0;
            vv[i] = boxes[i].vol > 1 ? variance(&boxes[i], &m) : 0.0;
        } else {
            vv[next] = 0.0;
            i--;
        }

        next = 0;
        double best = vv[0];

        for (size_t j = 1; j <= i; j++) {
            if (vv[j] > best) {
                best = vv[j];
                next = j;
            }
        }
        if (best <= 0.0) {
            generated = i + 1;
            break;
        }
    }

    for (size_t i = 0; i < generated; i++) {
        int64_t w = volume(&boxes[i], m.weights);

        if (w > 0) {
            int64_t r = volume(&boxes[i], m.r) / w;
            int64_t g = volume(&boxes[i], m.g) / w;
            int64_t b = volume(&boxes[i], m.b) / w;

            out[n].argb = 0xff000000u | (uint32_t)r << 16 | (uint32_t)g << 8 |
                          (uint32_t)b;
            out[n].population = (uint32_t)w;
            n++;
        }
    }

    free(block);
    free(boxes);
    free(vv);
    return n;
}
