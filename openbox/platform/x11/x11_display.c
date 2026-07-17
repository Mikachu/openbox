/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   x11_display.c - X11 Display Operations for PAL
   Copyright (c) 2026  Openbox Development Team

   X11 display backend implementation.
*/

#include "x11_backend.h"
#include "obt/internal.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

/* Extension support headers */
#ifdef XKB
#include <X11/XKBlib.h>
#endif
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#ifdef XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#ifdef SYNC
#include <X11/extensions/sync.h>
#endif

/* Global X11 state */
static ObX11Display *global_display = NULL;
static gboolean x11_error_ignored = FALSE;
static gboolean x11_error_occurred_flag = FALSE;

/* Error handler */
static int x11_error_handler(Display *d, XErrorEvent *e)
{
#ifdef DEBUG
    char errtxt[256];
    XGetErrorText(d, e->error_code, errtxt, 255);
    if (!x11_error_ignored) {
        g_warning("X Error: %s (request %d, minor %d)",
                  errtxt, e->request_code, e->minor_code);
    } else {
        g_debug("Ignoring X Error: %s", errtxt);
    }
#else
    (void)d;
    (void)e;
#endif
    
    x11_error_occurred_flag = TRUE;
    return 0;
}

/* Extension setup */
static void x11_setup_extensions(ObX11Display *d)
{
    int junk, major, minor;
    
    /* XKB */
#ifdef XKB
    major = XkbMajorVersion;
    minor = XkbMinorVersion;
    d->has_xkb = XkbQueryExtension(d->xdisplay, &junk, &d->xkb_event_base,
                                   &junk, &major, &minor);
    if (!d->has_xkb) {
        g_message("XKB extension not available");
    }
#endif
    
    /* Shape */
#ifdef SHAPE
    d->has_shape = XShapeQueryExtension(d->xdisplay, &d->shape_event_base,
                                        &junk);
    if (!d->has_shape) {
        g_message("X Shape extension not available");
    }
#endif
    
    /* Xinerama */
#ifdef XINERAMA
    d->has_xinerama = XineramaQueryExtension(d->xdisplay,
                                               &d->xinerama_event_base,
                                               &junk) &&
                      XineramaIsActive(d->xdisplay);
    if (!d->has_xinerama) {
        g_message("Xinerama extension not available");
    }
#endif
    
    /* XRandR */
#ifdef XRANDR
    d->has_randr = XRRQueryExtension(d->xdisplay, &d->randr_event_base,
                                     &junk);
    if (!d->has_randr) {
        g_message("XRandR extension not available");
    }
#endif
    
    /* XSync */
#ifdef SYNC
    d->has_sync = XSyncQueryExtension(d->xdisplay, &d->sync_event_base,
                                        &junk) &&
                  XSyncInitialize(d->xdisplay, &junk, &junk);
    if (!d->has_sync) {
        g_message("X Sync extension not available");
    }
#endif
}

