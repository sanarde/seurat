/* SPDX-License-Identifier: GPL-3.0-only */
#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 256

static char *trim(char *s)
{
    char *end;

    while (isspace((unsigned char)*s))
        s++;
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1]))
        end--;
    *end = '\0';
    return s;
}

static int parse_bool(const char *value, bool *out)
{
    if (strcmp(value, "true") == 0 || strcmp(value, "yes") == 0 ||
        strcmp(value, "1") == 0) {
        *out = true;
        return 0;
    }
    if (strcmp(value, "false") == 0 || strcmp(value, "no") == 0 ||
        strcmp(value, "0") == 0) {
        *out = false;
        return 0;
    }
    return -1;
}

static int parse_color(const char *value, uint32_t *out)
{
    char *end;
    unsigned long v;

    if (*value == '#')
        value++;
    if (strlen(value) != 6)
        return -1;
    v = strtoul(value, &end, 16);
    if (*end != '\0')
        return -1;
    *out = 0xff000000u | (uint32_t)v;
    return 0;
}

static int parse_scheme(const char *value, enum seuratd_color_scheme *out)
{
    if (strcmp(value, "default") == 0)
        *out = SEURATD_SCHEME_DEFAULT;
    else if (strcmp(value, "dark") == 0)
        *out = SEURATD_SCHEME_DARK;
    else if (strcmp(value, "light") == 0)
        *out = SEURATD_SCHEME_LIGHT;
    else
        return -1;
    return 0;
}

static int apply(struct seuratd_config *config, const char *key,
                 const char *value)
{
    if (strcmp(key, "enabled") == 0)
        return parse_bool(value, &config->enabled);
    if (strcmp(key, "color") == 0) {
        if (*value == '\0') {
            config->has_color = false;
            return 0;
        }
        config->has_color = true;
        return parse_color(value, &config->color);
    }
    if (strcmp(key, "color_scheme") == 0)
        return parse_scheme(value, &config->color_scheme);
    return -2;
}

void seuratd_config_defaults(struct seuratd_config *config)
{
    config->enabled = true;
    config->has_color = false;
    config->color = 0;
    config->color_scheme = SEURATD_SCHEME_DEFAULT;
}

int seuratd_config_parse(FILE *file, const char *name,
                         struct seuratd_config *config)
{
    struct seuratd_config parsed;
    char line[LINE_MAX_LEN];
    bool in_section = false;
    int lineno = 0;

    seuratd_config_defaults(&parsed);

    while (fgets(line, sizeof(line), file)) {
        char *s;
        char *eq;
        int rc;

        lineno++;
        if (!strchr(line, '\n') && !feof(file)) {
            fprintf(stderr, "seuratd: %s:%d: line too long\n", name, lineno);
            return -1;
        }

        s = trim(line);
        if (*s == '\0' || *s == '#' || *s == ';')
            continue;

        if (*s == '[') {
            size_t len = strlen(s);

            if (s[len - 1] != ']') {
                fprintf(stderr, "seuratd: %s:%d: malformed section\n", name,
                        lineno);
                return -1;
            }
            s[len - 1] = '\0';
            in_section = strcmp(trim(s + 1), "seurat") == 0;
            continue;
        }

        eq = strchr(s, '=');
        if (!eq) {
            fprintf(stderr, "seuratd: %s:%d: expected key=value\n", name,
                    lineno);
            return -1;
        }
        *eq = '\0';

        if (!in_section)
            continue;

        rc = apply(&parsed, trim(s), trim(eq + 1));
        if (rc == -2) {
            fprintf(stderr, "seuratd: %s:%d: unknown key '%s'\n", name, lineno,
                    trim(s));
        } else if (rc != 0) {
            fprintf(stderr, "seuratd: %s:%d: invalid value for '%s'\n", name,
                    lineno, trim(s));
            return -1;
        }
    }

    *config = parsed;
    return 0;
}

int seuratd_config_load(const char *path, struct seuratd_config *config)
{
    FILE *file = fopen(path, "r");
    int rc;

    if (!file) {
        if (errno == ENOENT) {
            seuratd_config_defaults(config);
            return 0;
        }
        fprintf(stderr, "seuratd: %s: %s\n", path, strerror(errno));
        return -1;
    }

    rc = seuratd_config_parse(file, path, config);
    fclose(file);
    return rc;
}

int seuratd_config_default_path(char *buf, size_t size)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    int n;

    if (xdg && *xdg)
        n = snprintf(buf, size, "%s/seurat/seurat.conf", xdg);
    else if (home && *home)
        n = snprintf(buf, size, "%s/.config/seurat/seurat.conf", home);
    else
        return -1;

    return (n < 0 || (size_t)n >= size) ? -1 : 0;
}
