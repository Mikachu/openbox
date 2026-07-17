/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   wayland_window.c - Wayland Window Operations for PAL
   Copyright (c) 2026  Openbox Development Team

   Wayland window backend implementation (stub for future completion).
*/

#include "wayland_backend.h"

/* XDG surface listener */
static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                   uint32_t serial)
{
    ObWaylandWindow *w = data;
    
    xdg_surface_ack_configure(xdg_surface, serial);
    w->configured = TRUE;
    
    /* Commit the surface */
    if (w->wl_surface) {
        wl_surface_commit(w->wl_surface);
    }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

/* XDG toplevel listener */
static void xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states)
{
    ObWaylandWindow *w = data;
    
    /* Store pending size */
    if (width > 0 && height > 0) {
        w->pending_width = width;
        w->pending_height = height;
    }
    
    /* Parse states */
    w->maximized = FALSE;
    w->fullscreen = FALSE;
    w->active = FALSE;
    w->minimized = FALSE;
    
    uint32_t *state;
    wl_array_for_each(state, states) {
        switch (*state) {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                w->maximized = TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                w->fullscreen = TRUE;
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                w->active = TRUE;
                break;
        }
    }
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
    ObWaylandWindow *w = data;
    
    /* Request to close the window */
    /* This would emit a close event to be handled by the WM */
    (void)w;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

/* Window operations implementation */
static gboolean wayland_window_create_impl(ObPlatformDisplay *display,
                                           ObPlatformWindow *parent,
                                           ObWindowAttributes *attrs,
                                           ObPlatformWindow **window)
{
    ObWaylandDisplay *d = (ObWaylandDisplay *)display;
    ObWaylandWindow *w;
    
    if (!d->wl_compositor || !d->xdg_wm_base) {
        g_warning("Wayland compositor or XDG shell not available");
        return FALSE;
    }
    
    w = g_new0(ObWaylandWindow, 1);
    w->display = d;
    
    /* Create Wayland surface */
    w->wl_surface = wl_compositor_create_surface(d->wl_compositor);
    if (!w->wl_surface) {
        g_free(w);
        return FALSE;
    }
    
    /* Create XDG surface */
    w->xdg_surface = xdg_wm_base_get_xdg_surface(d->xdg_wm_base, w->wl_surface);
    if (!w->xdg_surface) {
        wl_surface_destroy(w->wl_surface);
        g_free(w);
        return FALSE;
    }
    
    xdg_surface_add_listener(w->xdg_surface, &xdg_surface_listener, w);
    
    /* Create toplevel */
    w->xdg_toplevel = xdg_surface_get_toplevel(w->xdg_surface);
    if (!w->xdg_toplevel) {
        xdg_surface_destroy(w->xdg_surface);
        wl_surface_destroy(w->wl_surface);
        g_free(w);
        return FALSE;
    }
    
    xdg_toplevel_add_listener(w->xdg_toplevel, &xdg_toplevel_listener, w);
    
    /* Set initial properties */
    if (attrs->title) {
        xdg_toplevel_set_title(w->xdg_toplevel, attrs->title);
    }
    
    /* Set parent for transient windows */
    if (parent) {
        ObWaylandWindow *p = (ObWaylandWindow *)parent;
        if (p->xdg_toplevel) {
            xdg_toplevel_set_parent(w->xdg_toplevel, p->xdg_toplevel);
        }
    }
    
    /* Commit to apply initial state */
    wl_surface_commit(w->wl_surface);
    
    /* Wait for configure */
    wl_display_roundtrip(d->wl_display);
    
    *window = (ObPlatformWindow *)w;
    return TRUE;
}

static void wayland_window_destroy_impl(ObPlatformWindow *window)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    if (!w) return;
    
    if (w->decoration) {
        zxdg_toplevel_decoration_v1_destroy(w->decoration);
    }
    if (w->xdg_toplevel) {
        xdg_toplevel_destroy(w->xdg_toplevel);
    }
    if (w->xdg_surface) {
        xdg_surface_destroy(w->xdg_surface);
    }
    if (w->wl_surface) {
        wl_surface_destroy(w->wl_surface);
    }
    
    g_free(w);
}

static void wayland_window_show_impl(ObPlatformWindow *window)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    /* In Wayland, windows are shown by committing the surface */
    if (w->wl_surface && w->configured) {
        wl_surface_commit(w->wl_surface);
    }
}

static void wayland_window_hide_impl(ObPlatformWindow *window)
{
    /* Wayland doesn't have a direct hide equivalent */
    /* Could destroy surface or minimize */
    (void)window;
}

static void wayland_window_raise_impl(ObPlatformWindow *window)
{
    /* Wayland doesn't support explicit raise */
    /* Could use activation protocol */
    (void)window;
}

static void wayland_window_lower_impl(ObPlatformWindow *window)
{
    /* Wayland doesn't support explicit lower */
    (void)window;
}