/* Atom handling */
static void x11_atoms_init(ObX11Display *d)
{
    d->atoms = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    
    /* Pre-fetch common atoms */
    const gchar *common_atoms[] = {
        "WM_PROTOCOLS", "WM_DELETE_WINDOW", "WM_TAKE_FOCUS",
        "WM_STATE", "WM_NAME", "WM_CLASS", "WM_TRANSIENT_FOR",
        "WM_HINTS", "WM_NORMAL_HINTS",
        "_NET_SUPPORTED", "_NET_WM_NAME", "_NET_WM_VISIBLE_NAME",
        "_NET_WM_STATE", "_NET_WM_STATE_MODAL", "_NET_WM_STATE_STICKY",
        "_NET_WM_STATE_MAXIMIZED_VERT", "_NET_WM_STATE_MAXIMIZED_HORZ",
        "_NET_WM_STATE_SHADED", "_NET_WM_STATE_SKIP_TASKBAR",
        "_NET_WM_STATE_SKIP_PAGER", "_NET_WM_STATE_HIDDEN",
        "_NET_WM_STATE_FULLSCREEN", "_NET_WM_STATE_ABOVE",
        "_NET_WM_STATE_BELOW", "_NET_WM_STATE_DEMANDS_ATTENTION",
        "_NET_WM_DESKTOP", "_NET_WM_WINDOW_TYPE",
        "_NET_WM_WINDOW_TYPE_DESKTOP", "_NET_WM_WINDOW_TYPE_DOCK",
        "_NET_WM_WINDOW_TYPE_TOOLBAR", "_NET_WM_WINDOW_TYPE_MENU",
        "_NET_WM_WINDOW_TYPE_UTILITY", "_NET_WM_WINDOW_TYPE_SPLASH",
        "_NET_WM_WINDOW_TYPE_DIALOG", "_NET_WM_WINDOW_TYPE_NORMAL",
        "_NET_WM_ICON", "_NET_WM_ICON_NAME",
        "_NET_ACTIVE_WINDOW", "_NET_CLIENT_LIST", "_NET_CLIENT_LIST_STACKING",
        "_NET_NUMBER_OF_DESKTOPS", "_NET_CURRENT_DESKTOP",
        "_NET_DESKTOP_NAMES", "_NET_DESKTOP_VIEWPORT",
        "_NET_WORKAREA", "_NET_SUPPORTING_WM_CHECK",
        "_NET_CLOSE_WINDOW", "_NET_MOVERESIZE_WINDOW",
        "_NET_WM_MOVERESIZE", "_NET_RESTACK_WINDOW",
        "_NET_REQUEST_FRAME_EXTENTS", "_NET_FRAME_EXTENTS",
        "_NET_WM_PID", "_NET_WM_USER_TIME", "_NET_WM_PING",
        "_NET_WM_SYNC_REQUEST",
        "UTF8_STRING", "COMPOUND_TEXT", "TEXT", "STRING",
        "CLIPBOARD", "PRIMARY", "SECONDARY",
        "_OB_THEME", "_OB_CONFIG_FILE", "_OB_CONTROL",
        NULL
    };
    
    for (int i = 0; common_atoms[i]; i++) {
        Atom atom = XInternAtom(d->xdisplay, common_atoms[i], False);
        g_hash_table_insert(d->atoms, g_strdup(common_atoms[i]),
                           GUINT_TO_POINTER(atom));
    }
}

Atom x11_atom(ObX11Display *display, const gchar *name)
{
    Atom atom;
    gpointer val;
    
    if (g_hash_table_lookup_extended(display->atoms, name, NULL, &val)) {
        return GPOINTER_TO_UINT(val);
    }
    
    /* Not cached, look it up */
    atom = XInternAtom(display->xdisplay, name, False);
    g_hash_table_insert(display->atoms, g_strdup(name),
                       GUINT_TO_POINTER(atom));
    return atom;
}

const gchar *x11_atom_name(ObX11Display *display, Atom atom)
{
    /* This is expensive, use sparingly */
    return XGetAtomName(display->xdisplay, atom);
}

/* Display operations implementation */
static gboolean x11_display_open_impl(const gchar *name,
                                        ObPlatformDisplay **display)
{
    ObX11Display *d;
    
    if (global_display) {
        g_warning("X11 display already open");
        return FALSE;
    }
    
    d = g_new0(ObX11Display, 1);
    d->base.type = OB_PLATFORM_X11;
    
    /* Open X display */
    d->xdisplay = XOpenDisplay(name);
    if (!d->xdisplay) {
        g_free(d);
        g_warning("Failed to open X display: %s", 
                  name ? name : getenv("DISPLAY"));
        return FALSE;
    }
    
    /* Set close-on-exec */
    int fd = ConnectionNumber(d->xdisplay);
    int flags = fcntl(fd, F_GETFD);
    if (flags >= 0) {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }
    
    /* Setup error handler */
    XSetErrorHandler(x11_error_handler);
    
    /* Get screen and root window */
    d->screen = DefaultScreen(d->xdisplay);
    d->root = RootWindow(d->xdisplay, d->screen);
    
    /* Initialize extensions */
    x11_setup_extensions(d);
    
    /* Initialize atoms */
    x11_atoms_init(d);
    
    /* Initialize data structures */
    d->event_queue = g_queue_new();
    d->windows = g_hash_table_new(g_direct_hash, g_direct_equal);
    d->monitors = g_array_new(FALSE, TRUE, sizeof(ObPlatformMonitor *));
    d->seats = g_array_new(FALSE, TRUE, sizeof(ObPlatformSeat *));
    
    /* Create default seat */
    ObX11Seat *seat = g_new0(ObX11Seat, 1);
    seat->base.name = g_strdup("seat0");
    seat->base.has_pointer = TRUE;
    seat->base.has_keyboard = TRUE;
    seat->pointer_id = -1;
    seat->keyboard_id = -1;
    g_array_append_val(d->seats, seat);
    
    global_display = d;
    *display = (ObPlatformDisplay *)d;
    
    return TRUE;
}

