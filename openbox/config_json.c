/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_json.c - JSON Configuration Parser for Openbox 4
   Copyright (c) 2026  Openbox Development Team

   Uses json-glib for parsing and JSON-GLib for validation.
*/

#include "config.h"
#include "config_json.h"

#include <json-glib/json-glib.h>
#include <string.h>
#include <errno.h>

/* Schema validation */
typedef struct {
    gchar *schema_path;
    JsonNode *schema_root;
    gboolean loaded;
} SchemaCache;

static SchemaCache rc_schema = { NULL, NULL, FALSE };
static SchemaCache menu_schema = { NULL, NULL, FALSE };

/* Error context for reporting */
typedef struct {
    gchar *file_path;
    GList *errors;
} ConfigErrorContext;

static ConfigErrorContext *current_errors = NULL;

/* ============================================================================
 * Error Handling
 * ============================================================================ */

static void config_error_add(const gchar *path, const gchar *message)
{
    if (!current_errors) return;
    
    gchar *error_str = g_strdup_printf("%s: %s", path, message);
    current_errors->errors = g_list_append(current_errors->errors, error_str);
    
    g_warning("Config error at %s: %s", path, message);
}

static void config_error_clear(void)
{
    if (!current_errors) return;
    
    g_list_free_full(current_errors->errors, g_free);
    current_errors->errors = NULL;
}

/* ============================================================================
 * Schema Loading
 * ============================================================================ */

static gboolean schema_load(SchemaCache *cache, const gchar *filename)
{
    if (cache->loaded) return TRUE;
    
    gchar *path = g_build_filename(SYSCONFDIR, "openbox", "schemas", 
                                    filename, NULL);
    
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        /* Try user config dir */
        g_free(path);
        path = g_build_filename(g_get_user_config_dir(), "openbox", "schemas",
                               filename, NULL);
    }
    
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_warning("Schema file not found: %s", filename);
        g_free(path);
        return FALSE;
    }
    
    gchar *contents = NULL;
    gsize length;
    GError *error = NULL;
    
    if (!g_file_get_contents(path, &contents, &length, &error)) {
        g_warning("Failed to load schema %s: %s", filename, error->message);
        g_error_free(error);
        g_free(path);
        return FALSE;
    }
    
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, length, &error)) {
        g_warning("Failed to parse schema %s: %s", filename, error->message);
        g_error_free(error);
        g_object_unref(parser);
        g_free(contents);
        g_free(path);
        return FALSE;
    }
    
    cache->schema_root = json_parser_get_root(parser);
    if (cache->schema_root) {
        json_node_ref(cache->schema_root);
    }
    cache->schema_path = path;
    cache->loaded = TRUE;
    
    g_object_unref(parser);
    g_free(contents);
    
    return TRUE;
}

static void schema_unload(SchemaCache *cache)
{
    if (cache->schema_root) {
        json_node_unref(cache->schema_root);
        cache->schema_root = NULL;
    }
    g_free(cache->schema_path);
    cache->schema_path = NULL;
    cache->loaded = FALSE;
}

/* ============================================================================
 * JSON Node Helpers
 * ============================================================================ */

static gboolean json_node_get_boolean_default(JsonNode *node, const gchar *member,
                                             gboolean default_val)
{
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return default_val;
    
    JsonObject *obj = json_node_get_object(node);
    if (!json_object_has_member(obj, member)) return default_val;
    
    JsonNode *member_node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_VALUE(member_node)) return default_val;
    
    return json_node_get_boolean(member_node);
}

static gint json_node_get_int_default(JsonNode *node, const gchar *member,
                                       gint default_val)
{
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return default_val;
    
    JsonObject *obj = json_node_get_object(node);
    if (!json_object_has_member(obj, member)) return default_val;
    
    JsonNode *member_node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_VALUE(member_node)) return default_val;
    
    return (gint)json_node_get_int(member_node);
}

static const gchar *json_node_get_string_default(JsonNode *node, const gchar *member,
                                                const gchar *default_val)
{
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return default_val;
    
    JsonObject *obj = json_node_get_object(node);
    if (!json_object_has_member(obj, member)) return default_val;
    
    JsonNode *member_node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_VALUE(member_node)) return default_val;
    
    return json_node_get_string(member_node);
}

