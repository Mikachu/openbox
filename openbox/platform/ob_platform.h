/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   ob_platform.h - Platform Abstraction Layer for Openbox
   Copyright (c) 2026  Openbox Development Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Platform Abstraction Layer (PAL) - Provides unified interface for
   X11 and Wayland backends.
*/

#ifndef __ob_platform_h
#define __ob_platform_h

#include <glib.h>
#include "obrender/geom.h"

G_BEGIN_DECLS

/* Forward declarations for platform-specific types */
typedef struct _ObPlatformDisplay ObPlatformDisplay;
typedef struct _ObPlatformWindow ObPlatformWindow;
typedef struct _ObPlatformSurface ObPlatformSurface;
typedef struct _ObPlatformSeat ObPlatformSeat;
typedef struct _ObPlatformMonitor ObPlatformMonitor;
typedef struct _ObPlatformEvent ObPlatformEvent;

/* Platform backend type */
typedef enum {
    OB_PLATFORM_X11,
    OB_PLATFORM_WAYLAND,
    OB_PLATFORM_NONE
} ObPlatformType;

/* Event types - platform agnostic */
typedef enum {
    OB_EVENT_UNKNOWN = 0,
    OB_EVENT_EXPOSE,
    OB_EVENT_CONFIGURE,
    OB_EVENT_MAP,
    OB_EVENT_UNMAP,
    OB_EVENT_DESTROY,
    OB_EVENT_FOCUS_IN,
    OB_EVENT_FOCUS_OUT,
    OB_EVENT_ENTER,
    OB_EVENT_LEAVE,
    OB_EVENT_BUTTON_PRESS,
    OB_EVENT_BUTTON_RELEASE,
    OB_EVENT_MOTION,
    OB_EVENT_KEY_PRESS,
    OB_EVENT_KEY_RELEASE,
    OB_EVENT_PROPERTY,
    OB_EVENT_CLIENT_MESSAGE,
    OB_EVENT_DAMAGE,
    OB_EVENT_OUTPUT_CHANGE,  /* Monitor hotplug/resolution change */
    OB_EVENT_SEAT_ADDED,
    OB_EVENT_SEAT_REMOVED,
    OB_EVENT_NUM_EVENTS
} ObEventType;

/* Button masks */
typedef enum {
    OB_BUTTON_NONE   = 0,
    OB_BUTTON_LEFT   = 1 << 0,
    OB_BUTTON_MIDDLE = 1 << 1,
    OB_BUTTON_RIGHT  = 1 << 2,
    OB_BUTTON_SCROLL_UP    = 1 << 3,
    OB_BUTTON_SCROLL_DOWN  = 1 << 4,
    OB_BUTTON_SCROLL_LEFT  = 1 << 5,
    OB_BUTTON_SCROLL_RIGHT = 1 << 6,
} ObButtonMask;

/* Modifier keys */
typedef enum {
    OB_MODIFIER_NONE  = 0,
    OB_MODIFIER_SHIFT = 1 << 0,
    OB_MODIFIER_CTRL  = 1 << 1,
    OB_MODIFIER_ALT   = 1 << 2,
    OB_MODIFIER_META  = 1 << 3,
    OB_MODIFIER_SUPER = 1 << 4,
    OB_MODIFIER_HYPER = 1 << 5,
} ObModifierMask;

/* Window types */
typedef enum {
    OB_WINDOW_TYPE_UNKNOWN = 0,
    OB_WINDOW_TYPE_NORMAL,
    OB_WINDOW_TYPE_DOCK,
    OB_WINDOW_TYPE_DESKTOP,
    OB_WINDOW_TYPE_MENU,
    OB_WINDOW_TYPE_TOOLBAR,
    OB_WINDOW_TYPE_SPLASH,
    OB_WINDOW_TYPE_DIALOG,
    OB_WINDOW_TYPE_UTILITY,
    OB_WINDOW_TYPE_NOTIFICATION,
    OB_WINDOW_TYPE_NUM_TYPES
} ObWindowType;

