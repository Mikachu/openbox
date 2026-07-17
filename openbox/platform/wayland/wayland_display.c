/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   wayland_display.c - Wayland Display Operations for PAL
   Copyright (c) 2026  Openbox Development Team

   Wayland display backend implementation.
*/

#include "wayland_backend.h"

/* Registry listener */
static const struct wl_registry_listener registry_listener = {
    .global = wayland_registry_handle_global,
    .global_remove = wayland_registry_handle_global_remove,
};

/* XDG WM base listener */
static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = wayland_xdg_wm_base_ping,
};

void wayland_registry_handle_global(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version)
{
    ObWaylandDisplay *d = data;
    
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        d->wl_compositor = wl_registry_bind(registry, id,
                                            &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        d->wl_shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        /* Create new output */
        ObWaylandOutput *output = g_new0(ObWaylandOutput, 1);
        output->display = d;
        output->id = id;
        output->version = version;
        output->wl_output = wl_registry_bind(registry, id,
                                             &wl_output_interface, 3);
        
        /* Setup output listener */
        static const struct wl_output_listener output_listener = {
            .geometry = wayland_output_handle_geometry,
            .mode = wayland_output_handle_mode,
            .done = wayland_output_handle_done,
            .scale = wayland_output_handle_scale,
        };
        wl_output_add_listener(output->wl_output, &output_listener, output);
        
        wl_list_insert(&d->outputs, &output->link);
        d->output_count++;
        
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        /* Create new seat */
        ObWaylandSeat *seat = g_new0(ObWaylandSeat, 1);
        seat->display = d;
        seat->id = id;
        seat->version = version;
        seat->wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, 5);
        
        /* Setup seat listener */
        static const struct wl_seat_listener seat_listener = {
            .capabilities = wayland_seat_handle_capabilities,
            .name = wayland_seat_handle_name,
        };
        wl_seat_add_listener(seat->wl_seat, &seat_listener, seat);
        
        wl_list_insert(&d->seats, &seat->link);
        
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        d->xdg_wm_base = wl_registry_bind(registry, id,
                                          &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(d->xdg_wm_base, &xdg_wm_base_listener, d);
        
    } else if (strcmp(interface, 
                      zxdg_decoration_manager_v1_interface.name) == 0) {
        d->decoration_manager = wl_registry_bind(registry, id,
                                                 &zxdg_decoration_manager_v1_interface,
                                                 1);
    }
}

void wayland_registry_handle_global_remove(void *data,
                                           struct wl_registry *registry,
                                           uint32_t id)
{
    ObWaylandDisplay *d = data;
    
    /* Remove output if it matches */
    ObWaylandOutput *output;
    wl_list_for_each(output, &d->outputs, link) {
        if (output->id == id) {
            wl_list_remove(&output->link);
            wl_output_destroy(output->wl_output);
            g_free(output->monitor.name);
            g_free(output->monitor.make);
            g_free(output->monitor.model);
            g_free(output);
            d->output_count--;
            return;
        }
    }
    
    /* Remove seat if it matches */
    ObWaylandSeat *seat;
    wl_list_for_each(seat, &d->seats, link) {
        if (seat->id == id) {
            wl_list_remove(&seat->link);
            g_free(seat->name);
            wl_seat_destroy(seat->wl_seat);
            g_free(seat);
            return;
        }
    }
}

void wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
                                    int32_t x, int32_t y,
                                    int32_t physical_width, 
                                    int32_t physical_height,
                                    int32_t subpixel,
                                    const char *make, const char *model,
                                    int32_t transform)
{
    ObWaylandOutput *output = data;
    
    output->pending_width_mm = physical_width;
    output->pending_height_mm = physical_height;
    output->pending_make = g_strdup(make);
    output->pending_model = g_strdup(model);
    
    output->monitor.geometry.x = x;
    output->monitor.geometry.y = y;
}

void wayland_output_handle_mode(void *data, struct wl_output *wl_output,
                                uint32_t flags, int32_t width, int32_t height,
                                int32_t refresh)
{
    ObWaylandOutput *output = data;
    
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        output->monitor.geometry.width = width;
        output->monitor.geometry.height = height;
        output->monitor.refresh_rate = refresh / 1000; /* mHz to Hz */
    }
}

