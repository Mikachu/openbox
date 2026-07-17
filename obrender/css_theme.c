/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   css_theme.c - CSS Theme Integration for Openbox Renderer
   Copyright (c) 2026  Openbox Development Team

   Bridges CSS stylesheet data to RrTheme rendering structures.
*/

#include "css_theme.h"
#include "theme.h"
#include "color.h"
#include "font.h"

/* ============================================================================
 * CSS to RrTheme Conversion
 * ============================================================================ */

static gboolean css_get_color(CssStylesheet *sheet, const gchar *element,
                               const gchar *state, const gchar *property,
                               RrColor **color)
{
    CssValue *val = css_stylesheet_get_property(sheet, element, state, property);
    if (!val) return FALSE;
    
    return css_value_to_color(val, color);
}

static gboolean css_get_length(CssStylesheet *sheet, const gchar *element,
                                const gchar *state, const gchar *property,
                                gint *pixels, gint base)
{
    CssValue *val = css_stylesheet_get_property(sheet, element, state, property);
    if (!val) return FALSE;
    
    return css_value_to_length(val, pixels, base);
}

static gchar* css_get_string(CssStylesheet *sheet, const gchar *element,
                              const gchar *state, const gchar *property)
{
    CssValue *val = css_stylesheet_get_property(sheet, element, state, property);
    if (!val) return NULL;
    
    return css_value_to_string(val);
}

/* ============================================================================
 * Theme Loading
 * ============================================================================ */

RrTheme* ob_css_theme_load(const RrInstance *inst, const gchar *name,
                            gboolean allow_fallback, GError **error)
{
    g_return_val_if_fail(inst != NULL, NULL);
    g_return_val_if_fail(name != NULL, NULL);
    
    /* Search for CSS theme */
    gchar *css_path = NULL;
    const gchar * const *dirs = g_get_system_data_dirs();
    
    for (int i = 0; dirs[i]; i++) {
        css_path = g_build_filename(dirs[i], "themes", name, "openbox-4", 
                                     "theme.css", NULL);
        if (g_file_test(css_path, G_FILE_TEST_EXISTS)) {
            break;
        }
        g_free(css_path);
        css_path = NULL;
    }
    
    /* Try user themes */
    if (!css_path) {
        css_path = g_build_filename(g_get_user_data_dir(), "themes", name,
                                   "openbox-4", "theme.css", NULL);
        if (!g_file_test(css_path, G_FILE_TEST_EXISTS)) {
            g_free(css_path);
            css_path = NULL;
        }
    }
    
    /* Try legacy location */
    if (!css_path) {
        css_path = g_build_filename(g_get_home_dir(), ".themes", name,
                                   "openbox-4", "theme.css", NULL);
        if (!g_file_test(css_path, G_FILE_TEST_EXISTS)) {
            g_free(css_path);
            css_path = NULL;
        }
    }
    
    /* Fall back to legacy themerc if no CSS found */
    if (!css_path) {
        if (allow_fallback) {
            g_message("CSS theme '%s' not found, trying legacy format", name);
            return RrThemeNew(inst, name, allow_fallback,
                             NULL, NULL, NULL, NULL, NULL, NULL);
        } else {
            g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                       "CSS theme '%s' not found", name);
            return NULL;
        }
    }
    
    g_message("Loading CSS theme: %s", css_path);
    
    /* Load CSS */
    CssStylesheet *sheet = css_stylesheet_new(css_path);
    if (!css_stylesheet_load_file(sheet, css_path, error)) {
        css_stylesheet_free(sheet);
        g_free(css_path);
        return NULL;
    }
    
    g_free(css_path);
    
    /* Create theme and populate from CSS */
    RrTheme *theme = g_new0(RrTheme, 1);
    theme->inst = inst;
    theme->name = g_strdup(name);
    
    if (!ob_css_theme_apply(theme, sheet)) {
        g_warning("Failed to apply CSS theme fully");
    }
    
    css_stylesheet_free(sheet);
    
    return theme;
}

