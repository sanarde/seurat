/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_CONFIG_H
#define SEURATD_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum seuratd_color_scheme {
    SEURATD_SCHEME_DEFAULT,
    SEURATD_SCHEME_DARK,
    SEURATD_SCHEME_LIGHT,
};

struct seuratd_config {
    bool enabled;
    bool has_color;
    uint32_t color;
    enum seuratd_color_scheme color_scheme;
};

void seuratd_config_defaults(struct seuratd_config *config);

int seuratd_config_parse(FILE *file, const char *name,
                         struct seuratd_config *config);

int seuratd_config_load(const char *path, struct seuratd_config *config);

int seuratd_config_default_path(char *buf, size_t size);

#endif