void wayland_output_handle_done(void *data, struct wl_output *wl_output)
{
    ObWaylandOutput *output = data;
    
    /* Commit pending changes */
    output->monitor.width_mm = output->pending_width_mm;
    output->monitor.height_mm = output->pending_height_mm;
    
    g_free(output->monitor.make);
    output->monitor.make = output->pending_make;
    output->pending_make = NULL;
    
    g_free(output->monitor.model);
    output->monitor.model = output->pending_model;
    output->pending_model = NULL;
    
    output->monitor.connected = TRUE;
    
    /* Generate output change event */
    /* This would be queued for processing by the main loop */
}

void wayland_output_handle_scale(void *data, struct wl_output *wl_output,
                                 int32_t factor)
{
    ObWaylandOutput *output = data;
    output->monitor.scale = factor;
}

void wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
                                        uint32_t capabilities)
{
    ObWaylandSeat *seat = data;
    
    seat->seat.has_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
    seat->seat.has_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
    seat->seat.has_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;
    
    /* Setup pointer if available */
    if (seat->seat.has_pointer && !seat->pointer) {
        seat->pointer = wl_seat_get_pointer(wl_seat);
        /* Add pointer listener here */
    } else if (!seat->seat.has_pointer && seat->pointer) {
        wl_pointer_destroy(seat->pointer);
        seat->pointer = NULL;
    }
    
    /* Setup keyboard if available */
    if (seat->seat.has_keyboard && !seat->keyboard) {
        seat->keyboard = wl_seat_get_keyboard(wl_seat);
        /* Add keyboard listener here */
    } else if (!seat->seat.has_keyboard && seat->keyboard) {
        wl_keyboard_destroy(seat->keyboard);
        seat->keyboard = NULL;
    }
}

void wayland_seat_handle_name(void *data, struct wl_seat *wl_seat,
                              const char *name)
{
    ObWaylandSeat *seat = data;
    g_free(seat->name);
    seat->name = g_strdup(name);
    g_free(seat->seat.name);
    seat->seat.name = g_strdup(name);
}

void wayland_xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                              uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

/* Global Wayland state */
static ObWaylandDisplay *global_wl_display = NULL;

/* Display operations implementation */
static gboolean wayland_display_open_impl(const gchar *name,
                                          ObPlatformDisplay **display)
{
    ObWaylandDisplay *d;
    
    (void)name; /* Wayland uses WAYLAND_DISPLAY env var */
    
    if (global_wl_display) {
        g_warning("Wayland display already open");
        return FALSE;
    }
    
    d = g_new0(ObWaylandDisplay, 1);
    d->base.type = OB_PLATFORM_WAYLAND;
    
    /* Initialize lists */
    wl_list_init(&d->outputs);
    wl_list_init(&d->seats);
    
    /* Connect to Wayland display */
    d->wl_display = wl_display_connect(NULL);
    if (!d->wl_display) {
        g_free(d);
        g_warning("Failed to connect to Wayland display");
        return FALSE;
    }
    
    /* Get registry */
    d->wl_registry = wl_display_get_registry(d->wl_display);
    wl_registry_add_listener(d->wl_registry, &registry_listener, d);
    
    /* Roundtrip to get globals */
    wl_display_roundtrip(d->wl_display);
    
    /* Check required globals */
    if (!d->wl_compositor) {
        g_warning("Wayland compositor not available");
        wayland_display_close_impl((ObPlatformDisplay *)d);
        return FALSE;
    }
    
    if (!d->xdg_wm_base) {
        g_warning("XDG shell not available");
        wayland_display_close_impl((ObPlatformDisplay *)d);
        return FALSE;
    }
    
    global_wl_display = d;
    *display = (ObPlatformDisplay *)d;
    
    return TRUE;
}

