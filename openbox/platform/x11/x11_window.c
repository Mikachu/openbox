/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   x11_window.c - X11 Window Operations for PAL
   Copyright (c) 2026  Openbox Development Team

   X11 window backend implementation.
*/

#include "x11_backend.h"
#include <X11/cursorfont.h>

/* Window operations implementation */
static gboolean x11_window_create_impl(ObPlatformDisplay *display,
                                       ObPlatformWindow *parent,
                                       ObWindowAttributes *attrs,
                                       ObPlatformWindow **window)
{
    ObX11Display *d = (ObX11Display *)display;
    ObX11Window *w;
    Window xparent;
    XSetWindowAttributes xattrs;
    unsigned long xattr_mask;
    
    w = g_new0(ObX11Window, 1);
    w->display = d;
    
    /* Determine parent window */
    if (parent) {
        xparent = ((ObX11Window *)parent)->xwindow;
    } else {
        xparent = d->root;
    }
    
    /* Setup window attributes */
    xattrs.background_pixel = BlackPixel(d->xdisplay, d->screen);
    xattrs.border_pixel = BlackPixel(d->xdisplay, d->screen);
    xattrs.event_mask = StructureNotifyMask | PropertyChangeMask |
                       EnterWindowMask | LeaveWindowMask |
                       FocusChangeMask | ExposureMask;
    xattr_mask = CWBackPixel | CWBorderPixel | CWEventMask;
    
    /* Create the X window */
    w->xwindow = XCreateWindow(
        d->xdisplay,
        xparent,
        attrs->x, attrs->y,
        attrs->width, attrs->height,
        attrs->border_width,
        CopyFromParent,           /* depth */
        InputOutput,              /* class */
        CopyFromParent,           /* visual */
        xattr_mask,
        &xattrs
    );
    
    if (!w->xwindow) {
        g_free(w);
        return FALSE;
    }
    
    /* Store in window hash */
    g_hash_table_insert(d->windows, GUINT_TO_POINTER(w->xwindow), w);
    
    /* Set window properties */
    if (attrs->title) {
        XStoreName(d->xdisplay, w->xwindow, attrs->title);
        Atom utf8_string = x11_atom(d, "UTF8_STRING");
        Atom net_wm_name = x11_atom(d, "_NET_WM_NAME");
        XChangeProperty(d->xdisplay, w->xwindow, net_wm_name, utf8_string,
                       8, PropModeReplace,
                       (unsigned char *)attrs->title, strlen(attrs->title));
    }
    
    if (attrs->class) {
        XClassHint hints;
        hints.res_name = attrs->name ? (char *)attrs->name : (char *)"openbox";
        hints.res_class = (char *)attrs->class;
        XSetClassHint(d->xdisplay, w->xwindow, &hints);
    }
    
    /* Set window type */
    Atom window_type = None;
    switch (attrs->type) {
        case OB_WINDOW_TYPE_NORMAL:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_NORMAL");
            break;
        case OB_WINDOW_TYPE_DOCK:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_DOCK");
            break;
        case OB_WINDOW_TYPE_DESKTOP:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_DESKTOP");
            break;
        case OB_WINDOW_TYPE_DIALOG:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_DIALOG");
            break;
        case OB_WINDOW_TYPE_MENU:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_MENU");
            break;
        case OB_WINDOW_TYPE_TOOLBAR:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_TOOLBAR");
            break;
        case OB_WINDOW_TYPE_SPLASH:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_SPLASH");
            break;
        case OB_WINDOW_TYPE_UTILITY:
            window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE_UTILITY");
            break;
        default:
            break;
    }
    
    if (window_type != None) {
        Atom net_wm_window_type = x11_atom(d, "_NET_WM_WINDOW_TYPE");
        XChangeProperty(d->xdisplay, w->xwindow, net_wm_window_type, XA_ATOM,
                       32, PropModeReplace,
                       (unsigned char *)&window_type, 1);
    }
    
    /* Set transient for */
    if (attrs->transient_for) {
        Window transient_xwindow = ((ObX11Window *)attrs->transient_for)->xwindow;
        XSetTransientForHint(d->xdisplay, w->xwindow, transient_xwindow);
    }
    
    *window = (ObPlatformWindow *)w;
    return TRUE;
}

static void x11_window_destroy_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    if (!w || !w->xwindow) return;
    
    /* Remove from hash table */
    g_hash_table_remove(d->windows, GUINT_TO_POINTER(w->xwindow));
    
    /* Destroy X window */
    XDestroyWindow(d->xdisplay, w->xwindow);
    
    g_free(w);
}

