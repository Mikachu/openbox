/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   themerc2css.c - Convert legacy themerc to CSS format
   Copyright (c) 2026  Openbox Development Team

   Converts Openbox 3.x themerc files to Openbox 4 CSS format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>

#define PROGRAM_NAME "openbox-themerc2css"
#define PROGRAM_VERSION "4.0.0"

typedef struct {
    gchar *name;
    gchar *value;
} ThemeProperty;

typedef struct {
    gchar *window_bg;
    gchar *window_focused_border;
    gchar *window_unfocused_border;
    gchar *title_focused_bg;
    gchar *title_unfocused_bg;
    gchar *title_focused_text;
    gchar *title_unfocused_text;
    gchar *button_focused_bg;
    gchar *button_unfocused_bg;
    gchar *menu_bg;
    gchar *menu_text;
    gchar *menu_selected_bg;
    gchar *menu_selected_text;
    gchar *border_color;
    
    gint border_width;
    gint padding;
    gint title_height;
    gint handle_height;
    gint button_size;
    gchar *title_font;
    gchar *menu_font;
} ThemeColors;

static void print_usage(const gchar *program)
{
    g_print("Usage: %s [OPTIONS] <themerc>\\n", program);
    g_print("\\n");
    g_print("Convert Openbox themerc file to CSS format.\\n");
    g_print("\\n");
    g_print("Options:\\n");
    g_print("  -o, --output FILE     Output CSS file (default: theme.css)\\n");
    g_print("  -n, --name NAME       Theme name (default: from filename)\\n");
    g_print("  -v, --verbose         Verbose output\\n");
    g_print("  -d, --dark            Generate dark mode variant\\n");
    g_print("  -h, --help            Show this help\\n");
    g_print("  --version             Show version\\n");
    g_print("\\n");
    g_print("Examples:\\n");
    g_print("  %s ~/.themes/Clearlooks/openbox-3/themerc\\n", program);
    g_print("  %s -o mytheme.css -n \"My Theme\" themerc\\n", program);
}

static gchar* normalize_color(const gchar *color)
{
    if (!color || !*color) return NULL;
    
    /* Already hex */
    if (color[0] == '#') {
        return g_strdup(color);
    }
    
    /* RGB: 255 255 255 -> #ffffff */
    if (strchr(color, ' ')) {
        gint r, g, b;
        if (sscanf(color, "%d %d %d", &r, &g, &b) == 3) {
            return g_strdup_printf("#%02x%02x%02x", 
                                   CLAMP(r, 0, 255),
                                   CLAMP(g, 0, 255),
                                   CLAMP(b, 0, 255));
        }
    }
    
    return g_strdup(color);
}

static gchar* extract_color(const gchar *value)
{
    if (!value) return NULL;
    
    /* Handle solid() syntax */
    if (g_str_has_prefix(value, "solid(")) {
        const gchar *start = value + 6;
        const gchar *end = strchr(start, ')');
        if (end) {
            gchar *inner = g_strndup(start, end - start);
            gchar *color = normalize_color(inner);
            g_free(inner);
            return color;
        }
    }
    
    /* Handle gradient() - extract first color */
    if (g_str_has_prefix(value, "gradient(")) {
        const gchar *start = strchr(value, ' ');
        if (start) {
            start++;
            const gchar *end = strchr(start, ' ');
            if (end) {
                gchar *inner = g_strndup(start, end - start);
                gchar *color = normalize_color(inner);
                g_free(inner);
                return color;
            }
        }
    }
    
    /* Plain color */
    return normalize_color(value);
}

