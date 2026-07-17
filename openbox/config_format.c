/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_format.c - Configuration Format Detection and Loading
   Copyright (c) 2026  Openbox Development Team

   Handles auto-detection of XML vs JSON configuration formats and
   provides unified loading interface.
*/

#include "config.h"
#include "config_json.h"
#include "config_xml.h"

#include <string.h>
#include <magic.h>

typedef enum {
    CONFIG_FORMAT_UNKNOWN,
    CONFIG_FORMAT_XML,
    CONFIG_FORMAT_JSON,
    CONFIG_FORMAT_AUTO
} ConfigFormat;

/* ============================================================================
 * Format Detection
 * ============================================================================ */

static ConfigFormat detect_by_extension(const gchar *path)
{
    if (g_str_has_suffix(path, ".xml")) {
        return CONFIG_FORMAT_XML;
    }
    if (g_str_has_suffix(path, ".json")) {
        return CONFIG_FORMAT_JSON;
    }
    return CONFIG_FORMAT_UNKNOWN;
}

static ConfigFormat detect_by_content(const gchar *path)
{
    gchar *contents = NULL;
    gsize length;
    GError *error = NULL;
    
    if (!g_file_get_contents(path, &contents, &length, &error)) {
        if (error) g_error_free(error);
        return CONFIG_FORMAT_UNKNOWN;
    }
    
    /* Skip whitespace */
    gchar *p = contents;
    while (*p && g_ascii_isspace(*p)) p++;
    
    ConfigFormat format = CONFIG_FORMAT_UNKNOWN;
    
    if (*p == '<') {
        /* XML starts with < */
        format = CONFIG_FORMAT_XML;
    } else if (*p == '{' || *p == '[') {
        /* JSON starts with { or [ */
        format = CONFIG_FORMAT_JSON;
    }
    
    g_free(contents);
    return format;
}

const gchar *config_detect_format(const gchar *path)
{
    if (!path) return NULL;
    
    ConfigFormat format = detect_by_extension(path);
    if (format == CONFIG_FORMAT_UNKNOWN) {
        format = detect_by_content(path);
    }
    
    switch (format) {
        case CONFIG_FORMAT_XML: return "xml";
        case CONFIG_FORMAT_JSON: return "json";
        default: return NULL;
    }
}

/* ============================================================================
 * Unified Loading API
 * ============================================================================ */

gboolean config_load_auto(const gchar *path, GError **error)
{
    g_return_val_if_fail(path != NULL, FALSE);
    
    const gchar *format = config_detect_format(path);
    
    if (!format) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "Cannot determine format of %s", path);
        return FALSE;
    }
    
    g_message("Loading configuration from %s (format: %s)", path, format);
    
    if (strcmp(format, "json") == 0) {
        return config_load_json(path, error);
    } else {
        return config_load_xml(path, error);
    }
}

gboolean config_load_menu_auto(const gchar *path, GError **error)
{
    g_return_val_if_fail(path != NULL, FALSE);
    
    const gchar *format = config_detect_format(path);
    
    if (!format) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "Cannot determine format of %s", path);
        return FALSE;
    }
    
    g_message("Loading menu from %s (format: %s)", path, format);
    
    if (strcmp(format, "json") == 0) {
        return config_load_menu_json(path, error);
    } else {
        return config_load_menu_xml(path, error);
    }
}

/* ============================================================================
 * Path Resolution
 * ============================================================================ */

static gchar *find_config_file(const gchar *filename, gboolean system)
{
    gchar *path = NULL;
    
    if (system) {
        /* System paths */
        const gchar * const *dirs = g_get_system_config_dirs();
        for (int i = 0; dirs[i]; i++) {
            path = g_build_filename(dirs[i], "openbox", filename, NULL);
            if (g_file_test(path, G_FILE_TEST_EXISTS)) {
                return path;
            }
            g_free(path);
            path = NULL;
        }
        
        /* Fallback to SYSCONFDIR */
        path = g_build_filename(SYSCONFDIR, "openbox", filename, NULL);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            return path;
        }
        g_free(path);
    } else {
        /* User path */
        path = g_build_filename(g_get_user_config_dir(), "openbox", 
                               filename, NULL);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            return path;
        }
        g_free(path);
        
        /* Try with .json extension */
        gchar *json_name = g_strconcat(filename, ".json", NULL);
        path = g_build_filename(g_get_user_config_dir(), "openbox",
                               json_name, NULL);
        g_free(json_name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            return path;
        }
        g_free(path);
        
        /* Try with .xml extension */
        gchar *xml_name = g_strconcat(filename, ".xml", NULL);
        path = g_build_filename(g_get_user_config_dir(), "openbox",
                               xml_name, NULL);
        g_free(xml_name);
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            return path;
        }
        g_free(path);
    }
    
    return NULL;
}