static void wayland_display_close_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    
    /* Cleanup outputs */
    ObWaylandOutput *output, *tmp_output;
    wl_list_for_each_safe(output, tmp_output, &d->outputs, link) {
        wl_list_remove(&output->link);
        if (output->wl_output) {
            wl_output_destroy(output->wl_output);
        }
        g_free(output->monitor.name);
        g_free(output->monitor.make);
        g_free(output->monitor.model);
        g_free(output);
    }
    
    /* Cleanup seats */
    ObWaylandSeat *seat, *tmp_seat;
    wl_list_for_each_safe(seat, tmp_seat, &d->seats, link) {
        wl_list_remove(&seat->link);
        if (seat->pointer) {
            wl_pointer_destroy(seat->pointer);
        }
        if (seat->keyboard) {
            wl_keyboard_destroy(seat->keyboard);
        }
        g_free(seat->name);
        g_free(seat->seat.name);
        if (seat->wl_seat) {
            wl_seat_destroy(seat->wl_seat);
        }
        g_free(seat);
    }
    
    /* Cleanup Wayland objects */
    if (d->decoration_manager) {
        zxdg_decoration_manager_v1_destroy(d->decoration_manager);
    }
    if (d->xdg_wm_base) {
        xdg_wm_base_destroy(d->xdg_wm_base);
    }
    if (d->wl_shm) {
        wl_shm_destroy(d->wl_shm);
    }
    if (d->wl_compositor) {
        wl_compositor_destroy(d->wl_compositor);
    }
    if (d->wl_registry) {
        wl_registry_destroy(d->wl_registry);
    }
    if (d->wl_display) {
        wl_display_disconnect(d->wl_display);
    }
    
    g_free(d);
    global_wl_display = NULL;
}

static ObPlatformType wayland_display_get_type_impl(ObPlatformDisplay *display)
{
    (void)display;
    return OB_PLATFORM_WAYLAND;
}

static const gchar *wayland_display_get_name_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    return wl_display_get_name(d->wl_display);
}

static gint wayland_display_get_fd_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    return wl_display_get_fd(d->wl_display);
}

static gboolean wayland_display_prepare_read_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    
    /* Prepare to read events */
    if (wl_display_prepare_read(d->wl_display) == 0) {
        return TRUE;
    }
    
    /* Events already queued */
    return FALSE;
}

static void wayland_display_read_events_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    
    /* Read events from display */
    wl_display_read_events(d->wl_display);
}

static gboolean wayland_display_dispatch_events_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    
    /* Dispatch pending events */
    if (wl_display_dispatch_pending(d->wl_display) < 0) {
        g_warning("Wayland display dispatch error");
        return FALSE;
    }
    
    return TRUE;
}

static guint32 wayland_event_get_time_impl(ObPlatformDisplay *display)
{
    /* Wayland uses millisecond timestamps */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void wayland_sync_impl(ObPlatformDisplay *display)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    
    /* Roundtrip to sync */
    wl_display_roundtrip(d->wl_display);
}

/* Backend interface */
const ObPlatformInterface *wayland_backend_get_interface(void)
{
    static const ObPlatformInterface interface = {
        .display_open = wayland_display_open_impl,
        .display_close = wayland_display_close_impl,
        .display_get_type = wayland_display_get_type_impl,
        .display_get_name = wayland_display_get_name_impl,
        .display_get_fd = wayland_display_get_fd_impl,
        .display_prepare_read = wayland_display_prepare_read_impl,
        .display_read_events = wayland_display_read_events_impl,
        .display_dispatch_events = wayland_display_dispatch_events_impl,
        .event_get_time = wayland_event_get_time_impl,
        .sync = wayland_sync_impl,
        /* Window operations will be added here */
    };
    
    return &interface;
}

gboolean wayland_backend_init(void)
{
    return TRUE;
}

void wayland_backend_shutdown(void)
{
    if (global_wl_display) {
        wayland_display_close_impl((ObPlatformDisplay *)global_wl_display);
    }
}

ObWaylandDisplay *wayland_display_get(void)
{
    return global_wl_display;
}

struct wl_display *wayland_display_get_wl_display(ObPlatformDisplay *display)
{
    return ((ObWaylandDisplay *)display)->wl_display;
}