static void parse_themerc(const gchar *path, ThemeColors *colors, 
                          GHashTable *textures, GHashTable *fonts)
{
    gchar *contents = NULL;
    gsize len;
    
    if (!g_file_get_contents(path, &contents, &len, NULL)) {
        g_warning("Failed to read %s", path);
        return;
    }
    
    gchar **lines = g_strsplit(contents, "\\n", -1);
    
    for (gint i = 0; lines[i]; i++) {
        gchar *line = lines[i];
        g_strstrip(line);
        
        /* Skip comments and empty lines */
        if (!*line || *line == '#' || *line == '!') continue;
        
        /* Parse key: value */
        gchar *colon = strchr(line, ':');
        if (!colon) continue;
        
        *colon = '\\0';
        gchar *key = line;
        gchar *value = colon + 1;
        g_strstrip(key);
        g_strstrip(value);
        
        /* Store in appropriate table */
        if (strstr(key, "Color") || strstr(key, "Bg") || strstr(key, "Text")) {
            g_hash_table_insert(textures, g_strdup(key), g_strdup(value));
            
            /* Extract to ThemeColors */
            gchar *color = extract_color(value);
            
            if (strcmp(key, "window.active.border.color") == 0) {
                colors->window_focused_border = color;
            } else if (strcmp(key, "window.inactive.border.color") == 0) {
                colors->window_unfocused_border = color;
            } else if (strcmp(key, "window.active.title.bg") == 0) {
                colors->title_focused_bg = color;
            } else if (strcmp(key, "window.inactive.title.bg") == 0) {
                colors->title_unfocused_bg = color;
            } else if (strcmp(key, "window.active.label.text.color") == 0) {
                colors->title_focused_text = color;
            } else if (strcmp(key, "window.inactive.label.text.color") == 0) {
                colors->title_unfocused_text = color;
            } else if (strcmp(key, "window.active.button.unpressed.bg") == 0) {
                colors->button_focused_bg = color;
            } else if (strcmp(key, "window.inactive.button.unpressed.bg") == 0) {
                colors->button_unfocused_bg = color;
            } else if (strcmp(key, "menu.items.bg") == 0) {
                colors->menu_bg = color;
            } else if (strcmp(key, "menu.items.text.color") == 0) {
                colors->menu_text = color;
            } else if (strcmp(key, "menu.items.active.bg") == 0) {
                colors->menu_selected_bg = color;
            } else if (strcmp(key, "menu.items.active.text.color") == 0) {
                colors->menu_selected_text = color;
            } else {
                g_free(color);
            }
        } else if (strstr(key, "font")) {
            g_hash_table_insert(fonts, g_strdup(key), g_strdup(value));
        } else {
            /* Numeric values */
            if (strcmp(key, "border.width") == 0) {
                colors->border_width = atoi(value);
            } else if (strcmp(key, "padding.width") == 0) {
                colors->padding = atoi(value);
            } else if (strcmp(key, "window.handle.height") == 0) {
                colors->handle_height = atoi(value);
            } else if (strcmp(key, "window.label.height") == 0) {
                colors->title_height = atoi(value);
            }
        }
    }
    
    g_strfreev(lines);
    g_free(contents);
}

