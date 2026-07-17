/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   css_theme.h - CSS-Based Theme Engine for Openbox
   Copyright (c) 2026  Openbox Development Team

   CSS3 subset parser and theme engine supporting variables, cascade,
   and Openbox-specific properties.
*/

#ifndef __css_theme_h
#define __css_theme_h

#include <glib.h>
#include "render.h"

G_BEGIN_DECLS

/* CSS Value Types */
typedef enum {
    CSS_VALUE_NONE = 0,
    CSS_VALUE_NUMBER,       /* 10, 10.5 */
    CSS_VALUE_PERCENTAGE,   /* 50% */
    CSS_VALUE_LENGTH,       /* 10px, 1em */
    CSS_VALUE_COLOR,        /* #fff, rgb(), var() */
    CSS_VALUE_STRING,       /* "string" */
    CSS_VALUE_IDENT,        /* keyword */
    CSS_VALUE_URL,          /* url(...) */
    CSS_VALUE_LIST,         /* space-separated list */
} CssValueType;

/* CSS Unit Types */
typedef enum {
    CSS_UNIT_NONE = 0,
    CSS_UNIT_PX,
    CSS_UNIT_PT,
    CSS_UNIT_EM,
    CSS_UNIT_REM,
    CSS_UNIT_PERCENT,
} CssUnit;

/* CSS Value */
typedef struct _CssValue {
    CssValueType type;
    CssUnit unit;
    union {
        gdouble number;
        gchar *string;
        struct {
            guint8 r, g, b, a;
        } color;
        GList *list;  /* For CSS_VALUE_LIST */
    } data;
} CssValue;

/* CSS Declaration (property: value) */
typedef struct _CssDeclaration {
    gchar *property;
    CssValue *value;
    gboolean important;
} CssDeclaration;

/* CSS Selector Types */
typedef enum {
    CSS_SELECTOR_UNIVERSAL,     /* * */
    CSS_SELECTOR_TYPE,          /* element */
    CSS_SELECTOR_CLASS,           /* .class */
    CSS_SELECTOR_ID,              /* #id */
    CSS_SELECTOR_PSEUDO_CLASS,    /* :hover, :focused */
    CSS_SELECTOR_PSEUDO_ELEMENT,  /* ::before */
    CSS_SELECTOR_DESCENDANT,      /* ancestor descendant */
    CSS_SELECTOR_CHILD,           /* parent > child */
    CSS_SELECTOR_ADJACENT,        /* prev + next */
    CSS_SELECTOR_SIBLING,         /* prev ~ next */
} CssSelectorType;

/* CSS Selector Component */
typedef struct _CssSelector {
    CssSelectorType type;
    gchar *name;           /* element name, class name, id, etc. */
    struct _CssSelector *next;  /* For compound selectors */
    struct _CssSelector *child; /* For combinator chains */
} CssSelector;

/* CSS Rule (selector { declarations }) */
typedef struct _CssRule {
    CssSelector *selectors;     /* List of selectors (comma-separated) */
    GHashTable *declarations;   /* property -> CssDeclaration* */
    gint specificity;           /* For cascade ordering */
    gchar *source_file;
    gint line_number;
} CssRule;

/* CSS Stylesheet */
typedef struct _CssStylesheet {
    GList *rules;               /* List of CssRule* */
    GHashTable *variables;      /* CSS custom properties --* */
    gchar *source_path;
} CssStylesheet;

/* Openbox CSS Theme */
typedef struct _ObCssTheme {
    CssStylesheet *stylesheet;
    gchar *name;
    gchar *path;
    
    /* Cached computed values for performance */
    GHashTable *computed_cache;
    
    /* Dark mode support */
    gboolean prefers_dark;
    gboolean supports_dark_mode;
} ObCssTheme;

/* CSS Parser */
CssStylesheet* css_stylesheet_new(const gchar *path);
void css_stylesheet_free(CssStylesheet *sheet);
gboolean css_stylesheet_load(CssStylesheet *sheet, const gchar *css_data, GError **error);
gboolean css_stylesheet_load_file(CssStylesheet *sheet, const gchar *path, GError **error);

/* CSS Query */
CssValue* css_stylesheet_get_property(CssStylesheet *sheet, 
                                       const gchar *element,
                                       const gchar *pseudo_class,
                                       const gchar *property);
CssValue* css_stylesheet_get_property_with_state(CssStylesheet *sheet,
                                                  const gchar *element,
                                                  const gchar *state,
                                                  const gchar *property);

/* CSS Variables */
CssValue* css_variable_get(CssStylesheet *sheet, const gchar *name);
void css_variable_set(CssStylesheet *sheet, const gchar *name, CssValue *value);

/* Value Conversion */
gboolean css_value_to_color(CssValue *val, RrColor **color);
gboolean css_value_to_length(CssValue *val, gint *pixels, gint base_size);
gboolean css_value_to_number(CssValue *val, gdouble *number);
gchar* css_value_to_string(CssValue *val);

