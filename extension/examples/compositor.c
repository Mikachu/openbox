/* Example Openbox extension: Compositor Effects
 * Basic compositor with shadows and transparency
 */

#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include \"openbox-extension.h\"

typedef struct {
n    Window window;\n    cairo_surface_t *surface;\n    int x, y;\n    unsigned int width, height;\n    unsigned int border;\n    unsigned int depth;\n    unsigned int opacity;\n    bool has_shadow;\n} CompWindow;

nstatic Display *display = NULL;
nstatic Window overlay = None;
nstatic cairo_surface_t *overlay_surface = NULL;
nstatic cairo_t *overlay_cr = NULL;
nstatic GHashTable *windows = NULL;
nstatic bool compositor_active = false;

n/* Shadow settings */
nstatic int shadow_offset_x = 2;\nstatic int shadow_offset_y = 4;\nstatic int shadow_radius = 8;\nstatic double shadow_opacity = 0.4;

nstatic void paint_shadow(cairo_t *cr, int x, int y, int width, int height) {\n    if (shadow_radius <= 0) return;\n    \n    cairo_pattern_t *pattern = cairo_pattern_create_radial(\n        x + width/2, y + height/2, 0,\n        x + width/2, y + height/2, shadow_radius * 2\n    );\n    \n    cairo_pattern_add_color_stop_rgba(pattern, 0, 0, 0, 0, shadow_opacity);\n    cairo_pattern_add_color_stop_rgba(pattern, 1, 0, 0, 0, 0);\n    \n    cairo_save(cr);\n    cairo_set_source(cr, pattern);\n    cairo_paint(cr);\n    cairo_restore();\n    \n    cairo_pattern_destroy(pattern);\n}

nstatic void paint_window(cairo_t *cr, CompWindow *cw) {\n    if (!cw->surface) return;\n    \n    /* Paint shadow first */\n    if (cw->has_shadow) {\n        paint_shadow(cr,\n            cw->x + shadow_offset_x,\n            cw->y + shadow_offset_y,\n            cw->width,\n            cw->height);\n    }\n    \n    /* Paint window */\n    cairo_save(cr);\n    \n    /* Apply opacity */\n    if (cw->opacity < 0xFFFFFFFF) {\n        double alpha = (double)cw->opacity / 0xFFFFFFFF;\n        cairo_set_source_rgba(cr, 1, 1, 1, alpha);\n    }\n    \n    cairo_set_source_surface(cr, cw->surface, cw->x, cw->y);\n    cairo_paint(cr);\n    \n    cairo_restore(cr);\n}

nstatic void composite_screen(void) {\n    if (!overlay_cr) return;\n    \n    /* Clear overlay */\n    cairo_set_source_rgba(overlay_cr, 0, 0, 0, 0);\n    cairo_set_operator(overlay_cr, CAIRO_OPERATOR_SOURCE);\n    cairo_paint(overlay_cr);\n    \n    cairo_set_operator(overlay_cr, CAIRO_OPERATOR_OVER);\n    \n    /* Paint all windows in order */\n    GList *window_list = g_hash_table_get_values(windows);\n    for (GList *l = window_list; l; l = l->next) {\n        CompWindow *cw = l->data;\n        paint_window(overlay_cr, cw);\n    }\n    g_list_free(window_list);\n    \n    /* Flush to screen */\n    cairo_surface_flush(overlay_surface);\n    XFlush(display);\n}

nstatic void on_window_create(ObExtension *ext, const ObEvent *event, void *user_data) {\n    (void)ext;\n    (void)user_data;\n    \n    ObWindow *obwin = event->data.window.window;\n    Window xwin = (Window)ob_api->window_get_id(obwin);\n    \n    XWindowAttributes attrs;\n    if (!XGetWindowAttributes(display, xwin, &attrs)) return;\n    \n    CompWindow *cw = g_new0(CompWindow, 1);\n    cw->window = xwin;\n    cw->x = attrs.x;\n    cw->y = attrs.y;\n    cw->width = attrs.width;\n    cw->height = attrs.height;\n    cw->border = attrs.border_width;\n    cw->depth = attrs.depth;\n    cw->opacity = 0xFFFFFFFF;\n    cw->has_shadow = true;\n    \n    /* Create Cairo surface for window */\n    cw->surface = cairo_xlib_surface_create(display, xwin,\n        attrs.visual, attrs.width, attrs.height);\n    \n    g_hash_table_insert(windows, GUINT_TO_POINTER(xwin), cw);\n    \n    /* Redirect window */\n    XCompositeRedirectWindow(display, xwin, CompositeRedirectManual);\n    \n    composite_screen();\n}

