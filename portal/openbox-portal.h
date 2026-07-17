/* Openbox Portal Header - v4.0.1
 * xdg-desktop-portal implementation
 */

#ifndef OPENBOX_PORTAL_H
#define OPENBOX_PORTAL_H

#include <gio/gio.h>
#include <glib.h>

G_BEGIN_DECLS

/* Initialize portal service */
gboolean openbox_portal_init(void);

/* Shutdown portal service */
void openbox_portal_shutdown(void);

/* Emit setting changed notification */
void openbox_portal_emit_setting_changed(const gchar *namespace,
                                          const gchar *key,
                                          GVariant *value);

/* Check if app is allowed to run in background */
gboolean ob_config_app_allowed_background(const gchar *app_id);

/* Check if app is allowed to take screenshots */
gboolean ob_config_app_allowed_screenshots(const gchar *app_id);

/* Get theme variant */
const gchar *ob_theme_get_variant(void);

/* Get theme name */
const gchar *ob_theme_get_name(void);

/* Get accent color */
void ob_theme_get_accent_color(GdkRGBA *color);

/* Get number of desktops */
guint32 ob_screen_num_desktops(void);

/* Get current desktop */
guint32 ob_screen_current_desktop(void);

G_END_DECLS

#endif /* OPENBOX_PORTAL_H */