static JsonArray *json_node_get_array(JsonNode *node, const gchar *member)
{
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return NULL;
    
    JsonObject *obj = json_node_get_object(node);
    if (!json_object_has_member(obj, member)) return NULL;
    
    JsonNode *member_node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_ARRAY(member_node)) return NULL;
    
    return json_node_get_array(member_node);
}

static JsonObject *json_node_get_object(JsonNode *node, const gchar *member)
{
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return NULL;
    
    JsonObject *obj = json_node_get_object(node);
    if (!json_object_has_member(obj, member)) return NULL;
    
    JsonNode *member_node = json_object_get_member(obj, member);
    if (!JSON_NODE_HOLDS_OBJECT(member_node)) return NULL;
    
    return json_node_get_object(member_node);
}

/* ============================================================================
 * Action Parsing
 * ============================================================================ */

static void parse_startup_notify(JsonObject *obj, ObActionStartupNotify *notify)
{
    notify->enabled = json_node_get_boolean_default(
        json_object_get_member(obj, "startup_notify"), "enabled", TRUE);
    notify->name = g_strdup(json_node_get_string_default(
        json_object_get_member(obj, "startup_notify"), "name", NULL));
    notify->icon = g_strdup(json_node_get_string_default(
        json_object_get_member(obj, "startup_notify"), "icon", NULL));
    notify->wmclass = g_strdup(json_node_get_string_default(
        json_object_get_member(obj, "startup_notify"), "wmclass", NULL));
}

static GSList *parse_actions(JsonArray *array, GError **error)
{
    GSList *actions = NULL;
    guint len = json_array_get_length(array);
    
    for (guint i = 0; i < len; i++) {
        JsonNode *node = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(node)) continue;
        
        JsonObject *obj = json_node_get_object(node);
        const gchar *name = json_object_get_string_member(obj, "name");
        if (!name) continue;
        
        ObAction *act = g_slice_new0(ObAction);
        act->name = g_strdup(name);
        
        /* Parse action-specific data */
        if (json_object_has_member(obj, "command")) {
            act->data.execute.path = g_strdup(
                json_object_get_string_member(obj, "command"));
        }
        if (json_object_has_member(obj, "menu")) {
            act->data.showmenu.menu = g_strdup(
                json_object_get_string_member(obj, "menu"));
        }
        if (json_object_has_member(obj, "to")) {
            JsonNode *to_node = json_object_get_member(obj, "to");
            if (JSON_NODE_HOLDS_VALUE(to_node)) {
                if (json_node_get_value_type(to_node) == G_TYPE_INT64) {
                    act->data.desktop.dir = OB_DIRECTION_NORTH;
                    act->data.desktop.desktops = json_node_get_int(to_node);
                } else {
                    const gchar *dir_str = json_node_get_string(to_node);
                    act->data.desktop.dir = 
                        string_to_direction(dir_str, OB_DIRECTION_NORTH);
                }
            }
        }
        if (json_object_has_member(obj, "startup_notify")) {
            parse_startup_notify(obj, &act->data.execute.notify);
        }
        
        actions = g_slist_append(actions, act);
    }
    
    return actions;
}

/* ============================================================================
 * Keybind Parsing
 * ============================================================================ */

static gboolean parse_keybinds(JsonArray *array, GError **error)
{
    guint len = json_array_get_length(array);
    
    for (guint i = 0; i < len; i++) {
        JsonNode *node = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(node)) continue;
        
        JsonObject *obj = json_node_get_object(node);
        
        const gchar *key = json_object_get_string_member(obj, "key");
        if (!key) continue;
        
        /* Parse key combination */
        guint state = 0, keycode = 0;
        if (!parse_key_string(key, &state, &keycode)) {
            config_error_add(key, "Invalid key combination");
            continue;
        }
        
        /* Get actions */
        GSList *actions = NULL;
        if (json_object_has_member(obj, "action")) {
            JsonArray *act_array = json_array_new();
            json_array_add_object_element(act_array, 
                json_object_ref(json_object_get_object_member(obj, "action")));
            actions = parse_actions(act_array, error);
            json_array_unref(act_array);
        } else if (json_object_has_member(obj, "actions")) {
            JsonArray *act_array = json_node_get_array(
                json_object_get_member(obj, "actions"));
            if (act_array) {
                actions = parse_actions(act_array, error);
            }
        }
        
        if (actions) {
            config_keybind(key, actions);
            g_slist_free_full(actions, (GDestroyNotify)ob_action_unref);
        }
    }
    
    return TRUE;
}

