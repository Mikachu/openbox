/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   x11_backend.h - X11 Backend for Platform Abstraction Layer
   Copyright (c) 2026  Openbox Development Team

   X11-specific implementation of the platform abstraction layer.
*/

#ifndef __x11_backend_h
#define __x11_backend_h

#include "../ob_platform.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

G_BEGIN_DECLS

/* X11-specific display structure */
typedef struct _ObX11Display {
    ObPlatformDisplay base;
    
    Display *xdisplay;
    gint screen;
    Window root;
    
    /* X11 extensions */
    gboolean has_xkb;
    gboolean has_shape;
    gboolean has_xinerama;
    gboolean has_randr;
    gboolean has_sync;
    
    gint xkb_event_base;
    gint shape_event_base;
    gint xinerama_event_base;
    gint randr_event_base;
    gint sync_event_base;
    
    /* Atoms cache */
    GHashTable *atoms;
    
    /* Event handling */
    GQueue *event_queue;
    guint32 last_event_time;
    
    /* Windows hash table: XID -> ObX11Window* */
    GHashTable *windows;
    
    /* Monitors */
    GArray *monitors;
    
    /* Seats (X11 has one seat typically) */
    GArray *seats;
    
} ObX11Display;

/* X11-specific window structure */
typedef struct _ObX11Window {
    ObPlatformWindow base;
    
    Window xwindow;
    ObX11Display *display;
    
    /* Window attributes cache */
    XWindowAttributes xattrs;
    gboolean xattrs_valid;
    
    /* Frame window (if decorated) */
    Window frame_window;
    
    /* Event masks */
    glong event_mask;
    
    /* State */
    gboolean mapped;
    gboolean managed;
    
} ObX11Window;

/* X11-specific surface (for frames) */
typedef struct _ObX11Surface {
    ObPlatformSurface base;
    
    Window xwindow;
    ObX11Window *client;
    
    /* Child windows for decorations */
    Window titlebar;
    Window handle;
    Window close_btn;
    Window max_btn;
    Window iconify_btn;
    
} ObX11Surface;

/* X11-specific seat */
typedef struct _ObX11Seat {
    ObPlatformSeat base;
    
    gint pointer_x;
    gint pointer_y;
    
    /* X11 input device IDs */
    gint pointer_id;
    gint keyboard_id;
    
} ObX11Seat;

/* X11 backend initialization */
gboolean x11_backend_init(void);
void x11_backend_shutdown(void);

/* Get the X11 interface implementation */
const ObPlatformInterface *x11_backend_get_interface(void);

/* Utility functions for internal use */
ObX11Display *x11_display_get(void);
Display *x11_display_get_xdisplay(ObPlatformDisplay *display);
Window x11_window_get_xwindow(ObPlatformWindow *window);
ObPlatformWindow *x11_window_from_xid(Window xwindow);

/* Atom handling */
Atom x11_atom(ObX11Display *display, const gchar *name);
const gchar *x11_atom_name(ObX11Display *display, Atom atom);

/* Error handling */
void x11_ignore_errors(gboolean ignore);
gboolean x11_error_occurred(void);
void x11_clear_error(void);

/* Event conversion */
gboolean x11_event_translate(XEvent *xevent, ObPlatformEvent *event);
void x11_event_send(ObX11Window *window, XEvent *event);

/* X11-specific extensions */
#ifdef SHAPE
void x11_shape_combine(ObX11Window *window, gint kind);
#endif

G_END_DECLS

#endif /* __x11_backend_h */
