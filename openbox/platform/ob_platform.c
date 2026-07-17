/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   ob_platform.c - Platform Abstraction Layer Implementation
   Copyright (c) 2026  Openbox Development Team

   Main implementation of the platform abstraction layer.
*/

#include "ob_platform.h"
#include "x11/x11_backend.h"
#include "wayland/wayland_backend.h"

/* Current platform backend */
static ObPlatformType selected_backend = OB_PLATFORM_NONE;
static const ObPlatformInterface *current_interface = NULL;

/* Platform availability checks */
gboolean ob_platform_x11_available(void)
{
    const gchar *display = g_getenv("DISPLAY");
    return display != NULL && strlen(display) > 0;
}

gboolean ob_platform_wayland_available(void)
{
    const gchar *wayland_display = g_getenv("WAYLAND_DISPLAY");
    return wayland_display != NULL && strlen(wayland_display) > 0;
}

gboolean ob_platform_init(void)
{
    /* Initialize both backends */
    gboolean x11_ok = x11_backend_init();
    gboolean wayland_ok = wayland_backend_init();
    
    g_message("Platform backends initialized: X11=%s, Wayland=%s",
              x11_ok ? "yes" : "no",
              wayland_ok ? "yes" : "no");
    
    return x11_ok || wayland_ok;
}

void ob_platform_shutdown(void)
{
    x11_backend_shutdown();
    wayland_backend_shutdown();
    selected_backend = OB_PLATFORM_NONE;
    current_interface = NULL;
}

gboolean ob_platform_select_backend(ObPlatformType type)
{
    switch (type) {
        case OB_PLATFORM_X11:
            if (!ob_platform_x11_available()) {
                g_warning("X11 display not available");
                return FALSE;
            }
            current_interface = x11_backend_get_interface();
            selected_backend = OB_PLATFORM_X11;
            g_message("Selected X11 backend");
            return TRUE;
            
        case OB_PLATFORM_WAYLAND:
            if (!ob_platform_wayland_available()) {
                g_warning("Wayland display not available");
                return FALSE;
            }
            current_interface = wayland_backend_get_interface();
            selected_backend = OB_PLATFORM_WAYLAND;
            g_message("Selected Wayland backend");
            return TRUE;
            
        case OB_PLATFORM_NONE:
            /* Auto-detect */
            if (ob_platform_wayland_available()) {
                return ob_platform_select_backend(OB_PLATFORM_WAYLAND);
            } else if (ob_platform_x11_available()) {
                return ob_platform_select_backend(OB_PLATFORM_X11);
            } else {
                g_error("No display backend available");
                return FALSE;
            }
            
        default:
            g_warning("Unknown backend type: %d", type);
            return FALSE;
    }
}

ObPlatformType ob_platform_get_selected(void)
{
    return selected_backend;
}

const ObPlatformInterface *ob_platform_get_interface(void)
{
    if (!current_interface) {
        g_error("No platform backend selected. Call ob_platform_select_backend() first.");
    }
    return current_interface;
}

/* Convenience wrapper functions */
ObPlatformDisplay *ob_platform_display_get(void)
{
    /* This would return the global display from the current backend */
    switch (selected_backend) {
        case OB_PLATFORM_X11:
            return (ObPlatformDisplay *)x11_display_get();
        case OB_PLATFORM_WAYLAND:
            return (ObPlatformDisplay *)wayland_display_get();
        default:
            return NULL;
    }
}

gboolean ob_platform_display_open(const gchar *name)
{
    const ObPlatformInterface *iface = ob_platform_get_interface();
    ObPlatformDisplay *display = NULL;
    
    if (!iface->display_open(name, &display)) {
        return FALSE;
    }
    
    return TRUE;
}

void ob_platform_display_close(void)
{
    const ObPlatformInterface *iface = ob_platform_get_interface();
    ObPlatformDisplay *display = ob_platform_display_get();
    
    if (iface && display) {
        iface->display_close(display);
    }
}

/* Utility functions */
void ob_platform_event_copy(ObPlatformEvent *dest,
                            const ObPlatformEvent *src)
{
    if (!dest || !src) return;
    
    memcpy(dest, src, sizeof(ObPlatformEvent));
    
    /* Deep copy platform-specific data */
    switch (src->type) {
        case OB_EVENT_PROPERTY:
            dest->data.property.name = g_strdup(src->data.property.name);
            break;
        case OB_EVENT_CLIENT_MESSAGE:
            dest->data.client.message_type = g_strdup(src->data.client.message_type);
            break;
        default:
            break;
    }
}

gchar *ob_platform_event_type_to_string(ObEventType type)
{
    switch (type) {
        case OB_EVENT_EXPOSE: return g_strdup("expose");
        case OB_EVENT_CONFIGURE: return g_strdup("configure");
        case OB_EVENT_MAP: return g_strdup("map");
        case OB_EVENT_UNMAP: return g_strdup("unmap");
        case OB_EVENT_DESTROY: return g_strdup("destroy");
        case OB_EVENT_FOCUS_IN: return g_strdup("focus-in");
        case OB_EVENT_FOCUS_OUT: return g_strdup("focus-out");
        case OB_EVENT_ENTER: return g_strdup("enter");
        case OB_EVENT_LEAVE: return g_strdup("leave");
        case OB_EVENT_BUTTON_PRESS: return g_strdup("button-press");
        case OB_EVENT_BUTTON_RELEASE: return g_strdup("button-release");
        case OB_EVENT_MOTION: return g_strdup("motion");
        case OB_EVENT_KEY_PRESS: return g_strdup("key-press");
        case OB_EVENT_KEY_RELEASE: return g_strdup("key-release");
        case OB_EVENT_PROPERTY: return g_strdup("property");
        case OB_EVENT_CLIENT_MESSAGE: return g_strdup("client-message");
        case OB_EVENT_DAMAGE: return g_strdup("damage");
        case OB_EVENT_OUTPUT_CHANGE: return g_strdup("output-change");
        case OB_EVENT_SEAT_ADDED: return g_strdup("seat-added");
        case OB_EVENT_SEAT_REMOVED: return g_strdup("seat-removed");
        default: return g_strdup("unknown");
    }
}