/* ============================================================================
 * Mousebind Parsing
 * ============================================================================ */

static gboolean parse_mousebinds(JsonArray *array, GError **error)
{
    guint len = json_array_get_length(array);
    
    for (guint i = 0; i < len; i++) {
        JsonNode *node = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(node)) continue;
        
        JsonObject *obj = json_node_get_object(node);
        
        const gchar *context = json_object_get_string_member(obj, "context");
        const gchar *button = json_object_get_string_member(obj, "button");
        const gchar *event = json_object_get_string_member(obj, "event");
        
        if (!context || !button || !event) {
            config_error_add("mousebind", "Missing required field");
            continue;
        }
        
        /* Parse button */
        guint state = 0, button_num = 0;
        if (!parse_button_string(button, &state, &button_num)) {
            config_error_add(button, "Invalid button specification");
            continue;
        }
        
        /* Get actions */
        GSList *actions = NULL;
        if (json_object_has_member(obj, "action")) {
            JsonArray *act_array = json_array_new();
            json_array_add_object_element(act_array,
                json_object_ref(json_object_get_object_member(obj, "action")));
            actions = parse_actions(act_array, error);
            json_array_unref(act_array);
        } else if (json_object_has_member(obj, "actions")) {
            JsonArray *act_array = json_node_get_array(
                json_object_get_member(obj, "actions"));
            if (act_array) {
                actions = parse_actions(act_array, error);
            }
        }
        
        if (actions) {
            config_mousebind(context, button_num, state, 
                          string_to_mouse_event(event), actions);
            g_slist_free_full(actions, (GDestroyNotify)ob_action_unref);
        }
    }
    
    return TRUE;
}

/* ============================================================================
 * Main Config Parsing
 * ============================================================================ */