static void x11_window_show_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    XMapWindow(w->display->xdisplay, w->xwindow);
    w->mapped = TRUE;
}

static void x11_window_hide_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    XUnmapWindow(w->display->xdisplay, w->xwindow);
    w->mapped = FALSE;
}

static void x11_window_raise_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    XRaiseWindow(w->display->xdisplay, w->xwindow);
}

static void x11_window_lower_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    XLowerWindow(w->display->xdisplay, w->xwindow);
}

static void x11_window_move_impl(ObPlatformWindow *window, gint x, gint y)
{
    ObX11Window *w = (ObX11Window *)window;
    XMoveWindow(w->display->xdisplay, w->xwindow, x, y);
}

static void x11_window_resize_impl(ObPlatformWindow *window, 
                                    gint width, gint height)
{
    ObX11Window *w = (ObX11Window *)window;
    XResizeWindow(w->display->xdisplay, w->xwindow, width, height);
}

static void x11_window_move_resize_impl(ObPlatformWindow *window,
                                        gint x, gint y,
                                        gint width, gint height)
{
    ObX11Window *w = (ObX11Window *)window;
    XMoveResizeWindow(w->display->xdisplay, w->xwindow, x, y, width, height);
}

static void x11_window_set_title_impl(ObPlatformWindow *window, 
                                       const gchar *title)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    XStoreName(d->xdisplay, w->xwindow, title);
    
    Atom utf8_string = x11_atom(d, "UTF8_STRING");
    Atom net_wm_name = x11_atom(d, "_NET_WM_NAME");
    XChangeProperty(d->xdisplay, w->xwindow, net_wm_name, utf8_string,
                   8, PropModeReplace,
                   (unsigned char *)title, strlen(title));
}

static void x11_window_set_icon_impl(ObPlatformWindow *window,
                                      gint width, gint height,
                                      const guchar *data)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    /* Create ARGB icon data for _NET_WM_ICON */
    gulong *icon_data = g_new(gulong, 2 + width * height);
    icon_data[0] = width;
    icon_data[1] = height;
    
    /* Copy pixel data (convert from RGBA to ARGB if needed) */
    memcpy(icon_data + 2, data, width * height * sizeof(gulong));
    
    Atom net_wm_icon = x11_atom(d, "_NET_WM_ICON");
    Atom cardinal = x11_atom(d, "CARDINAL");
    XChangeProperty(d->xdisplay, w->xwindow, net_wm_icon, cardinal,
                   32, PropModeReplace,
                   (unsigned char *)icon_data, 2 + width * height);
    
    g_free(icon_data);
}

static void x11_window_set_state_impl(ObPlatformWindow *window, 
                                       ObWindowState state)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    /* Build state atoms array */
    Atom states[32];
    int nstates = 0;
    
    if (state & OB_WINDOW_STATE_MODAL)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_MODAL");
    if (state & OB_WINDOW_STATE_STICKY)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_STICKY");
    if (state & OB_WINDOW_STATE_MAXIMIZED_VERT)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_MAXIMIZED_VERT");
    if (state & OB_WINDOW_STATE_MAXIMIZED_HORZ)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_MAXIMIZED_HORZ");
    if (state & OB_WINDOW_STATE_SHADED)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_SHADED");
    if (state & OB_WINDOW_STATE_SKIP_TASKBAR)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_SKIP_TASKBAR");
    if (state & OB_WINDOW_STATE_SKIP_PAGER)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_SKIP_PAGER");
    if (state & OB_WINDOW_STATE_HIDDEN)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_HIDDEN");
    if (state & OB_WINDOW_STATE_FULLSCREEN)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_FULLSCREEN");
    if (state & OB_WINDOW_STATE_ABOVE)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_ABOVE");
    if (state & OB_WINDOW_STATE_BELOW)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_BELOW");
    if (state & OB_WINDOW_STATE_DEMANDS_ATTENTION)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_DEMANDS_ATTENTION");
    if (state & OB_WINDOW_STATE_FOCUSED)
        states[nstates++] = x11_atom(d, "_NET_WM_STATE_FOCUSED");
    
    Atom net_wm_state = x11_atom(d, "_NET_WM_STATE");
    XChangeProperty(d->xdisplay, w->xwindow, net_wm_state, XA_ATOM,
                   32, PropModeReplace,
                   (unsigned char *)states, nstates);
}