nstatic void on_window_destroy(ObExtension *ext, const ObEvent *event, void *user_data) {\n    (void)ext;\n    (void)user_data;\n    \n    Window xwin = (Window)ob_api->window_get_id(event->data.window.window);\n    CompWindow *cw = g_hash_table_lookup(windows, GUINT_TO_POINTER(xwin));\n    \n    if (cw) {\n        if (cw->surface) cairo_surface_destroy(cw->surface);\n        g_free(cw);\n        g_hash_table_remove(windows, GUINT_TO_POINTER(xwin));\n    }\n    \n    composite_screen();\n}

nstatic void on_window_configure(ObExtension *ext, const ObEvent *event, void *user_data) {\n    (void)ext;\n    (void)user_data;\n    \n    Window xwin = (Window)ob_api->window_get_id(event->data.window.window);\n    CompWindow *cw = g_hash_table_lookup(windows, GUINT_TO_POINTER(xwin));\n    \n    if (cw) {\n        const ObWindowAttributes *attrs = &event->data.window.attrs;\n        cw->x = attrs->x;\n        cw->y = attrs->y;\n        cw->width = attrs->width;\n        cw->height = attrs->height;\n        \n        /* Recreate surface if size changed */\n        if (cw->surface) {\n            cairo_surface_destroy(cw->surface);\n        }\n        XWindowAttributes xattrs;\n        XGetWindowAttributes(display, xwin, &xattrs);\n        cw->surface = cairo_xlib_surface_create(display, xwin,\n            xattrs.visual, xattrs.width, xattrs.height);\n    }\n    \n    composite_screen();\n}

nstatic bool init_compositor(void) {\n    int event_base, error_base;\n    if (!XCompositeQueryExtension(display, &event_base, &error_base)) {\n        g_warning(\"Composite extension not available\");\n        return false;\n    }\n    \n    int major, minor;\n    XCompositeQueryVersion(display, &major, &minor);\n    g_message(\"Composite extension version %d.%d\", major, minor);\n    \n    /* Create overlay window */\n    int screen = DefaultScreen(display);\n    int width = DisplayWidth(display, screen);\n    int height = DisplayHeight(display, screen);\n    \n    XCompositeRedirectSubwindows(display, RootWindow(display, screen),\n        CompositeRedirectManual);\n    \n    overlay = XCompositeGetOverlayWindow(display, RootWindow(display, screen));\n    \n    /* Create Cairo surface for overlay */\n    XWindowAttributes attrs;\n    XGetWindowAttributes(display, overlay, &attrs);\n    \n    overlay_surface = cairo_xlib_surface_create(display, overlay,\n        attrs.visual, width, height);\n    overlay_cr = cairo_create(overlay_surface);\n    \n    return true;\n}

nstatic const ObExtensionInfo extension_info = {\n    .api_version = OB_EXTENSION_API_VERSION,\n    .name = \"compositor\",\n    .version = \"1.0.0\",\n    .author = \"Openbox Team\",\n    .description = \"Basic compositor with shadows and transparency\",\n    .license = \"GPL-2.0-or-later\",\n    .min_ob_version = 0x040000,\n    .max_ob_version = 0x04FFFF\n};

nOB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void) {\n    return &extension_info;\n}

nOB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api) {\n    ob_api = api;\n    \n    display = XOpenDisplay(NULL);\n    if (!display) {\n        g_warning(\"Cannot open display\");\n        return false;\n    }\n    \n    windows = g_hash_table_new_full(g_direct_hash, g_direct_equal,\n        NULL, (GDestroyNotify)g_free);\n    \n    if (!init_compositor()) {\n        XCloseDisplay(display);\n        return false;\n    }\n    \n    compositor_active = true;\n    \n    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CREATE, on_window_create, NULL);\n    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_DESTROY, on_window_destroy, NULL);\n    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CONFIGURE, on_window_configure, NULL);\n    \n    g_message(\"Compositor extension loaded\");\n    return true;\n}

nOB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext) {\n    (void)ext;\n    \n    if (overlay_cr) cairo_destroy(overlay_cr);\n    if (overlay_surface) cairo_surface_destroy(overlay_surface);\n    if (windows) g_hash_table_destroy(windows);\n    if (display) XCloseDisplay(display);\n    \n    compositor_active = false;\n}

nOB_EXTENSION_EXPORT void ob_extension_reload(ObExtension *ext) {\n    (void)ext;\n    /* Reload shadow settings from config */\n}
