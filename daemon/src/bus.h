/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURATD_BUS_H
#define SEURATD_BUS_H

#include <dbus/dbus.h>
#include <stdint.h>

#include "loop.h"
#include "state.h"

struct seuratd_bus_watch {
    struct seuratd_source source;
    DBusWatch *watch;
    int fd;
};

struct seuratd_bus {
    DBusConnection *connection;
    int epfd;
    const struct seuratd_state *state;
};

int seuratd_bus_open(struct seuratd_bus *bus, int epfd,
                     const struct seuratd_state *state);

void seuratd_bus_close(struct seuratd_bus *bus);

void seuratd_bus_event(struct seuratd_bus *bus,
                       struct seuratd_bus_watch *watch, uint32_t events);

void seuratd_bus_emit_changed(struct seuratd_bus *bus);

#endif
