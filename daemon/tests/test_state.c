/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "check.h"
#include "state.h"

int main(void)
{
    struct seuratd_state state;
    struct seurat_scheme expected;
    char buf[SEURATD_TEXT_MAX];
    char small[16];
    int len;
    int lines = 0;

    seuratd_state_compute(&state, 0xff6750a4, SEURATD_SCHEME_DARK);
    seurat_scheme_from_argb(0xff6750a4, true, &expected);
    CHECK(state.dark);
    CHECK(state.source == 0xff6750a4);
    CHECK(memcmp(&state.scheme, &expected, sizeof(expected)) == 0);

    seuratd_state_compute(&state, 0xff6750a4, SEURATD_SCHEME_DEFAULT);
    CHECK(!state.dark);

    len = seuratd_state_format(&state, buf, sizeof(buf));
    CHECK(len > 0);
    CHECK((size_t)len == strlen(buf));
    CHECK(strncmp(buf, "scheme source=#6750a4 mode=light\n", 33) == 0);
    CHECK(strstr(buf, "\nprimary=#") != NULL);
    CHECK(strcmp(buf + len - 4, "end\n") == 0);

    for (const char *p = buf; *p; p++)
        lines += *p == '\n';
    CHECK(lines == SEURAT_COLOR_ROLE_COUNT + 2);

    CHECK(seuratd_state_format(&state, small, sizeof(small)) < 0);

    return check_failures ? 1 : 0;
}
