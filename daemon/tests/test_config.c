/* SPDX-License-Identifier: GPL-3.0-only */
#include <string.h>

#include "check.h"
#include "config.h"

static int parse(const char *text, struct seuratd_config *config)
{
    FILE *f = fmemopen((void *)text, strlen(text), "r");
    int rc = seuratd_config_parse(f, "test", config);

    fclose(f);
    return rc;
}

int main(void)
{
    struct seuratd_config config;

    CHECK(parse("", &config) == 0);
    CHECK(config.enabled);
    CHECK(!config.has_color);
    CHECK(config.color_scheme == SEURATD_SCHEME_DEFAULT);

    CHECK(parse("# note\n[seurat]\nenabled = false\ncolor = #6750A4\n"
                "color_scheme = dark\n",
                &config) == 0);
    CHECK(!config.enabled);
    CHECK(config.has_color);
    CHECK(config.color == 0xff6750a4);
    CHECK(config.color_scheme == SEURATD_SCHEME_DARK);

    CHECK(parse("[seurat]\ncolor = 00ff00\n[other]\nenabled = maybe\n",
                &config) == 0);
    CHECK(config.color == 0xff00ff00);
    CHECK(config.enabled);

    CHECK(parse("[seurat]\ncolor = #6750a4\ncolor =\n", &config) == 0);
    CHECK(!config.has_color);

    CHECK(parse("[seurat]\nunknown = 1\ncolor_scheme = light", &config) == 0);
    CHECK(config.color_scheme == SEURATD_SCHEME_LIGHT);

    config.enabled = false;
    CHECK(parse("[seurat]\nenabled = maybe\n", &config) != 0);
    CHECK(!config.enabled);
    CHECK(parse("[seurat]\ncolor = #12345\n", &config) != 0);
    CHECK(parse("[seurat]\ncolor = #12345g\n", &config) != 0);
    CHECK(parse("[seurat]\ncolor = -00001\n", &config) != 0);
    CHECK(parse("[seurat]\ncolor = +12345\n", &config) != 0);
    CHECK(parse("[seurat]\ncolor = 0x1234\n", &config) != 0);
    CHECK(parse("[seurat]\ncolor_scheme = blue\n", &config) != 0);
    CHECK(parse("[seurat\n", &config) != 0);
    CHECK(parse("[seurat]\nenabled\n", &config) != 0);

    return check_failures ? 1 : 0;
}