static gboolean parse_config_json(JsonNode *root, GError **error)
{
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "Root must be a JSON object");
        return FALSE;
    }
    
    JsonObject *obj = json_node_get_object(root);
    
    /* Check version */
    const gchar *version = json_object_get_string_member(obj, "version");
    if (!version) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "Missing required 'version' field");
        return FALSE;
    }
    
    g_message("Loading Openbox configuration version %s", version);
    
    /* Parse resistance */
    JsonObject *resistance = json_node_get_object(root, "resistance");
    if (resistance) {
        config_resistance_strength = json_object_get_int_member_with_default(
            resistance, "strength", 10);
        config_resistance_screen_edge_strength = 
            json_object_get_int_member_with_default(
                resistance, "screen_edge_strength", 20);
    }
    
    /* Parse focus */
    JsonObject *focus = json_node_get_object(root, "focus");
    if (focus) {
        config_focus_new = json_object_get_boolean_member_with_default(
            focus, "focus_new", TRUE);
        config_focus_follow = json_object_get_boolean_member_with_default(
            focus, "follow_mouse", FALSE);
        config_focus_last = json_object_get_boolean_member_with_default(
            focus, "focus_last", TRUE);
        config_focus_under_mouse = json_object_get_boolean_member_with_default(
            focus, "under_mouse", FALSE);
        config_focus_delay = json_object_get_int_member_with_default(
            focus, "focus_delay", 200);
        config_focus_raise = json_object_get_boolean_member_with_default(
            focus, "raise_on_focus", FALSE);
    }
    
    /* Parse theme */
    JsonObject *theme = json_node_get_object(root, "theme");
    if (theme) {
        config_theme = g_strdup(json_object_get_string_member_with_default(
            theme, "name", "Clearlooks"));
        config_theme_keepborder = json_object_get_boolean_member_with_default(
            theme, "keep_border", TRUE);
        config_animate_iconify = json_object_get_boolean_member_with_default(
            theme, "animate_iconify", TRUE);
        
        /* Parse fonts */
        JsonObject *fonts = json_object_get_object_member(theme, "fonts");
        if (fonts) {
            parse_fonts_json(fonts);
        }
    }
    
    /* Parse desktops */
    JsonObject *desktops = json_node_get_object(root, "desktops");
    if (desktops) {
        config_desktops_num = json_object_get_int_member_with_default(
            desktops, "count", 4);
        config_desktops_first = json_object_get_int_member_with_default(
            desktops, "first", 1);
        config_desktops_popup_time = json_object_get_int_member_with_default(
            desktops, "popup_time", 875);
        
        /* Parse desktop names */
        JsonArray *names = json_object_get_array_member(desktops, "names");
        if (names) {
            parse_desktop_names_json(names);
        }
    }
    
    /* Parse resize */
    JsonObject *resize = json_node_get_object(root, "resize");
    if (resize) {
        config_resize_redraw = json_object_get_boolean_member_with_default(
            resize, "draw_contents", TRUE);
        const gchar *popup_show = json_object_get_string_member_with_default(
            resize, "popup_show", "nonpixel");
        config_resize_popup = string_to_resize_popup(popup_show);
    }
    
    /* Parse margins */
    JsonObject *margins = json_node_get_object(root, "margins");
    if (margins) {
        config_margins.top = json_object_get_int_member_with_default(
            margins, "top", 0);
        config_margins.bottom = json_object_get_int_member_with_default(
            margins, "bottom", 0);
        config_margins.left = json_object_get_int_member_with_default(
            margins, "left", 0);
        config_margins.right = json_object_get_int_member_with_default(
            margins, "right", 0);
    }
    
    /* Parse keyboard */
    JsonObject *keyboard = json_node_get_object(root, "keyboard");
    if (keyboard) {
        const gchar *chain_quit = json_object_get_string_member_with_default(
            keyboard, "chain_quit_key", "C-g");
        parse_chain_quit_key(chain_quit);
        
        JsonArray *keybinds = json_object_get_array_member(keyboard, "keybinds");
        if (keybinds) {
            parse_keybinds(keybinds, error);
        }
    }
    
    /* Parse mouse */
    JsonObject *mouse = json_node_get_object(root, "mouse");
    if (mouse) {
        config_mouse_threshold = json_object_get_int_member_with_default(
            mouse, "drag_threshold", 1);
        config_mouse_dclicktime = json_object_get_int_member_with_default(
            mouse, "double_click_time", 500);
        
        JsonArray *bindings = json_object_get_array_member(mouse, "bindings");
        if (bindings) {
            parse_mousebinds(bindings, error);
        }
    }
    
    /* Parse menu */
    JsonObject *menu = json_node_get_object(root, "menu");
    if (menu) {
        config_menu_hide_delay = json_object_get_int_member_with_default(
            menu, "hide_delay", 200);
        config_menu_middle = json_object_get_boolean_member_with_default(
            menu, "middle", FALSE);
        config_menu_sub_show_delay = json_object_get_int_member_with_default(
            menu, "submenu_show_delay", 100);
        config_menu_sub_hide_delay = json_object_get_int_member_with_default(
            menu, "submenu_hide_delay", 400);
        config_menu_show_icons = json_object_get_boolean_member_with_default(
            menu, "show_icons", TRUE);
    }
    
    return TRUE;
}

/* ============================================================================
 * Public API
 * ============================================================================ */