/* Openbox Theme Integration */
RrTheme* ob_css_theme_load(const RrInstance *inst, const gchar *name,
                            gboolean allow_fallback, GError **error);
gboolean ob_css_theme_apply(RrTheme *theme, ObCssTheme *css_theme);

/* Theme Conversion */
gboolean ob_theme_convert_themerc_to_css(const gchar *themerc_path,
                                          const gchar *css_output_path,
                                          GError **error);

/* Dark Mode */
void ob_css_theme_set_dark_mode(ObCssTheme *theme, gboolean dark);
gboolean ob_css_theme_has_dark_mode(ObCssTheme *theme);

/* CSS Property Definitions for Openbox */
#define CSS_PROP_WINDOW_BG              "window-background"
#define CSS_PROP_WINDOW_BORDER_WIDTH    "window-border-width"
#define CSS_PROP_WINDOW_BORDER_COLOR    "window-border-color"
#define CSS_PROP_WINDOW_BORDER_RADIUS   "window-border-radius"
#define CSS_PROP_WINDOW_SHADOW          "window-shadow"

#define CSS_PROP_TITLEBAR_BG            "titlebar-background"
#define CSS_PROP_TITLEBAR_HEIGHT        "titlebar-height"
#define CSS_PROP_TITLEBAR_PADDING       "titlebar-padding"
#define CSS_PROP_TITLE_ALIGN            "title-text-align"

#define CSS_PROP_TITLE_COLOR            "title-color"
#define CSS_PROP_TITLE_COLOR_UNFOCUSED  "title-color-unfocused"
#define CSS_PROP_TITLE_FONT             "title-font"
#define CSS_PROP_TITLE_FONT_SIZE        "title-font-size"
#define CSS_PROP_TITLE_FONT_WEIGHT      "title-font-weight"
#define CSS_PROP_TITLE_SHADOW           "title-text-shadow"

#define CSS_PROP_BUTTON_SIZE            "button-size"
#define CSS_PROP_BUTTON_MARGIN          "button-margin"
#define CSS_PROP_BUTTON_BG              "button-background"
#define CSS_PROP_BUTTON_BG_HOVER        "button-background-hover"
#define CSS_PROP_BUTTON_BG_ACTIVE       "button-background-active"
#define CSS_PROP_BUTTON_ICON_COLOR      "button-icon-color"

#define CSS_PROP_GRIP_SIZE              "grip-size"
#define CSS_PROP_GRIP_BG                "grip-background"
#define CSS_PROP_HANDLE_HEIGHT          "handle-height"
#define CSS_PROP_HANDLE_BG              "handle-background"

#define CSS_PROP_MENU_BG                "menu-background"
#define CSS_PROP_MENU_BORDER_WIDTH      "menu-border-width"
#define CSS_PROP_MENU_BORDER_COLOR      "menu-border-color"
#define CSS_PROP_MENU_BORDER_RADIUS     "menu-border-radius"
#define CSS_PROP_MENU_SHADOW            "menu-shadow"
#define CSS_PROP_MENU_PADDING           "menu-padding"

#define CSS_PROP_MENU_ITEM_HEIGHT       "menu-item-height"
#define CSS_PROP_MENU_ITEM_PADDING      "menu-item-padding"
#define CSS_PROP_MENU_ITEM_BG_HOVER     "menu-item-background-hover"
#define CSS_PROP_MENU_ITEM_COLOR        "menu-item-color"
#define CSS_PROP_MENU_ITEM_COLOR_HOVER  "menu-item-color-hover"

/* CSS Selectors for Openbox Elements */
#define CSS_SELECTOR_WINDOW             "window"
#define CSS_SELECTOR_WINDOW_FOCUSED     "window:focus"
#define CSS_SELECTOR_WINDOW_UNFOCUSED   "window:unfocused"
#define CSS_SELECTOR_WINDOW_MAXIMIZED   "window:maximized"
#define CSS_SELECTOR_WINDOW_SHADED      "window:shaded"

#define CSS_SELECTOR_TITLEBAR           "titlebar"
#define CSS_SELECTOR_TITLE              "title"

#define CSS_SELECTOR_BUTTON             "button"
#define CSS_SELECTOR_BUTTON_CLOSE       "button.close"
#define CSS_SELECTOR_BUTTON_MAXIMIZE    "button.maximize"
#define CSS_SELECTOR_BUTTON_ICONIFY     "button.iconify"
#define CSS_SELECTOR_BUTTON_SHADE       "button.shade"
#define CSS_SELECTOR_BUTTON_DESK        "button.desk"

#define CSS_SELECTOR_GRIP               "grip"
#define CSS_SELECTOR_HANDLE             "handle"

#define CSS_SELECTOR_MENU               "menu"
#define CSS_SELECTOR_MENU_TITLE         "menu-title"
#define CSS_SELECTOR_MENU_ITEM          "menu-item"
#define CSS_SELECTOR_MENU_ITEM_SELECTED "menu-item:hover"
#define CSS_SELECTOR_MENU_SEPARATOR     "menu-separator"

G_END_DECLS

#endif /* __css_theme_h */