/* Window states */
typedef enum {
    OB_WINDOW_STATE_NONE        = 0,
    OB_WINDOW_STATE_MODAL       = 1 << 0,
    OB_WINDOW_STATE_STICKY      = 1 << 1,
    OB_WINDOW_STATE_MAXIMIZED_VERT = 1 << 2,
    OB_WINDOW_STATE_MAXIMIZED_HORZ = 1 << 3,
    OB_WINDOW_STATE_SHADED      = 1 << 4,
    OB_WINDOW_STATE_SKIP_TASKBAR = 1 << 5,
    OB_WINDOW_STATE_SKIP_PAGER  = 1 << 6,
    OB_WINDOW_STATE_HIDDEN      = 1 << 7,
    OB_WINDOW_STATE_FULLSCREEN  = 1 << 8,
    OB_WINDOW_STATE_ABOVE       = 1 << 9,
    OB_WINDOW_STATE_BELOW       = 1 << 10,
    OB_WINDOW_STATE_DEMANDS_ATTENTION = 1 << 11,
    OB_WINDOW_STATE_FOCUSED     = 1 << 12,
} ObWindowState;

/* Window attributes */
typedef struct _ObWindowAttributes {
    gchar *title;
    gchar *class;
    gchar *name;
    gchar *role;
    gchar *icon_name;
    
    gint x, y;
    gint width, height;
    gint border_width;
    
    gboolean mapped;
    gboolean viewable;
    gboolean decorated;
    gboolean managed;
    
    ObWindowType type;
    ObWindowState state;
    
    gint desktop;  /* -1 for all desktops */
    guint32 user_time;
    
    /* Parent window (for dialogs, etc.) */
    ObPlatformWindow *transient_for;
    
    /* Icon data */
    guchar *icon_data;
    gint icon_width;
    gint icon_height;
} ObWindowAttributes;

/* Event structure */
struct _ObPlatformEvent {
    ObEventType type;
    guint32 time;  /* Timestamp in milliseconds */
    ObPlatformDisplay *display;
    
    union {
        struct {
            Rect area;
            gint count;
        } expose;
        
        struct {
            gint x, y;
            gint width, height;
            gint border_width;
            gboolean above;
        } configure;
        
        struct {
            gint x, y;
            ObButtonMask button;
            ObModifierMask modifiers;
        } button;
        
        struct {
            gint x, y;
            ObModifierMask modifiers;
        } motion;
        
        struct {
            guint keycode;
            guint keysym;
            ObModifierMask modifiers;
            gboolean is_repeat;
        } key;
        
        struct {
            gchar *name;
            gboolean deleted;
        } property;
        
        struct {
            gchar *message_type;
            gulong data[5];
        } client;
        
        struct {
            ObPlatformMonitor *monitor;
            gboolean connected;
        } output;
        
        struct {
            ObPlatformSeat *seat;
        } seat;
    } data;
    
    /* Platform-specific data (X11: XEvent*, Wayland: struct wl_event*) */
    gpointer native_event;
};

/* Monitor/Output information */
struct _ObPlatformMonitor {
    guint id;
    gchar *name;
    gchar *make;
    gchar *model;
    
    Rect geometry;      /* Physical position and size */
    Rect work_area;     /* Usable area (minus panels/struts) */
    
    gint width_mm;      /* Physical size in mm */
    gint height_mm;
    
    gint refresh_rate;  /* Hz */
    gfloat scale;       /* HiDPI scale factor */
    
    gboolean primary;
    gboolean connected;
};

/* Seat (input device collection) */
struct _ObPlatformSeat {
    gchar *name;
    gboolean has_pointer;
    gboolean has_keyboard;
    gboolean has_touch;
    
    /* Current pointer position */
    gint pointer_x;
    gint pointer_y;
    
    /* Focused window */
    ObPlatformWindow *focus;
};

/* ============================================================================
 * Platform Interface (vtable)
 * ============================================================================ */