static void x11_display_close_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    
    if (d != global_display) {
        g_warning("Attempting to close non-current display");
        return;
    }
    
    /* Cleanup */
    if (d->atoms) {
        g_hash_table_destroy(d->atoms);
    }
    if (d->event_queue) {
        g_queue_free(d->event_queue);
    }
    if (d->windows) {
        g_hash_table_destroy(d->windows);
    }
    
    /* Free monitors */
    for (guint i = 0; i < d->monitors->len; i++) {
        ObPlatformMonitor *m = g_array_index(d->monitors, ObPlatformMonitor *, i);
        g_free(m->name);
        g_free(m->make);
        g_free(m->model);
        g_free(m);
    }
    g_array_free(d->monitors, TRUE);
    
    /* Free seats */
    for (guint i = 0; i < d->seats->len; i++) {
        ObX11Seat *s = g_array_index(d->seats, ObX11Seat *, i);
        g_free(s->base.name);
        g_free(s);
    }
    g_array_free(d->seats, TRUE);
    
    /* Close X display */
    if (d->xdisplay) {
        XCloseDisplay(d->xdisplay);
    }
    
    g_free(d);
    global_display = NULL;
}

static ObPlatformType x11_display_get_type_impl(ObPlatformDisplay *display)
{
    (void)display;
    return OB_PLATFORM_X11;
}

static const gchar *x11_display_get_name_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    return DisplayString(d->xdisplay);
}

static gint x11_display_get_fd_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    return ConnectionNumber(d->xdisplay);
}

static gboolean x11_display_prepare_read_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    return XPending(d->xdisplay) > 0;
}

static void x11_display_read_events_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    XEvent xevent;
    
    while (XPending(d->xdisplay) > 0) {
        XNextEvent(d->xdisplay, &xevent);
        
        /* Convert and queue event */
        ObPlatformEvent *event = g_new0(ObPlatformEvent, 1);
        if (x11_event_translate(&xevent, event)) {
            g_queue_push_tail(d->event_queue, event);
        } else {
            g_free(event);
        }
    }
}

static gboolean x11_display_dispatch_events_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    gboolean handled = FALSE;
    
    while (!g_queue_is_empty(d->event_queue)) {
        ObPlatformEvent *event = g_queue_pop_head(d->event_queue);
        
        /* Process event - this would call into the main event handler */
        /* For now, just mark as handled */
        handled = TRUE;
        
        g_free(event);
    }
    
    return handled;
}

static guint32 x11_event_get_time_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    return d->last_event_time;
}

static void x11_sync_impl(ObPlatformDisplay *display)
{
    ObX11Display *d = (ObX11Display *)display;
    XSync(d->xdisplay, False);
}

/* Error handling */
void x11_ignore_errors(gboolean ignore)
{
    XSync(global_display->xdisplay, False);
    x11_error_ignored = ignore;
    if (ignore) {
        x11_error_occurred_flag = FALSE;
    }
}

gboolean x11_error_occurred(void)
{
    return x11_error_occurred_flag;
}

void x11_clear_error(void)
{
    x11_error_occurred_flag = FALSE;
}

/* Accessors */
ObX11Display *x11_display_get(void)
{
    return global_display;
}

Display *x11_display_get_xdisplay(ObPlatformDisplay *display)
{
    return ((ObX11Display *)display)->xdisplay;
}

/* Backend interface */
const ObPlatformInterface *x11_backend_get_interface(void)
{
    static const ObPlatformInterface interface = {
        .display_open = x11_display_open_impl,
        .display_close = x11_display_close_impl,
        .display_get_type = x11_display_get_type_impl,
        .display_get_name = x11_display_get_name_impl,
        .display_get_fd = x11_display_get_fd_impl,
        .display_prepare_read = x11_display_prepare_read_impl,
        .display_read_events = x11_display_read_events_impl,
        .display_dispatch_events = x11_display_dispatch_events_impl,
        .event_get_time = x11_event_get_time_impl,
        .sync = x11_sync_impl,
        /* Window operations will be added here */
    };
    
    return &interface;
}

gboolean x11_backend_init(void)
{
    /* Nothing special needed for init */
    return TRUE;
}

void x11_backend_shutdown(void)
{
    if (global_display) {
        x11_display_close_impl((ObPlatformDisplay *)global_display);
    }
}