gboolean config_load_json(const gchar *path, GError **error)
{
    g_return_val_if_fail(path != NULL, FALSE);
    
    /* Setup error context */
    current_errors = g_new0(ConfigErrorContext, 1);
    current_errors->file_path = g_strdup(path);
    
    /* Load schema if available */
    schema_load(&rc_schema, "rc-schema.json");
    
    /* Read file */
    gchar *contents = NULL;
    gsize length;
    
    if (!g_file_get_contents(path, &contents, &length, error)) {
        g_free(current_errors->file_path);
        g_free(current_errors);
        current_errors = NULL;
        return FALSE;
    }
    
    /* Parse JSON */
    JsonParser *parser = json_parser_new();
    gboolean success = json_parser_load_from_data(parser, contents, length, error);
    
    g_free(contents);
    
    if (!success) {
        g_object_unref(parser);
        g_free(current_errors->file_path);
        g_free(current_errors);
        current_errors = NULL;
        return FALSE;
    }
    
    /* Get root and parse */
    JsonNode *root = json_parser_get_root(parser);
    success = parse_config_json(root, error);
    
    g_object_unref(parser);
    
    /* Report errors */
    if (current_errors->errors) {
        g_warning("Configuration had %d validation errors",
                 g_list_length(current_errors->errors));
    }
    
    /* Cleanup */
    config_error_clear();
    g_free(current_errors->file_path);
    g_free(current_errors);
    current_errors = NULL;
    
    return success;
}

gboolean config_load_menu_json(const gchar *path, GError **error)
{
    g_return_val_if_fail(path != NULL, FALSE);
    
    schema_load(&menu_schema, "menu-schema.json");
    
    gchar *contents = NULL;
    gsize length;
    
    if (!g_file_get_contents(path, &contents, &length, error)) {
        return FALSE;
    }
    
    JsonParser *parser = json_parser_new();
    gboolean success = json_parser_load_from_data(parser, contents, length, error);
    
    g_free(contents);
    
    if (!success) {
        g_object_unref(parser);
        return FALSE;
    }
    
    JsonNode *root = json_parser_get_root(parser);
    
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "Menu root must be a JSON object");
        g_object_unref(parser);
        return FALSE;
    }
    
    JsonObject *obj = json_node_get_object(root);
    
    /* Parse menus array */
    JsonArray *menus = json_object_get_array_member(obj, "menus");
    if (menus) {
        parse_menus_json(menus, error);
    }
    
    g_object_unref(parser);
    return TRUE;
}

void config_json_cleanup(void)
{
    schema_unload(&rc_schema);
    schema_unload(&menu_schema);
}

/* ============================================================================
 * Hot Reload Support
 * ============================================================================ */

static GFileMonitor *config_monitor = NULL;
static guint reload_timeout_id = 0;

static gboolean reload_config_cb(gpointer data)
{
    const gchar *path = data;
    GError *error = NULL;
    
    g_message("Reloading configuration from %s", path);
    
    /* Try JSON first */
    if (g_str_has_suffix(path, ".json")) {
        if (!config_load_json(path, &error)) {
            g_warning("Failed to reload config: %s", error->message);
            g_error_free(error);
        }
    } else {
        /* Fall back to XML */
        if (!config_load_xml(path, &error)) {
            g_warning("Failed to reload config: %s", error->message);
            g_error_free(error);
        }
    }
    
    reload_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

static void config_changed_cb(GFileMonitor *monitor, GFile *file, GFile *other,
                              GFileMonitorEvent event_type, gpointer data)
{
    const gchar *path = data;
    
    if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
        event_type != G_FILE_MONITOR_EVENT_CREATED) {
        return;
    }
    
    /* Debounce rapid changes */
    if (reload_timeout_id) {
        g_source_remove(reload_timeout_id);
    }
    
    reload_timeout_id = g_timeout_add(500, reload_config_cb, 
                                       (gpointer)g_strdup(path));
}

gboolean config_enable_hot_reload(const gchar *path)
{
    GFile *file = g_file_new_for_path(path);
    GError *error = NULL;
    
    config_monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, 
                                          NULL, &error);
    g_object_unref(file);
    
    if (!config_monitor) {
        g_warning("Failed to monitor config: %s", error->message);
        g_error_free(error);
        return FALSE;
    }
    
    g_signal_connect(config_monitor, "changed", 
                    G_CALLBACK(config_changed_cb), g_strdup(path));
    
    return TRUE;
}

void config_disable_hot_reload(void)
{
    if (reload_timeout_id) {
        g_source_remove(reload_timeout_id);
        reload_timeout_id = 0;
    }
    
    if (config_monitor) {
        g_file_monitor_cancel(config_monitor);
        g_object_unref(config_monitor);
        config_monitor = NULL;
    }
}