typedef struct _ObPlatformInterface {
    /* Display operations */
    gboolean (*display_open)(const gchar *name, ObPlatformDisplay **display);
    void (*display_close)(ObPlatformDisplay *display);
    ObPlatformType (*display_get_type)(ObPlatformDisplay *display);
    const gchar *(*display_get_name)(ObPlatformDisplay *display);
    
    /* Event loop integration */
    gint (*display_get_fd)(ObPlatformDisplay *display);
    gboolean (*display_prepare_read)(ObPlatformDisplay *display);
    void (*display_read_events)(ObPlatformDisplay *display);
    gboolean (*display_dispatch_events)(ObPlatformDisplay *display);
    
    /* Event handling */
    gboolean (*event_poll)(ObPlatformDisplay *display, ObPlatformEvent *event);
    void (*event_free)(ObPlatformEvent *event);
    guint32 (*event_get_time)(ObPlatformDisplay *display);
    
    /* Window operations */
    gboolean (*window_create)(ObPlatformDisplay *display,
                              ObPlatformWindow *parent,
                              ObWindowAttributes *attrs,
                              ObPlatformWindow **window);
    void (*window_destroy)(ObPlatformWindow *window);
    void (*window_show)(ObPlatformWindow *window);
    void (*window_hide)(ObPlatformWindow *window);
    void (*window_raise)(ObPlatformWindow *window);
    void (*window_lower)(ObPlatformWindow *window);
    void (*window_move)(ObPlatformWindow *window, gint x, gint y);
    void (*window_resize)(ObPlatformWindow *window, gint width, gint height);
    void (*window_move_resize)(ObPlatformWindow *window, gint x, gint y,
                               gint width, gint height);
    void (*window_set_title)(ObPlatformWindow *window, const gchar *title);
    void (*window_set_icon)(ObPlatformWindow *window,
                            gint width, gint height,
                            const guchar *data);
    void (*window_set_state)(ObPlatformWindow *window, ObWindowState state);
    void (*window_set_desktop)(ObPlatformWindow *window, gint desktop);
    void (*window_focus)(ObPlatformWindow *window);
    gboolean (*window_get_attributes)(ObPlatformWindow *window,
                                      ObWindowAttributes *attrs);
    void (*window_attributes_free)(ObWindowAttributes *attrs);
    
    /* Frame/decoration operations */
    gboolean (*frame_create)(ObPlatformWindow *client_window,
                             ObPlatformSurface **frame_surface);
    void (*frame_destroy)(ObPlatformSurface *surface);
    void (*frame_update)(ObPlatformSurface *surface);
    
    /* Monitor operations */
    guint (*monitor_get_count)(ObPlatformDisplay *display);
    ObPlatformMonitor *(*monitor_get)(ObPlatformDisplay *display, guint index);
    ObPlatformMonitor *(*monitor_get_primary)(ObPlatformDisplay *display);
    ObPlatformMonitor *(*monitor_at_point)(ObPlatformDisplay *display,
                                           gint x, gint y);
    guint (*monitor_get_window)(ObPlatformDisplay *display,
                                ObPlatformWindow *window);
    
    /* Seat operations */
    guint (*seat_get_count)(ObPlatformDisplay *display);
    ObPlatformSeat *(*seat_get)(ObPlatformDisplay *display, guint index);
    ObPlatformSeat *(*seat_get_default)(ObPlatformDisplay *display);
    
    /* Cursor operations */
    gboolean (*cursor_set)(ObPlatformSeat *seat, const gchar *name);
    gboolean (*cursor_set_position)(ObPlatformSeat *seat, gint x, gint y);
    
    /* Selection/clipboard */
    gboolean (*selection_own)(ObPlatformDisplay *display,
                              const gchar *selection);
    gboolean (*selection_get)(ObPlatformDisplay *display,
                              const gchar *selection,
                              gchar **data,
                              gsize *len);
    
    /* Sync */
    void (*sync)(ObPlatformDisplay *display);
    
} ObPlatformInterface;

/* ============================================================================
 * Public API
 * ============================================================================ */

/* Initialize platform subsystem */
gboolean ob_platform_init(void);
void ob_platform_shutdown(void);

/* Get available backends */
gboolean ob_platform_x11_available(void);
gboolean ob_platform_wayland_available(void);

/* Select and initialize backend */
gboolean ob_platform_select_backend(ObPlatformType type);
ObPlatformType ob_platform_get_selected(void);

/* Get the platform interface */
const ObPlatformInterface *ob_platform_get_interface(void);

/* Display operations */
ObPlatformDisplay *ob_platform_display_get(void);
gboolean ob_platform_display_open(const gchar *name);
void ob_platform_display_close(void);

/* Convenience macros for calling interface functions */
#define OB_PLATFORM_CALL(func, ...) \
    (ob_platform_get_interface()->func(__VA_ARGS__))

#define OB_PLATFORM_CALL_RET(func, ...) \
    (ob_platform_get_interface()->func(__VA_ARGS__))

/* Utility functions */
void ob_platform_event_copy(ObPlatformEvent *dest,
                            const ObPlatformEvent *src);
gchar *ob_platform_event_type_to_string(ObEventType type);

G_END_DECLS

#endif /* __ob_platform_h */