static void generate_css(const gchar *output_path, const gchar *theme_name,
                         ThemeColors *colors, gboolean dark_mode)
{
    GString *css = g_string_new(NULL);
    
    /* Header */
    g_string_append_printf(css, "/* %s Theme for Openbox 4 */\\n", theme_name);\n    g_string_append(css, "/* Converted from legacy themerc format */\\n\\n");
    
    /* CSS Variables */
    g_string_append(css, ":root {\\n");
    g_string_append_printf(css, "  /* Theme: %s */\\n", theme_name);
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Window colors */\\n");
    g_string_append_printf(css, "  --window-bg: %s;\\n", 
                          colors->title_focused_bg ? colors->title_focused_bg : "#edeceb");
    g_string_append_printf(css, "  --window-border: %s;\\n",
                          colors->window_focused_border ? colors->window_focused_border : "#bfb8b0");
    g_string_append_printf(css, "  --window-border-unfocused: %s;\\n",
                          colors->window_unfocused_border ? colors->window_unfocused_border : "#bfb8b0");
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Titlebar colors */\\n");
    g_string_append_printf(css, "  --titlebar-bg: %s;\\n",
                          colors->title_focused_bg ? colors->title_focused_bg : "#edeceb");
    g_string_append_printf(css, "  --titlebar-bg-unfocused: %s;\\n",
                          colors->title_unfocused_bg ? colors->title_unfocused_bg : "#edeceb");
    g_string_append_printf(css, "  --title-color: %s;\\n",
                          colors->title_focused_text ? colors->title_focused_text : "#000000");
    g_string_append_printf(css, "  --title-color-unfocused: %s;\\n",
                          colors->title_unfocused_text ? colors->title_unfocused_text : "#606060");
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Button colors */\\n");
    g_string_append_printf(css, "  --button-bg: %s;\\n",
                          colors->button_focused_bg ? colors->button_focused_bg : "transparent");
    g_string_append_printf(css, "  --button-bg-unfocused: %s;\\n",
                          colors->button_unfocused_bg ? colors->button_unfocused_bg : "transparent");
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Menu colors */\\n");
    g_string_append_printf(css, "  --menu-bg: %s;\\n",
                          colors->menu_bg ? colors->menu_bg : "#ffffff");
    g_string_append_printf(css, "  --menu-text: %s;\\n",
                          colors->menu_text ? colors->menu_text : "#000000");
    g_string_append_printf(css, "  --menu-selected-bg: %s;\\n",
                          colors->menu_selected_bg ? colors->menu_selected_bg : "#7c99c1");
    g_string_append_printf(css, "  --menu-selected-text: %s;\\n",
                          colors->menu_selected_text ? colors->menu_selected_text : "#ffffff");
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Typography */\\n");
    g_string_append(css, "  --font-family: sans-serif;\\n");
    g_string_append(css, "  --font-size: 8pt;\\n");
    g_string_append(css, "  \\n");
    g_string_append(css, "  /* Dimensions */\\n");
    g_string_append_printf(css, "  --border-width: %dpx;\\n", colors->border_width);
    g_string_append_printf(css, "  --padding: %dpx;\\n", colors->padding);
    g_string_append_printf(css, "  --titlebar-height: %dpx;\\n", 
                          colors->title_height > 0 ? colors->title_height : 24);
    g_string_append_printf(css, "  --button-size: %dpx;\\n", colors->button_size);
    g_string_append(css, "}\\n\\n");
    
    /* Window styles */
    g_string_append(css, "/* Window */\\n");
    g_string_append(css, "window {\\n");
    g_string_append(css, "  background: var(--window-bg);\\n");
    g_string_append_printf(css, "  border: %dpx solid var(--window-border);\\n", colors->border_width);
    g_string_append(css, "  border-radius: 4px 4px 0 0;\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "window:focus {\\n");
    g_string_append(css, "  border-color: var(--window-border);\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "window:unfocused {\\n");
    g_string_append(css, "  border-color: var(--window-border-unfocused);\\n");
    g_string_append(css, "}\\n\\n");
    
    /* Titlebar */
    g_string_append(css, "/* Titlebar */\\n");
    g_string_append(css, "titlebar {\\n");
    g_string_append(css, "  background: var(--titlebar-bg);\\n");
    g_string_append_printf(css, "  height: %dpx;\\n", 
                          colors->title_height > 0 ? colors->title_height : 24);
    g_string_append_printf(css, "  padding: %dpx;\\n", colors->padding);
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "window:focus titlebar {\\n");
    g_string_append(css, "  background: var(--titlebar-bg);\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "window:unfocused titlebar {\\n");
    g_string_append(css, "  background: var(--titlebar-bg-unfocused);\\n");
    g_string_append(css, "}\\n\\n");
    
    /* Title */
    g_string_append(css, "/* Title */\\n");
    g_string_append(css, "title {\\n");
    g_string_append(css, "  color: var(--title-color);\\n");
    g_string_append(css, "  font-family: var(--font-family);\\n");
    g_string_append(css, "  font-size: var(--font-size);\\n");
    g_string_append(css, "  font-weight: bold;\\n");
    g_string_append(css, "  text-align: center;\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "window:unfocused title {\\n");
    g_string_append(css, "  color: var(--title-color-unfocused);\\n");
    g_string_append(css, "}\\n\\n");
    
    /* Buttons */
    g_string_append(css, "/* Buttons */\\n");
    g_string_append(css, "button {\\n");
    g_string_printf(css, "  size: %dpx;\\n", colors->button_size > 0 ? colors->button_size : 16);
    g_string_append(css, "  background: var(--button-bg);\\n");
    g_string_append(css, "}\\n\\n");
    
    /* Menu */
    g_string_append(css, "/* Menu */\\n");
    g_string_append(css, "menu {\\n");
    g_string_append(css, "  background: var(--menu-bg);\\n");
    g_string_append(css, "  border: 1px solid var(--window-border);\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "menu-item {\\n");
    g_string_append(css, "  color: var(--menu-text);\\n");
    g_string_append(css, "}\\n\\n");
    
    g_string_append(css, "menu-item:hover {\\n");
    g_string_append(css, "  background: var(--menu-selected-bg);\\n");
    g_string_append(css, "  color: var(--menu-selected-text);\\n");
    g_string_append(css, "}\\n\\n");
    
    /* Dark mode media query if requested */
    if (dark_mode) {
        g_string_append(css, "/* Dark mode */\\n");
        g_string_append(css, "@media (prefers-color-scheme: dark) {\\n");
        g_string_append(css, "  :root {\\n");
        g_string_append(css, "    --window-bg: #2d2d2d;\\n");
        g_string_append(css, "    --window-border: #404040;\\n");
        g_string_append(css, "    --titlebar-bg: #3d3d3d;\\n");
        g_string_append(css, "    --title-color: #e0e0e0;\\n");
        g_string_append(css, "    --menu-bg: #2d2d2d;\\n");
        g_string_append(css, "    --menu-text: #e0e0e0;\\n");
        g_string_append(css, "    --menu-selected-bg: #5e81ac;\\n");
        g_string_append(css, "  }\\n");
        g_string_append(css, "}\\n");
    }
    
    /* Write output */
    if (g_file_set_contents(output_path, css->str, -1, NULL)) {
        g_print("Created: %s\\n", output_path);
    } else {\n        g_printerr("Error: Failed to write %s\\n", output_path);
    }
    
    g_string_free(css, TRUE);
}

int main(int argc, char **argv)
{
    gchar *output_path = NULL;
    gchar *theme_name = NULL;
    gboolean verbose = FALSE;
    gboolean dark_mode = FALSE;
    gchar *input_path = NULL;
    
    /* Parse arguments */\n    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {\n            g_print("%s version %s\\n", PROGRAM_NAME, PROGRAM_VERSION);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (++i >= argc) {
                g_printerr("Error: --output requires an argument\\n");
                return 1;\n            }
n            output_path = argv[i];\n        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {\            if (++i >= argc) {\n                g_printerr("Error: --name requires an argument\\n");
                return 1;\n            }\n            theme_name = argv[i];\n        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {\n            verbose = TRUE;\n        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dark") == 0) {\n            dark_mode = TRUE;\n        } else if (argv[i][0] != '-') {\n            input_path = argv[i];\n        }\n    }\n    \n    if (!input_path) {\n        g_printerr("Error: No input file specified\\n");\n        print_usage(argv[0]);\n        return 1;\n    }\n    \n    /* Default output */\n    if (!output_path) {\n        output_path = \"theme.css\";\n    }\n    \n    /* Default theme name from input filename */\n    if (!theme_name) {\n        theme_name = g_path_get_basename(input_path);\n        gchar *dot = strrchr(theme_name, '.');\n        if (dot) *dot = '\\0';\n    }\n    \n    if (verbose) {\n        g_print(\"Converting: %s -> %s\\n\", input_path, output_path);\n        g_print(\"Theme name: %s\\n\", theme_name);\n    }\n    \n    /* Parse and convert */\n    ThemeColors colors = {0};\n    GHashTable *textures = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);\n    GHashTable *fonts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);\n    \n    parse_themerc(input_path, &colors, textures, fonts);\n    \n    if (verbose) {\n        g_print(\"Found %d color properties\\n\", g_hash_table_size(textures));\n    }\n    \n    generate_css(output_path, theme_name, &colors, dark_mode);\n    \n    /* Cleanup */\n    g_free(colors.window_bg);\n    g_free(colors.window_focused_border);\n    g_free(colors.window_unfocused_border);\n    g_free(colors.title_focused_bg);\n    g_free(colors.title_unfocused_bg);\n    g_free(colors.title_focused_text);\n    g_free(colors.title_unfocused_text);\n    g_free(colors.button_focused_bg);\n    g_free(colors.button_unfocused_bg);\n    g_free(colors.menu_bg);\n    g_free(colors.menu_text);\n    g_free(colors.menu_selected_bg);\n    g_free(colors.menu_selected_text);\n    g_free(colors.border_color);\n    \n    g_hash_table_destroy(textures);\n    g_hash_table_destroy(fonts);\n    \n    return 0;\n}
