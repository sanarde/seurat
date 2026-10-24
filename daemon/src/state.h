/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_STATE_H
#define SEURATD_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "seurat/scheme.h"

#define SEURATD_TEXT_MAX 4096

struct seuratd_state {
    uint32_t source;
    bool dark;
    struct seurat_scheme scheme;
};

void seuratd_state_compute(struct seuratd_state *state, uint32_t source,
                           enum seuratd_color_scheme preference);

int seuratd_state_format(const struct seuratd_state *state, char *buf,
                         size_t size);

#endif
