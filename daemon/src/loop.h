/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_LOOP_H
#define SEURATD_LOOP_H

enum seuratd_source_kind {
    SEURATD_SRC_SIGNAL,
    SEURATD_SRC_CONFIG,
    SEURATD_SRC_LISTEN,
    SEURATD_SRC_CLIENT,
    SEURATD_SRC_BUS,
};

struct seuratd_source {
    enum seuratd_source_kind kind;
};

#endif