gchar *config_find_rc_file(void)
{
    gchar *path;
    
    /* Try user config first (JSON preferred) */
    path = find_config_file("rc.json", FALSE);
    if (path) return path;
    
    path = find_config_file("rc.xml", FALSE);
    if (path) return path;
    
    /* Fall back to system config */
    path = find_config_file("rc.json", TRUE);
    if (path) return path;
    
    path = find_config_file("rc.xml", TRUE);
    if (path) return path;
    
    /* Last resort - create new JSON config */
    return g_build_filename(g_get_user_config_dir(), "openbox", 
                           "rc.json", NULL);
}

gchar *config_find_menu_file(void)
{
    gchar *path;
    
    /* Try user config first (JSON preferred) */
    path = find_config_file("menu.json", FALSE);
    if (path) return path;
    
    path = find_config_file("menu.xml", FALSE);
    if (path) return path;
    
    /* Fall back to system config */
    path = find_config_file("menu.json", TRUE);
    if (path) return path;
    
    path = find_config_file("menu.xml", TRUE);
    if (path) return path;
    
    return NULL;
}

/* ============================================================================
 * Migration Detection
 * ============================================================================ */

gboolean config_should_migrate(const gchar *path)
{
    if (!path) return FALSE;
    
    /* Check if this is an XML file that should be migrated */
    if (!g_str_has_suffix(path, ".xml")) {
        return FALSE;
    }
    
    /* Check if JSON version already exists */
    gchar *json_path = g_strdup(path);
    gsize len = strlen(json_path);
    if (len > 4) {
        strcpy(json_path + len - 4, ".json");
    }
    
    gboolean should = !g_file_test(json_path, G_FILE_TEST_EXISTS);
    g_free(json_path);
    
    return should;
}

void config_suggest_migration(const gchar *xml_path)
{
    g_print("\\n");
    g_print("========================================\\n");
    g_print("  Configuration Migration Available\\n");
    g_print("========================================\\n");
    g_print("\\n");
    g_print("Your configuration is using the legacy XML format.\\n");
    g_print("Openbox 4 now supports JSON configuration with:\\n");
    g_print("  - Better validation and error messages\\n");
    g_print("  - Hot-reload support\\n");
    g_print("  - Easier editing with IDE support\\n");
    g_print("\\n");
    g_print("To migrate, run:\\n");
    g_print("  openbox-xml2json %s\\n", xml_path);
    g_print("\\n");
    g_print("Or to migrate automatically on next startup:\\n");
    g_print("  openbox --migrate-config\\n");
    g_print("\\n");
    g_print("XML configuration will continue to work.\\n");
    g_print("========================================\\n");
    g_print("\\n");
}

/* ============================================================================
 * Environment-based Format Selection
 * ============================================================================ */

static ConfigFormat get_env_format(void)
{
    const gchar *format = g_getenv("OPENBOX_CONFIG_FORMAT");
    if (!format) return CONFIG_FORMAT_AUTO;
    
    if (g_ascii_strcasecmp(format, "json") == 0) {
        return CONFIG_FORMAT_JSON;
    } else if (g_ascii_strcasecmp(format, "xml") == 0) {
        return CONFIG_FORMAT_XML;
    }
    
    return CONFIG_FORMAT_AUTO;
}

gboolean config_load_with_env(const gchar *path, GError **error)
{
    ConfigFormat env_format = get_env_format();
    
    switch (env_format) {
        case CONFIG_FORMAT_JSON:
            return config_load_json(path, error);
        case CONFIG_FORMAT_XML:
            return config_load_xml(path, error);
        default:
            return config_load_auto(path, error);
    }
}
