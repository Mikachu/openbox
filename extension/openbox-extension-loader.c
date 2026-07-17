/* Openbox Extension Loader
 * Dynamically loads and manages extensions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <glib.h>
#include \"openbox-extension.h\"

#define EXTENSION_DIR SYSCONFDIR \"/openbox/extensions\"
#define USER_EXTENSION_DIR \".config/openbox/extensions\"

typedef struct _ObExtensionPrivate {
n    void *handle;\n    char *path;\n    char *name;\n    bool loaded;\n    const ObExtensionInfo *info;\n    ObExtensionInitFunc init_func;\n    ObExtensionFiniFunc fini_func;\n    ObExtensionReloadFunc reload_func;\n} ObExtensionPrivate;

n/* Global extension list */\nstatic GList *extensions = NULL;
nstatic const ObApi *api_table = NULL;

n/* API implementation - filled in by Openbox core */
nstatic const ObApi ob_api = {
n    .get_api_version = ob_ext_get_api_version,
n    .check_version = ob_ext_check_version,
n    \n    /* Logging */\n    .log_debug = ob_ext_log_debug,
n    .log_info = ob_ext_log_info,
n    .log_warning = ob_ext_log_warning,
n    .log_error = ob_ext_log_error,
n    .set_log_callback = ob_ext_set_log_callback,
n    \n    /* Window management - implemented in core */\n    .window_get_focused = ob_core_window_get_focused,
n    .window_get_under_mouse = ob_core_window_get_under_mouse,
n    .window_get_all = ob_core_window_get_all,
n    .window_list_free = ob_core_window_list_free,
n    .window_get_attributes = ob_core_window_get_attributes,
n    .window_attributes_free = ob_core_window_attributes_free,
n    .window_get_id = ob_core_window_get_id,
n    .window_is_focused = ob_core_window_is_focused,
n    .window_is_visible = ob_core_window_is_visible,
n    .window_focus = ob_core_window_focus,
n    .window_raise = ob_core_window_raise,
n    .window_lower = ob_core_window_lower,
n    .window_close = ob_core_window_close,
n    .window_kill = ob_core_window_kill,
n    .window_iconify = ob_core_window_iconify,
n    .window_deiconify = ob_core_window_deiconify,
n    .window_maximize = ob_core_window_maximize,
n    .window_unmaximize = ob_core_window_unmaximize,
n    .window_shade = ob_core_window_shade,
n    .window_unshade = ob_core_window_unshade,
n    .window_set_fullscreen = ob_core_window_set_fullscreen,
n    .window_set_above = ob_core_window_set_above,
n    .window_set_below = ob_core_window_set_below,
n    .window_set_desktop = ob_core_window_set_desktop,
n    .window_set_geometry = ob_core_window_set_geometry,
n    .window_move = ob_core_window_move,
n    .window_resize = ob_core_window_resize,
n    \n    /* Desktop management */\n    .desktop_get_count = ob_core_desktop_get_count,
n    .desktop_get_current = ob_core_desktop_get_current,\n    .desktop_set_current = ob_core_desktop_set_current,\n    .desktop_add = ob_core_desktop_add,
n    .desktop_remove = ob_core_desktop_remove,\n    .desktop_get_name = ob_core_desktop_get_name,\n    .desktop_set_name = ob_core_desktop_set_name,\n    .desktop_get_windows = ob_core_desktop_get_windows,
n    \n    /* Screen management */\n    .screen_get_count = ob_core_screen_get_count,
n    .screen_get_geometry = ob_core_screen_get_geometry,
n    .screen_get_usable_area = ob_core_screen_get_usable_area,
n    .screen_get_current = ob_core_screen_get_current,
n    \n    /* Configuration */\n    .config_get_string = ob_core_config_get_string,
n    .config_get_int = ob_core_config_get_int,
n    .config_get_bool = ob_core_config_get_bool,
n    .config_get_double = ob_core_config_get_double,
n    .config_set = ob_core_config_set,
n    .config_reload = ob_core_config_reload,
n    \n    /* Theme */\n    .theme_get_name = ob_core_theme_get_name,
n    .theme_get_variant = ob_core_theme_get_variant,
n    .theme_get_color = ob_core_theme_get_color,
n    .theme_reload = ob_core_theme_reload,
n    \n    /* Events */\n    .event_subscribe = ob_core_event_subscribe,
n    .event_unsubscribe = ob_core_event_unsubscribe,
n    .event_unsubscribe_all = ob_core_event_unsubscribe_all,
n    \n    /* Timers */\n    .timer_add = ob_core_timer_add,
n    .timer_remove = ob_core_timer_remove,
n    \n    /* Menus */\n    .menu_show = ob_core_menu_show,
n    .menu_hide = ob_core_menu_hide,
n    \n    /* Actions */\n    .action_execute = ob_core_action_execute,
n    .action_show_menu = ob_core_action_show_menu,
n    .action_next_window = ob_core_action_next_window,
n    .action_previous_window = ob_core_action_previous_window,
n    .action_restart = ob_core_action_restart,
n    .action_exit = ob_core_action_exit,
n    \n    /* Extension management */\n    .extension_get = ob_core_extension_get,
n    .extension_get_symbol = ob_core_extension_get_symbol,
n    .extension_emit_signal = ob_core_extension_emit_signal,
n    .extension_connect_signal = ob_core_extension_connect_signal,
n    \n    /* Memory */\n    .mem_alloc = malloc,
n    .mem_free = free,
n    .mem_strdup = strdup
};