gboolean ob_css_theme_apply(RrTheme *theme, CssStylesheet *sheet)
{
    g_return_val_if_fail(theme != NULL, FALSE);
    g_return_val_if_fail(sheet != NULL, FALSE);
    
    const RrInstance *inst = theme->inst;
    gint base_font_size = 9; /* Default base size for em calculations */
    
    /* Window border colors */
    css_get_color(sheet, CSS_SELECTOR_WINDOW, "focus", 
                  CSS_PROP_WINDOW_BORDER_COLOR,
                  &theme->frame_focused_border_color);
    css_get_color(sheet, CSS_SELECTOR_WINDOW, "unfocused",
                  CSS_PROP_WINDOW_BORDER_COLOR,
                  &theme->frame_unfocused_border_color);
    
    /* Titlebar colors */
    css_get_color(sheet, CSS_SELECTOR_TITLEBAR, NULL,
                  CSS_PROP_TITLEBAR_BG,
                  &theme->title_focused_color);
    css_get_color(sheet, CSS_SELECTOR_TITLEBAR, NULL,
                  "background-unfocused",
                  &theme->title_unfocused_color);
    
    /* Title colors */
    css_get_color(sheet, CSS_SELECTOR_TITLE, NULL,
                  CSS_PROP_TITLE_COLOR,
                  &theme->title_focused_color);
    css_get_color(sheet, CSS_SELECTOR_TITLE, "unfocused",
                  CSS_PROP_TITLE_COLOR_UNFOCUSED,
                  &theme->title_unfocused_color);
    
    /* Title dimensions */
    css_get_length(sheet, CSS_SELECTOR_TITLEBAR, NULL,
                   CSS_PROP_TITLEBAR_HEIGHT,
                   &theme->title_height, base_font_size);
    css_get_length(sheet, CSS_SELECTOR_TITLEBAR, NULL,
                   CSS_PROP_TITLEBAR_PADDING,
                   &theme->paddingx, base_font_size);
    
    /* Button colors */
    css_get_color(sheet, CSS_SELECTOR_BUTTON, NULL,
                  CSS_PROP_BUTTON_BG,
                  &theme->titlebut_focused_unpressed_color);
    css_get_color(sheet, CSS_SELECTOR_BUTTON, "hover",
                  CSS_PROP_BUTTON_BG_HOVER,
                  &theme->titlebut_focused_hover_color);
    css_get_color(sheet, CSS_SELECTOR_BUTTON, "active",
                  CSS_PROP_BUTTON_BG_ACTIVE,
                  &theme->titlebut_focused_pressed_color);
    
    /* Button size */
    css_get_length(sheet, CSS_SELECTOR_BUTTON, NULL,
                    CSS_PROP_BUTTON_SIZE,
                    &theme->button_size, base_font_size);
    
    /* Menu colors */
    css_get_color(sheet, CSS_SELECTOR_MENU, NULL,
                  CSS_PROP_MENU_BG,
                  &theme->menu_color);
    css_get_color(sheet, CSS_SELECTOR_MENU_ITEM, NULL,
                  CSS_PROP_MENU_ITEM_COLOR,
                  &theme->menu_text_color);
    css_get_color(sheet, CSS_SELECTOR_MENU_ITEM, "hover",
                  CSS_PROP_MENU_ITEM_COLOR_HOVER,
                  &theme->menu_selected_color);
    
    /* Menu dimensions */
    css_get_length(sheet, CSS_SELECTOR_MENU, NULL,
                   CSS_PROP_MENU_BORDER_WIDTH,
                   &theme->mbwidth, base_font_size);
    
    /* Handle/grip */
    css_get_length(sheet, CSS_SELECTOR_HANDLE, NULL,
                    CSS_PROP_HANDLE_HEIGHT,
                    &theme->handle_height, base_font_size);
    
    /* Fonts */
    gchar *font_family = css_get_string(sheet, CSS_SELECTOR_TITLE, NULL,
                                        CSS_PROP_TITLE_FONT);
    gchar *font_size_str = css_get_string(sheet, CSS_SELECTOR_TITLE, NULL,
                                           CSS_PROP_TITLE_FONT_SIZE);
    
    if (font_family || font_size_str) {
        gint font_size = 8;
        if (font_size_str) {
            font_size = atoi(font_size_str);
            g_free(font_size_str);
        }
        
        gchar *font_desc = g_strdup_printf("%s %d",
                                           font_family ? font_family : "sans",
                                           font_size);
        theme->win_font_focused = RrFontOpen(inst, font_desc);
        theme->win_font_unfocused = RrFontOpen(inst, font_desc);
        g_free(font_desc);
    }
    
    g_free(font_family);
    
    /* Fallback fonts */
    if (!theme->win_font_focused) {
        theme->win_font_focused = RrFontOpen(inst, "sans 8");
    }
    if (!theme->win_font_unfocused) {
        theme->win_font_unfocused = RrFontOpen(inst, "sans 8");
    }
    
    /* Calculate derived values */
    theme->label_height = theme->title_height - theme->paddingx * 2;
    theme->win_font_height = RrFontHeight(theme->win_font_focused);
    theme->title_height = MAX(theme->title_height, theme->win_font_height + 
                                               theme->paddingx * 2);
    theme->grip_width = theme->handle_height;
    
    /* Set default border width if not specified */
    if (theme->fbwidth == 0) {
        theme->fbwidth = 1;
    }
    
    return TRUE;
}

