/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   wayland_backend.h - Wayland Backend for Platform Abstraction Layer
   Copyright (c) 2026  Openbox Development Team

   Wayland-specific implementation of the platform abstraction layer.
*/

#ifndef __wayland_backend_h
#define __wayland_backend_h

#include "../ob_platform.h"

/* Wayland headers */
#include <wayland-client.h>
#include <wayland-cursor.h>

/* Protocol headers (generated from wayland.xml) */
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

G_BEGIN_DECLS

/* Wayland-specific display structure */
typedef struct _ObWaylandDisplay {
    ObPlatformDisplay base;
    
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_compositor *wl_compositor;
    struct wl_shm *wl_shm;
    
    /* XDG Shell */
    struct xdg_wm_base *xdg_wm_base;
    struct zxdg_decoration_manager_v1 *decoration_manager;
    
    /* Outputs (monitors) */
    struct wl_list outputs;  /* ObWaylandOutput::link */
    guint output_count;
    
    /* Seats (input) */
    struct wl_list seats;  /* ObWaylandSeat::link */
    
    /* Event handling */
    struct wl_event_queue *event_queue;
    
    /* State */
    gboolean roundtrip_needed;
    
} ObWaylandDisplay;

/* Wayland output (monitor) */
typedef struct _ObWaylandOutput {
    struct wl_list link;
    
    struct wl_output *wl_output;
    ObWaylandDisplay *display;
    
    /* Platform monitor data */
    ObPlatformMonitor monitor;
    
    /* Wayland output info */
    guint32 id;
    gint version;
    
    /* Pending state */
    gchar *pending_make;
    gchar *pending_model;
    gint pending_width_mm;
    gint pending_height_mm;
    
} ObWaylandOutput;

/* Wayland seat */
typedef struct _ObWaylandSeat {
    struct wl_list link;
    
    struct wl_seat *wl_seat;
    ObWaylandDisplay *display;
    
    /* Platform seat data */
    ObPlatformSeat seat;
    
    /* Wayland seat info */
    guint32 id;
    gint version;
    gchar *name;
    
    /* Input devices */
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    struct wl_touch *touch;
    
    /* Cursor */
    struct wl_cursor_theme *cursor_theme;
    struct wl_cursor *current_cursor;
    struct wl_surface *cursor_surface;
    
    /* Pointer state */
    gint pointer_x;
    gint pointer_y;
    guint32 pointer_enter_serial;
    
    /* Keyboard state */
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;
    
} ObWaylandSeat;

/* Wayland-specific window structure */
typedef struct _ObWaylandWindow {
    ObPlatformWindow base;
    
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;
    
    ObWaylandDisplay *display;
    
    /* Parent for transient windows */
    struct wl_surface *parent_surface;
    
    /* State */
    gboolean configured;
    gboolean active;
    gboolean maximized;
    gboolean minimized;
    gboolean fullscreen;
    
    /* Pending size from compositor */
    gint pending_width;
    gint pending_height;
    
} ObWaylandWindow;

/* Wayland-specific surface (for frames) */
typedef struct _ObWaylandSurface {
    ObPlatformSurface base;
    
    struct wl_surface *wl_surface;
    struct wl_subsurface *wl_subsurface;
    
    ObWaylandWindow *client;
    
    /* Position relative to parent */
    gint x, y;
    
} ObWaylandSurface;

/* Wayland backend initialization */
gboolean wayland_backend_init(void);
void wayland_backend_shutdown(void);

/* Get the Wayland interface implementation */
const ObPlatformInterface *wayland_backend_get_interface(void);

/* Utility functions */
ObWaylandDisplay *wayland_display_get(void);
struct wl_display *wayland_display_get_wl_display(ObPlatformDisplay *display);

/* Registry callbacks */
void wayland_registry_handle_global(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version);
void wayland_registry_handle_global_remove(void *data, 
                                           struct wl_registry *registry,
                                           uint32_t id);

/* Output callbacks */
void wayland_output_handle_geometry(void *data, struct wl_output *wl_output,
                                    int32_t x, int32_t y,
                                    int32_t physical_width, 
                                    int32_t physical_height,
                                    int32_t subpixel,
                                    const char *make, const char *model,
                                    int32_t transform);
void wayland_output_handle_mode(void *data, struct wl_output *wl_output,
                                uint32_t flags, int32_t width, int32_t height,
                                int32_t refresh);
void wayland_output_handle_done(void *data, struct wl_output *wl_output);
void wayland_output_handle_scale(void *data, struct wl_output *wl_output,
                                 int32_t factor);

/* Seat callbacks */
void wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
                                      uint32_t capabilities);
void wayland_seat_handle_name(void *data, struct wl_seat *wl_seat,
                              const char *name);

/* XDG shell callbacks */
void wayland_xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                              uint32_t serial);

G_END_DECLS

#endif /* __wayland_backend_h */