static void wayland_window_move_impl(ObPlatformWindow *window, gint x, gint y)
{
    /* Wayland clients don't set their own position */
    /* This is controlled by the compositor */
    (void)window;
    (void)x;
    (void)y;
}

static void wayland_window_resize_impl(ObPlatformWindow *window,
                                        gint width, gint height)
{
    /* In Wayland, the compositor decides the size */
    /* We can only suggest a size */
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    if (w->xdg_toplevel) {
        /* Set size hints if supported */
        /* xdg_toplevel_set_min_size/max_size */
    }
    
    (void)width;
    (void)height;
}

static void wayland_window_move_resize_impl(ObPlatformWindow *window,
                                            gint x, gint y,
                                            gint width, gint height)
{
    /* Combined operation - see individual notes above */
    wayland_window_move_impl(window, x, y);
    wayland_window_resize_impl(window, width, height);
}

static void wayland_window_set_title_impl(ObPlatformWindow *window,
                                           const gchar *title)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    if (w->xdg_toplevel) {
        xdg_toplevel_set_title(w->xdg_toplevel, title);
    }
}

static void wayland_window_set_icon_impl(ObPlatformWindow *window,
                                          gint width, gint height,
                                          const guchar *data)
{
    /* Wayland icon support varies by compositor */
    /* Could use xdg-toplevel-icon protocol in future */
    (void)window;
    (void)width;
    (void)height;
    (void)data;
}

static void wayland_window_set_state_impl(ObPlatformWindow *window,
                                           ObWindowState state)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    if (!w->xdg_toplevel) return;
    
    if (state & OB_WINDOW_STATE_MAXIMIZED_VERT ||
        state & OB_WINDOW_STATE_MAXIMIZED_HORZ) {
        xdg_toplevel_set_maximized(w->xdg_toplevel);
    } else {
        xdg_toplevel_unset_maximized(w->xdg_toplevel);
    }
    
    if (state & OB_WINDOW_STATE_FULLSCREEN) {
        xdg_toplevel_set_fullscreen(w->xdg_toplevel, NULL);
    } else {
        xdg_toplevel_unset_fullimized(w->xdg_toplevel);
    }
}

static void wayland_window_set_desktop_impl(ObPlatformWindow *window,
                                             gint desktop)
{
    /* Wayland doesn't have traditional desktops */
    /* Could use workspace protocol in future */
    (void)window;
    (void)desktop;
}

static void wayland_window_focus_impl(ObPlatformWindow *window)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    /* Request activation */
    if (w->xdg_toplevel) {
        /* Use xdg-activation protocol if available */
    }
}

static gboolean wayland_window_get_attributes_impl(ObPlatformWindow *window,
                                                  ObWindowAttributes *attrs)
{
    ObWaylandWindow *w = (ObWaylandWindow *)window;
    
    memset(attrs, 0, sizeof(ObWindowAttributes));
    
    /* Return pending/current size */
    attrs->width = w->pending_width;
    attrs->height = w->pending_height;
    attrs->mapped = w->configured;
    
    return TRUE;
}

static void wayland_window_attributes_free_impl(ObWindowAttributes *attrs)
{
    if (!attrs) return;
    
    g_free(attrs->title);
    g_free(attrs->name);
    g_free(attrs->class);
    g_free(attrs->role);
    g_free(attrs->icon_name);
    g_free(attrs->icon_data);
    
    memset(attrs, 0, sizeof(ObWindowAttributes));
}

/* Update the interface with window functions */
const ObPlatformInterface *wayland_backend_get_interface_complete(void)
{
    static ObPlatformInterface iface;
    static gboolean initialized = FALSE;
    
    if (!initialized) {
        /* Copy base interface */
        iface = *wayland_backend_get_interface();
        
        /* Add window functions */
        iface.window_create = wayland_window_create_impl;
        iface.window_destroy = wayland_window_destroy_impl;
        iface.window_show = wayland_window_show_impl;
        iface.window_hide = wayland_window_hide_impl;
        iface.window_raise = wayland_window_raise_impl;
        iface.window_lower = wayland_window_lower_impl;
        iface.window_move = wayland_window_move_impl;
        iface.window_resize = wayland_window_resize_impl;
        iface.window_move_resize = wayland_window_move_resize_impl;
        iface.window_set_title = wayland_window_set_title_impl;
        iface.window_set_icon = wayland_window_set_icon_impl;
        iface.window_set_state = wayland_window_set_state_impl;
        iface.window_set_desktop = wayland_window_set_desktop_impl;
        iface.window_focus = wayland_window_focus_impl;
        iface.window_get_attributes = wayland_window_get_attributes_impl;
        iface.window_attributes_free = wayland_window_attributes_free_impl;
        
        initialized = TRUE;
    }
    
    return &iface;
}
