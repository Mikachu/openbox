/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_json.h - JSON Configuration Parser Header
   Copyright (c) 2026  Openbox Development Team
*/

#ifndef __config_json_h
#define __config_json_h

#include <glib.h>

G_BEGIN_DECLS

/**
 * Load configuration from JSON file
 * 
 * @param path Path to rc.json file
 * @param error Error return location
 * @return TRUE on success, FALSE on error
 */
gboolean config_load_json(const gchar *path, GError **error);

/**
 * Load menu configuration from JSON file
 * 
 * @param path Path to menu.json file
 * @param error Error return location
 * @return TRUE on success, FALSE on error
 */
gboolean config_load_menu_json(const gchar *path, GError **error);

/**
 * Enable automatic config reloading on file changes
 * 
 * @param path Path to config file to monitor
 * @return TRUE if monitoring was started
 */
gboolean config_enable_hot_reload(const gchar *path);

/**
 * Disable automatic config reloading
 */
void config_disable_hot_reload(void);

/**
 * Cleanup JSON configuration resources
 */
void config_json_cleanup(void);

/**
 * Detect configuration file format
 * 
 * @param path Path to check
 * @return "json" for JSON files, "xml" for XML files, NULL for unknown
 */
const gchar *config_detect_format(const gchar *path);

G_END_DECLS

#endif /* __config_json_h */