nbool ob_extension_load(const char *path) {
n    g_debug(\"Loading extension from: %s\", path);
n    \n    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
n    if (!handle) {\n        g_warning(\"Failed to load extension %s: %s\", path, dlerror());\n        return false;\n    }\n    \n    /* Get extension info */\n    ObExtensionInfoFunc info_func = dlsym(handle, \"ob_extension_get_info\");\n    if (!info_func) {\n        g_warning(\"Extension %s missing ob_extension_get_info\", path);\n        dlclose(handle);\n        return false;\n    }\n    \n    const ObExtensionInfo *info = info_func();\n    if (!info) {\n        g_warning(\"Extension %s returned NULL info\", path);\n        dlclose(handle);\n        return false;\n    }\n    \n    /* Check API version */\n    if (info->api_version > OB_EXTENSION_API_VERSION) {\n        g_warning(\"Extension %s requires newer API: %u > %u\",\n                  path, info->api_version, OB_EXTENSION_API_VERSION);\n        dlclose(handle);\n        return false;\n    }\n    \n    /* Check Openbox version compatibility */\n    uint32_t ob_version = 0x040000; /* TODO: Get actual version */\n    if (ob_version < info->min_ob_version || ob_version > info->max_ob_version) {\n        g_warning(\"Extension %s incompatible with Openbox version\", path);\n        dlclose(handle);\n        return false;\n    }\n    \n    /* Get entry points */\n    ObExtensionInitFunc init_func = dlsym(handle, \"ob_extension_init\");\n    ObExtensionFiniFunc fini_func = dlsym(handle, \"ob_extension_fini\");\n    \n    if (!init_func) {\n        g_warning(\"Extension %s missing ob_extension_init\", path);\n        dlclose(handle);\n        return false;\n    }\n    \n    /* Create extension object */\n    ObExtensionPrivate *ext = g_new0(ObExtensionPrivate, 1);\n    ext->handle = handle;\n    ext->path = g_strdup(path);\n    ext->name = g_strdup(info->name);\n    ext->info = info;\n    ext->init_func = init_func;\n    ext->fini_func = fini_func;\n    ext->reload_func = dlsym(handle, \"ob_extension_reload\");\n    \n    /* Initialize extension */\n    if (!init_func((ObExtension*)ext, &ob_api)) {\n        g_warning(\"Extension %s failed to initialize\", path);\n        g_free(ext->path);\n        g_free(ext->name);\n        g_free(ext);\n        dlclose(handle);\n        return false;\n    }\n    \n    ext->loaded = true;\n    extensions = g_list_append(extensions, ext);\n    \n    g_message(\"Loaded extension: %s v%s by %s\",\n              info->name, info->version, info->author);\n    \n    return true;\n}

nvoid ob_extension_unload(ObExtensionPrivate *ext) {
n    if (!ext) return;\n    \n    g_debug(\"Unloading extension: %s\", ext->name);\n    \n    if (ext->fini_func) {\n        ext->fini_func((ObExtension*)ext);\n    }\n    \n    if (ext->handle) {\n        dlclose(ext->handle);\n    }\n    \n    extensions = g_list_remove(extensions, ext);\n    \n    g_free(ext->path);\n    g_free(ext->name);\n    g_free(ext);\n}

nvoid ob_extension_unload_all(void) {\n    GList *l = extensions;\n    while (l) {\n        ObExtensionPrivate *ext = l->data;\n        l = l->next;\n        ob_extension_unload(ext);\n    }\n    extensions = NULL;\n}

nvoid ob_extension_reload(ObExtensionPrivate *ext) {\n    if (!ext || !ext->reload_func) return;\n    \n    g_debug(\"Reloading extension: %s\", ext->name);\n    ext->reload_func((ObExtension*)ext);\n}

nvoid ob_extension_reload_all(void) {
n    for (GList *l = extensions; l; l = l->next) {\n        ob_extension_reload(l->data);\n    }\n}

nvoid ob_extension_autoload(void) {
n    /* System extensions */\n    GDir *dir = g_dir_open(EXTENSION_DIR, 0, NULL);\n    if (dir) {\n        const char *name;\n        while ((name = g_dir_read_name(dir))) {\n            if (g_str_has_suffix(name, \".so\")) {\n                char *path = g_build_filename(EXTENSION_DIR, name, NULL);\n                ob_extension_load(path);\n                g_free(path);\n            }\n        }\n        g_dir_close(dir);\n    }\n    \n    /* User extensions */\n    char *user_dir = g_build_filename(g_get_home_dir(), USER_EXTENSION_DIR, NULL);\n    dir = g_dir_open(user_dir, 0, NULL);\n    if (dir) {\n        const char *name;\n        while ((name = g_dir_read_name(dir))) {\n            if (g_str_has_suffix(name, \".so\")) {\n                char *path = g_build_filename(user_dir, name, NULL);\n                ob_extension_load(path);\n                g_free(path);\n            }\n        }\n        g_dir_close(dir);\n    }\n    g_free(user_dir);\n}

nObExtensionPrivate* ob_extension_find(const char *name) {
n    for (GList *l = extensions; l; l = l->next) {\n        ObExtensionPrivate *ext = l->data;\n        if (g_strcmp0(ext->name, name) == 0) {\n            return ext;\n        }\n    }\n    return NULL;\n}

nGList* ob_extension_list(void) {\n    return extensions;\n}

nconst char* ob_extension_get_name(ObExtensionPrivate *ext) {
n    return ext ? ext->name : NULL;\n}

nconst ObExtensionInfo* ob_extension_get_info(ObExtensionPrivate *ext) {
n    return ext ? ext->info : NULL;\n}

nbool ob_extension_is_loaded(ObExtensionPrivate *ext) {
n    return ext ? ext->loaded : false;\n}
