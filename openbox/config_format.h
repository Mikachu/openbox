/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_format.h - Configuration Format Detection Header
   Copyright (c) 2026  Openbox Development Team
*/

#ifndef __config_format_h
#define __config_format_h

#include <glib.h>

G_BEGIN_DECLS

/**
 * Detect configuration file format
 * 
 * @param path Path to check
 * @return "json" for JSON files, "xml" for XML files, NULL for unknown
 */
const gchar *config_detect_format(const gchar *path);

/**
 * Load configuration with automatic format detection
 * 
 * @param path Path to config file
 * @param error Error return location
 * @return TRUE on success, FALSE on error
 */
gboolean config_load_auto(const gchar *path, GError **error);

/**
 * Load menu configuration with automatic format detection
 * 
 * @param path Path to menu file
 * @param error Error return location
 * @return TRUE on success, FALSE on error
 */
gboolean config_load_menu_auto(const gchar *path, GError **error);

/**
 * Load configuration respecting OPENBOX_CONFIG_FORMAT environment variable
 * 
 * @param path Path to config file
 * @param error Error return location
 * @return TRUE on success, FALSE on error
 */
gboolean config_load_with_env(const gchar *path, GError **error);

/**
 * Find the appropriate rc configuration file
 * 
 * Searches in order: user JSON, user XML, system JSON, system XML
 * 
 * @return Path to config file (must be freed by caller), or NULL
 */
gchar *config_find_rc_file(void);

/**
 * Find the appropriate menu configuration file
 * 
 * @return Path to menu file (must be freed by caller), or NULL
 */
gchar *config_find_menu_file(void);

/**
 * Check if configuration should be migrated from XML to JSON
 * 
 * @param path Path to check
 * @return TRUE if XML file exists and JSON doesn't
 */
gboolean config_should_migrate(const gchar *path);

/**
 * Print migration suggestion to user
 * 
 * @param xml_path Path to XML configuration
 */
void config_suggest_migration(const gchar *xml_path);

G_END_DECLS

#endif /* __config_format_h */
