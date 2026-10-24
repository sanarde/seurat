/* SPDX-License-Identifier: GPL-3.0-only */
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "check.h"
#include "watch.h"

static void touch(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);

    if (fd >= 0)
        close(fd);
}

static void poll_watch(struct seuratd_watch *watch, int *config,
                       int *wallpaper)
{
    *config = 0;
    *wallpaper = 0;
    seuratd_watch_read(watch, config, wallpaper);
}

int main(void)
{
    char dir[] = "/tmp/seurat-watch-XXXXXX";
    char conf[PATH_MAX];
    char conf_moved[PATH_MAX];
    char image[PATH_MAX];
    char other[PATH_MAX];
    struct seuratd_watch watch;
    int config;
    int wallpaper;

    if (!mkdtemp(dir))
        return 1;
    snprintf(conf, sizeof(conf), "%s/seurat.conf", dir);
    snprintf(conf_moved, sizeof(conf_moved), "%s/seurat.conf.bak", dir);
    snprintf(image, sizeof(image), "%s/wallpaper.png", dir);
    snprintf(other, sizeof(other), "%s/other", dir);

    CHECK(seuratd_watch_open(&watch, conf, image) == 0);
    CHECK(watch.config_wd >= 0);
    CHECK(watch.wallpaper_wd >= 0);
    fcntl(watch.fd, F_SETFL, O_NONBLOCK);

    touch(other);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(!config && !wallpaper);

    touch(conf);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(config && !wallpaper);

    touch(image);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(!config && wallpaper);

    rename(conf, conf_moved);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(!config && !wallpaper);

    unlink(image);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(!config && !wallpaper);

    rename(conf_moved, conf);
    poll_watch(&watch, &config, &wallpaper);
    CHECK(config && !wallpaper);

    close(watch.fd);
    CHECK(seuratd_watch_open(&watch, conf, "") == 0);
    CHECK(watch.config_wd >= 0);
    CHECK(watch.wallpaper_wd < 0);
    close(watch.fd);

    CHECK(seuratd_watch_open(&watch, "/nonexistent/seurat.conf", "") != 0);

    unlink(conf);
    unlink(other);
    rmdir(dir);

    return check_failures ? 1 : 0;
}
