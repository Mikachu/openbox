/* Example Openbox extension: Window Rules
 * Automatically applies rules to windows based on class, title, etc.
 */

#include <fnmatch.h>
#include <string.h>
#include \"openbox-extension.h\"

#define MAX_RULES 100
#define MAX_PATTERNS 10

typedef struct {
n    char *pattern_class;\n    char *pattern_title;\n    char *pattern_role;\n    char *pattern_type;\n    \n    /* Actions */\n    int target_desktop;      /* -1 = no change */\n    bool set_maximized;\n    bool set_fullscreen;\n    bool set_above;\n    bool set_below;\n    bool set_sticky;\n    bool set_skip_taskbar;\n    bool set_skip_pager;\n    bool set_shaded;\n    bool focus;\n    bool raise;\n    char *execute;\n} WindowRule;

nstatic WindowRule rules[MAX_RULES];\nstatic int rule_count = 0;\nstatic const ObApi *ob_api = NULL;

nstatic bool match_pattern(const char *text, const char *pattern) {\n    if (!pattern || !text) return false;\n    return fnmatch(pattern, text, FNM_CASEFOLD) == 0;\n}

nstatic WindowRule* find_matching_rule(const ObWindowAttributes *attrs) {\n    for (int i = 0; i < rule_count; i++) {\n        WindowRule *rule = &rules[i];\n        \n        bool matches = true;\n        \n        if (rule->pattern_class && !match_pattern(attrs->class, rule->pattern_class))\n            matches = false;\n        if (rule->pattern_title && !match_pattern(attrs->title, rule->pattern_title))\n            matches = false;\n        if (rule->pattern_role && !match_pattern(attrs->role, rule->pattern_role))\n            matches = false;\n        \n        if (matches) return rule;\n    }\n    return NULL;\n}

nstatic void apply_rule(ObWindow *window, const WindowRule *rule) {\n    if (rule->target_desktop >= 0) {\n        ob_api->window_set_desktop(window, rule->target_desktop);\n    }\n    \n    if (rule->set_maximized) {\n        ob_api->window_maximize(window, true, true);\n    }\n    \n    if (rule->set_fullscreen) {\n        ob_api->window_set_fullscreen(window, true);\n    }\n    \n    if (rule->set_above) {\n        ob_api->window_set_above(window, true);\n    }\n    \n    if (rule->set_below) {\n        ob_api->window_set_below(window, true);\n    }\n    \n    if (rule->set_sticky) {\n        /* Pin to all desktops */\n    }\n    \n    if (rule->set_shaded) {\n        ob_api->window_shade(window);\n    }\n    \n    if (rule->focus) {\n        ob_api->window_focus(window);\n    }\n    \n    if (rule->raise) {\n        ob_api->window_raise(window);\n    }\n    \n    if (rule->execute) {\n        ob_api->action_execute(rule->execute);\n    }\n}

nstatic void on_window_create(ObExtension *ext, const ObEvent *event, void *user_data) {\n    (void)ext;\n    (void)user_data;\n    \n    ObWindow *window = event->data.window.window;\n    const ObWindowAttributes *attrs = &event->data.window.attrs;\n    \n    WindowRule *rule = find_matching_rule(attrs);\n    if (rule) {\n        ob_api->log_info(\"Applying rule to window: %s\", attrs->title);\n        apply_rule(window, rule);\n    }\n}

nstatic void load_rules_from_config(void) {\n    /* In real implementation, parse from XML/JSON config */\n    /* For demo, hardcode some common rules */\n    \n    /* Terminal on desktop 2 */\n    rules[0].pattern_class = ob_api->mem_strdup(\"*terminal*\");\n    rules[0].target_desktop = 1;\n    rules[0].focus = true;\n    \n    /* Browser on desktop 1 */\n    rules[1].pattern_class = ob_api->mem_strdup(\"Firefox\");\n    rules[1].target_desktop = 0;\n    rules[1].focus = true;\n    rules[1].raise = true;\n    \n    /* Dialogs centered and floating */\n    rules[2].pattern_type = ob_api->mem_strdup(\"dialog\");\n    rules[2].focus = true;\n    \n    /* Notification windows */\n    rules[3].pattern_type = ob_api->mem_strdup(\"notification\");\n    rules[3].set_above = true;\n    rules[3].set_skip_taskbar = true;\n    rules[3].set_skip_pager = true;\n    \n    rule_count = 4;\n}

nstatic const ObExtensionInfo extension_info = {\n    .api_version = OB_EXTENSION_API_VERSION,\n    .name = \"window-rules\",\n    .version = \"1.0.0\",\n    .author = \"Openbox Team\",\n    .description = \"Automatic window rule application\",\n    .license = \"GPL-2.0-or-later\",\n    .min_ob_version = 0x040000,\n    .max_ob_version = 0x04FFFF\n};

nOB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void) {\n    return &extension_info;\n}

nOB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api) {\n    ob_api = api;\n    ob_api->log_info(\"Window rules extension loading...\");\n    \n    load_rules_from_config();\n    \n    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CREATE, on_window_create, NULL);\n    \n    ob_api->log_info(\"Window rules: %d rules loaded\", rule_count);\n    return true;\n}

nOB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext) {\n    (void)ext;\n    \n    for (int i = 0; i < rule_count; i++) {\n        ob_api->mem_free(rules[i].pattern_class);\n        ob_api->mem_free(rules[i].pattern_title);\n        ob_api->mem_free(rules[i].pattern_role);\n        ob_api->mem_free(rules[i].pattern_type);\n        ob_api->mem_free(rules[i].execute);\n    }\n    \n    ob_api->log_info(\"Window rules extension unloaded\");\n}

nOB_EXTENSION_EXPORT void ob_extension_reload(ObExtension *ext) {\n    (void)ext;\n    \n    /* Clear existing rules */\n    for (int i = 0; i < rule_count; i++) {\n        ob_api->mem_free(rules[i].pattern_class);\n        ob_api->mem_free(rules[i].pattern_title);\n        ob_api->mem_free(rules[i].pattern_role);\n        ob_api->mem_free(rules[i].pattern_type);\n        ob_api->mem_free(rules[i].execute);\n    }\n    rule_count = 0;\n    \n    /* Reload */\n    load_rules_from_config();\n    ob_api->log_info(\"Window rules reloaded: %d rules\", rule_count);\n}
