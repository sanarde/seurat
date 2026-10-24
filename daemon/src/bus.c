/* SPDX-License-Identifier: GPL-3.0-only */
#include "bus.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#define BUS_NAME "org.nuros.sanar.seurat"
#define OBJECT_PATH "/org/nuros/sanar/seurat"
#define INTERFACE "org.nuros.sanar.Seurat"

static const char introspection_xml[] =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection "
    "1.0//EN\" "
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
    "<node>\n"
    "  <interface name=\"" INTERFACE "\">\n"
    "    <method name=\"GetScheme\">\n"
    "      <arg type=\"a{su}\" direction=\"out\"/>\n"
    "    </method>\n"
    "    <method name=\"GetSourceColor\">\n"
    "      <arg type=\"u\" direction=\"out\"/>\n"
    "    </method>\n"
    "    <signal name=\"SchemeChanged\">\n"
    "      <arg type=\"a{su}\"/>\n"
    "    </signal>\n"
    "  </interface>\n"
    "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
    "    <method name=\"Introspect\">\n"
    "      <arg type=\"s\" direction=\"out\"/>\n"
    "    </method>\n"
    "  </interface>\n"
    "</node>\n";

static int watch_update(struct seuratd_bus *bus, struct seuratd_bus_watch *w,
                        int op)
{
    struct epoll_event ev = {.events = 0, .data.ptr = &w->source};

    if (dbus_watch_get_enabled(w->watch)) {
        unsigned int flags = dbus_watch_get_flags(w->watch);

        if (flags & DBUS_WATCH_READABLE)
            ev.events |= EPOLLIN;
        if (flags & DBUS_WATCH_WRITABLE)
            ev.events |= EPOLLOUT;
    }
    return epoll_ctl(bus->epfd, op, w->fd, &ev);
}

static dbus_bool_t add_watch(DBusWatch *watch, void *data)
{
    struct seuratd_bus *bus = data;
    struct seuratd_bus_watch *w = calloc(1, sizeof(*w));

    if (!w)
        return FALSE;
    w->source.kind = SEURATD_SRC_BUS;
    w->watch = watch;
    w->fd = fcntl(dbus_watch_get_unix_fd(watch), F_DUPFD_CLOEXEC, 0);
    if (w->fd < 0) {
        free(w);
        return FALSE;
    }
    if (watch_update(bus, w, EPOLL_CTL_ADD) != 0) {
        close(w->fd);
        free(w);
        return FALSE;
    }
    dbus_watch_set_data(watch, w, NULL);
    return TRUE;
}

static void remove_watch(DBusWatch *watch, void *data)
{
    struct seuratd_bus *bus = data;
    struct seuratd_bus_watch *w = dbus_watch_get_data(watch);

    epoll_ctl(bus->epfd, EPOLL_CTL_DEL, w->fd, NULL);
    close(w->fd);
    dbus_watch_set_data(watch, NULL, NULL);
    free(w);
}

static void toggle_watch(DBusWatch *watch, void *data)
{
    watch_update(data, dbus_watch_get_data(watch), EPOLL_CTL_MOD);
}

static void append_scheme(DBusMessage *message, const struct seuratd_state *state)
{
    DBusMessageIter iter, array, entry;

    dbus_message_iter_init_append(message, &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{su}", &array);
    for (int i = 0; i < SEURAT_COLOR_ROLE_COUNT; i++) {
        const char *name = seurat_color_role_name((enum seurat_color_role)i);
        dbus_uint32_t argb = state->scheme.colors[i];

        dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL,
                                         &entry);
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &name);
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_UINT32, &argb);
        dbus_message_iter_close_container(&array, &entry);
    }
    dbus_message_iter_close_container(&iter, &array);
}