/* ============================================================================
 * Dark Mode Support
 * ============================================================================ */

void ob_css_theme_set_dark_mode(ObCssTheme *theme, gboolean dark)
{
    g_return_if_fail(theme != NULL);
    theme->prefers_dark = dark;
    
    /* Reload with dark media query */
    if (theme->stylesheet) {
        /* In a full implementation, this would re-parse with
           @media (prefers-color-scheme: dark) rules enabled */
    }
}

gboolean ob_css_theme_has_dark_mode(ObCssTheme *theme)
{
    g_return_val_if_fail(theme != NULL, FALSE);
    return theme->supports_dark_mode;
}

/* ============================================================================
 * Theme Conversion
 * ============================================================================ */

gboolean ob_theme_convert_themerc_to_css(const gchar *themerc_path,
                                          const gchar *css_output_path,
                                          GError **error)
{
    g_return_val_if_fail(themerc_path != NULL, FALSE);
    g_return_val_if_fail(css_output_path != NULL, FALSE);
    
    if (!g_file_test(themerc_path, G_FILE_TEST_EXISTS)) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
                   "Themerc file not found: %s", themerc_path);
        return FALSE;
    }
    
    /* Use external converter tool or internal conversion */
    gchar *cmd = g_strdup_printf("openbox-themerc2css -o \"%s\" \"%s\"",
                                  css_output_path, themerc_path);
    
    gint exit_status;
    gboolean success = g_spawn_command_line_sync(cmd, NULL, NULL, 
                                                  &exit_status, error);
    g_free(cmd);
    
    return success && exit_status == 0;
}

/* ============================================================================
 * CSS Value Helpers
 * ============================================================================ */

CssValue* css_value_new_color(guint8 r, guint8 g, guint8 b, guint8 a)
{
    CssValue *v = css_value_new(CSS_VALUE_COLOR);
    v->data.color.r = r;
    v->data.color.g = g;
    v->data.color.b = b;
    v->data.color.a = a;
    return v;
}

CssValue* css_value_new_length(gdouble num, CssUnit unit)
{
    CssValue *v = css_value_new(CSS_VALUE_LENGTH);
    v->data.number = num;
    v->unit = unit;
    return v;
}

CssValue* css_value_new_string(const gchar *str)
{
    CssValue *v = css_value_new(CSS_VALUE_STRING);
    v->data.string = g_strdup(str);
    return v;
}

CssValue* css_value_new_ident(const gchar *ident)
{
    CssValue *v = css_value_new(CSS_VALUE_IDENT);
    v->data.string = g_strdup(ident);
    return v;
}

/* ============================================================================
 * Theme Caching
 * ============================================================================ */

void ob_css_theme_cache_clear(RrTheme *theme)
{
    /* Clear computed value cache for dynamic updates */
    if (!theme) return;
    
    /* In a full implementation, this would clear cached RrAppearance
       objects so they get regenerated from CSS on next use */
}

/* ============================================================================
 * Debug Helpers
 * ============================================================================ */

void css_stylesheet_dump(CssStylesheet *sheet, const gchar *element_filter)
{
    if (!sheet) {
        g_print("Stylesheet is NULL\\n");
        return;
    }
    
    g_print("Stylesheet: %s\\n", sheet->source_path ? sheet->source_path : "<inline>");
    g_print("Rules: %d\\n", g_list_length(sheet->rules));
    g_print("Variables: %d\\n", g_hash_table_size(sheet->variables));
    g_print("\\n");
    
    for (GList *l = sheet->rules; l; l = l->next) {
        CssRule *rule = l->data;
        
        g_print("Rule (specificity: %d, line %d):\\n", 
               rule->specificity, rule->line_number);
        
        /* Print selectors */
        g_print("  Selectors: ");
        for (GList *sl = rule->selectors; sl; sl = sl->next) {
            CssSelector *sel = sl->data;
            if (sel->name) {
                g_print("%s ", sel->name);
            } else {
                g_print("* ");
            }
        }
        g_print("\\n");
        
        /* Print declarations */
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, rule->declarations);
        
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            CssDeclaration *decl = value;
            gchar *val_str = css_value_to_string(decl->value);
            g_print("  %s: %s%s\\n", 
                   (gchar *)key, 
                   val_str ? val_str : "<null>",
                   decl->important ? " !important" : "");
            g_free(val_str);
        }
        
        g_print("\\n");
    }
}
