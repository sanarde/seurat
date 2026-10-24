/* SPDX-License-Identifier: GPL-3.0-only */
#include "state.h"

#include <stdio.h>

void seuratd_state_compute(struct seuratd_state *state, uint32_t source,
                           enum seuratd_color_scheme preference)
{
    state->source = source;
    state->dark = preference == SEURATD_SCHEME_DARK;
    seurat_scheme_from_argb(source, state->dark, &state->scheme);
}

int seuratd_state_format(const struct seuratd_state *state, char *buf,
                         size_t size)
{
    size_t len = 0;
    int n = snprintf(buf, size, "scheme source=#%06x mode=%s\n",
                     state->source & 0xffffff, state->dark ? "dark" : "light");

    if (n < 0 || (size_t)n >= size)
        return -1;
    len = (size_t)n;

    for (int i = 0; i < SEURAT_COLOR_ROLE_COUNT; i++) {
        n = snprintf(buf + len, size - len, "%s=#%06x\n",
                     seurat_color_role_name((enum seurat_color_role)i),
                     state->scheme.colors[i] & 0xffffff);
        if (n < 0 || (size_t)n >= size - len)
            return -1;
        len += (size_t)n;
    }

    n = snprintf(buf + len, size - len, "end\n");
    if (n < 0 || (size_t)n >= size - len)
        return -1;
    return (int)(len + (size_t)n);
}