static DBusHandlerResult handle_message(DBusConnection *connection,
                                        DBusMessage *message, void *data)
{
    struct seuratd_bus *bus = data;
    DBusMessage *reply;

    if (dbus_message_is_method_call(message, INTERFACE, "GetScheme")) {
        reply = dbus_message_new_method_return(message);
        if (reply)
            append_scheme(reply, bus->state);
    } else if (dbus_message_is_method_call(message, INTERFACE,
                                           "GetSourceColor")) {
        dbus_uint32_t source = bus->state->source;

        reply = dbus_message_new_method_return(message);
        if (reply)
            dbus_message_append_args(reply, DBUS_TYPE_UINT32, &source,
                                     DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(
                   message, "org.freedesktop.DBus.Introspectable",
                   "Introspect")) {
        const char *xml = introspection_xml;

        reply = dbus_message_new_method_return(message);
        if (reply)
            dbus_message_append_args(reply, DBUS_TYPE_STRING, &xml,
                                     DBUS_TYPE_INVALID);
    } else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (!reply)
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    dbus_connection_send(connection, reply, NULL);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static const DBusObjectPathVTable vtable = {
    .message_function = handle_message,
};

int seuratd_bus_open(struct seuratd_bus *bus, int epfd,
                     const struct seuratd_state *state)
{
    DBusError error;

    bus->connection = NULL;
    bus->epfd = epfd;
    bus->state = state;

    dbus_error_init(&error);
    bus->connection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
    if (!bus->connection) {
        fprintf(stderr, "seuratd: session bus: %s\n", error.message);
        dbus_error_free(&error);
        return -1;
    }
    dbus_connection_set_exit_on_disconnect(bus->connection, FALSE);

    if (dbus_bus_request_name(bus->connection, BUS_NAME,
                              DBUS_NAME_FLAG_DO_NOT_QUEUE,
                              &error) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "seuratd: cannot own %s%s%s\n", BUS_NAME,
                dbus_error_is_set(&error) ? ": " : "",
                dbus_error_is_set(&error) ? error.message : "");
        dbus_error_free(&error);
        goto fail;
    }

    if (!dbus_connection_register_object_path(bus->connection, OBJECT_PATH,
                                              &vtable, bus) ||
        !dbus_connection_set_watch_functions(bus->connection, add_watch,
                                             remove_watch, toggle_watch, bus,
                                             NULL)) {
        fprintf(stderr, "seuratd: cannot register D-Bus object\n");
        goto fail;
    }
    return 0;

fail:
    dbus_connection_close(bus->connection);
    dbus_connection_unref(bus->connection);
    bus->connection = NULL;
    return -1;
}

void seuratd_bus_close(struct seuratd_bus *bus)
{
    if (!bus->connection)
        return;
    dbus_connection_close(bus->connection);
    dbus_connection_unref(bus->connection);
    bus->connection = NULL;
}

void seuratd_bus_event(struct seuratd_bus *bus,
                       struct seuratd_bus_watch *watch, uint32_t events)
{
    unsigned int flags = 0;

    if (!bus->connection || !dbus_watch_get_enabled(watch->watch))
        return;
    if (events & EPOLLIN)
        flags |= DBUS_WATCH_READABLE;
    if (events & EPOLLOUT)
        flags |= DBUS_WATCH_WRITABLE;
    if (events & EPOLLERR)
        flags |= DBUS_WATCH_ERROR;
    if (events & EPOLLHUP)
        flags |= DBUS_WATCH_HANGUP;

    dbus_watch_handle(watch->watch, flags);
    while (dbus_connection_get_dispatch_status(bus->connection) ==
           DBUS_DISPATCH_DATA_REMAINS)
        dbus_connection_dispatch(bus->connection);

    if (!dbus_connection_get_is_connected(bus->connection)) {
        fprintf(stderr, "seuratd: disconnected from session bus\n");
        seuratd_bus_close(bus);
    }
}

void seuratd_bus_emit_changed(struct seuratd_bus *bus)
{
    DBusMessage *signal;

    if (!bus->connection)
        return;
    signal = dbus_message_new_signal(OBJECT_PATH, INTERFACE, "SchemeChanged");
    if (!signal)
        return;
    append_scheme(signal, bus->state);
    dbus_connection_send(bus->connection, signal, NULL);
    dbus_message_unref(signal);
}