static void x11_window_set_desktop_impl(ObPlatformWindow *window, gint desktop)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    Atom net_wm_desktop = x11_atom(d, "_NET_WM_DESKTOP");
    Atom cardinal = x11_atom(d, "CARDINAL");
    
    unsigned long desk = desktop;
    XChangeProperty(d->xdisplay, w->xwindow, net_wm_desktop, cardinal,
                   32, PropModeReplace,
                   (unsigned char *)&desk, 1);
}

static void x11_window_focus_impl(ObPlatformWindow *window)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    
    XSetInputFocus(d->xdisplay, w->xwindow, RevertToParent, CurrentTime);
    
    /* Also set as active window */
    Atom net_active_window = x11_atom(d, "_NET_ACTIVE_WINDOW");
    Atom window_type = x11_atom(d, "WINDOW");
    
    Window root = RootWindow(d->xdisplay, d->screen);
    XChangeProperty(d->xdisplay, root, net_active_window, window_type,
                   32, PropModeReplace,
                   (unsigned char *)&w->xwindow, 1);
}

static gboolean x11_window_get_attributes_impl(ObPlatformWindow *window,
                                                ObWindowAttributes *attrs)
{
    ObX11Window *w = (ObX11Window *)window;
    ObX11Display *d = w->display;
    XWindowAttributes xattrs;
    
    if (!XGetWindowAttributes(d->xdisplay, w->xwindow, &xattrs)) {
        return FALSE;
    }
    
    attrs->x = xattrs.x;
    attrs->y = xattrs.y;
    attrs->width = xattrs.width;
    attrs->height = xattrs.height;
    attrs->border_width = xattrs.border_width;
    attrs->mapped = (xattrs.map_state != IsUnmapped);
    
    /* Get title */
    char *title = NULL;
    Atom utf8_string = x11_atom(d, "UTF8_STRING");
    Atom net_wm_name = x11_atom(d, "_NET_WM_NAME");
    
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    
    if (XGetWindowProperty(d->xdisplay, w->xwindow, net_wm_name,
                          0, 1024, False, utf8_string,
                          &actual_type, &actual_format,
                          &nitems, &bytes_after, &prop) == Success) {
        if (prop) {
            attrs->title = g_strdup((char *)prop);
            XFree(prop);
        }
    }
    
    if (!attrs->title) {
        /* Fallback to WM_NAME */
        XFetchName(d->xdisplay, w->xwindow, &title);
        if (title) {
            attrs->title = g_strdup(title);
            XFree(title);
        }
    }
    
    /* Get class hint */
    XClassHint class_hint;
    if (XGetClassHint(d->xdisplay, w->xwindow, &class_hint)) {
        attrs->name = g_strdup(class_hint.res_name);
        attrs->class = g_strdup(class_hint.res_class);
        XFree(class_hint.res_name);
        XFree(class_hint.res_class);\n    }
    
    return TRUE;
}

static void x11_window_attributes_free_impl(ObWindowAttributes *attrs)
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

/* Utility accessors */
Window x11_window_get_xwindow(ObPlatformWindow *window)
{
    return ((ObX11Window *)window)->xwindow;
}

ObPlatformWindow *x11_window_from_xid(Window xwindow)
{
    if (!global_display) return NULL;
    return g_hash_table_lookup(global_display->windows, 
                               GUINT_TO_POINTER(xwindow));
}

/* Update the interface with window functions */
const ObPlatformInterface *x11_backend_get_interface_complete(void)
{
    static ObPlatformInterface iface;
    static gboolean initialized = FALSE;
    
    if (!initialized) {
        /* Copy base interface */
        iface = *x11_backend_get_interface();
        
        /* Add window functions */
        iface.window_create = x11_window_create_impl;
        iface.window_destroy = x11_window_destroy_impl;
        iface.window_show = x11_window_show_impl;
        iface.window_hide = x11_window_hide_impl;
        iface.window_raise = x11_window_raise_impl;
        iface.window_lower = x11_window_lower_impl;
        iface.window_move = x11_window_move_impl;
        iface.window_resize = x11_window_resize_impl;
        iface.window_move_resize = x11_window_move_resize_impl;
        iface.window_set_title = x11_window_set_title_impl;
        iface.window_set_icon = x11_window_set_icon_impl;
        iface.window_set_state = x11_window_set_state_impl;
        iface.window_set_desktop = x11_window_set_desktop_impl;
        iface.window_focus = x11_window_focus_impl;
        iface.window_get_attributes = x11_window_get_attributes_impl;
        iface.window_attributes_free = x11_window_attributes_free_impl;
        
        initialized = TRUE;
    }
    
    return &iface;
}
