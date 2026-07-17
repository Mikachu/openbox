/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   css_theme.h - CSS-Based Theme Engine for Openbox
   Copyright (c) 2026  Openbox Development Team

   CSS3 subset parser and theme engine supporting variables, cascade,
   and Openbox-specific properties.
   
   PERFORMANCE OPTIMIZATIONS 4.0.1:
   - Added property value caching to avoid repeated lookups
   - Added computed style caching for window states
   - Optimized selector matching with early exit
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
        struct {
            gdouble value;
            gchar *unit_str;
        } length;
        GList *list;  /* For CSS_VALUE_LIST */
    } v;
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
    gchar *element;        /* NULL for universal */
    gchar *class_name;
    gchar *id;
    gchar *pseudo_class;
    gchar *pseudo_element;
    struct _CssSelector *next;  /* For combinators */
} CssSelector;

/* CSS Rule (selectors + declarations) */
typedef struct _CssRule {
    CssSelector **selectors;
    gsize n_selectors;
    CssDeclaration **declarations;
    gsize n_declarations;
    guint specificity;     /* Cached specificity */
} CssRule;

/* CSS Stylesheet */
typedef struct _CssStylesheet {
    CssRule **rules;
    gsize n_rules;
    GHashTable *variables;  /* Custom properties --* */
    
    /* PERFORMANCE: Cached property lookups */
    GHashTable *property_cache;  /* "element:state:property" -> CssValue* */
    guint cache_hits;
    guint cache_misses;
} CssStylesheet;

/* Computed Style (cached) */
typedef struct _CssComputedStyle {
    GHashTable *properties;  /* property name -> CssValue* */
    guint64 generation;      /* For cache invalidation */
    guint64 last_accessed;   /* For LRU eviction */
} CssComputedStyle;

/* Theme Cache Entry */
typedef struct _ThemeCacheEntry {
    gchar *key;            /* "element:state:property" */
    CssValue *value;
    guint64 generation;
} ThemeCacheEntry;

/* Function Prototypes */

/* Stylesheet Management */
CssStylesheet* css_stylesheet_new(void);
void css_stylesheet_free(CssStylesheet *sheet);
CssStylesheet* css_stylesheet_parse(const gchar *css, const gchar *file);

/* Rule Management */
void css_rule_free(CssRule *rule);
void css_selector_free(CssSelector *sel);
void css_declaration_free(CssDeclaration *decl);

/* Value Management */
CssValue* css_value_new(CssValueType type);
void css_value_free(CssValue *val);
gboolean css_value_to_color(CssValue *val, RrColor **color);
gboolean css_value_to_length(CssValue *val, gint *pixels, gint base);
gchar* css_value_to_string(CssValue *val);

/* Selector Matching */
gboolean css_selector_matches(CssSelector *sel, const gchar *element, 
                               const gchar *state);

/* Property Lookup with Caching */
CssValue* css_stylesheet_get_property(CssStylesheet *sheet,
                                       const gchar *element,
                                       const gchar *state,
                                       const gchar *property);

/* Cache Management */
void css_stylesheet_clear_cache(CssStylesheet *sheet);
void css_stylesheet_invalidate_cache(CssStylesheet *sheet, const gchar *element);
void css_stylesheet_get_cache_stats(CssStylesheet *sheet, 
                                     guint *hits, guint *misses);

/* Computed Style */
CssComputedStyle* css_computed_style_new(void);
void css_computed_style_free(CssComputedStyle *style);
CssValue* css_computed_style_get(CssComputedStyle *style, const gchar *property);
void css_computed_style_set(CssComputedStyle *style, const gchar *property, 
                             CssValue *value);

/* Variable Resolution */
CssValue* css_stylesheet_get_variable(CssStylesheet *sheet, const gchar *name);
void css_stylesheet_set_variable(CssStylesheet *sheet, const gchar *name,
                                  CssValue *value);

/* Theme Loading */
RrTheme* ob_css_theme_load(const RrInstance *inst, const gchar *name,
                            gboolean allow_fallback, GError **error);

/* Performance Monitoring */
void ob_css_theme_get_stats(guint *parsed_rules, guint *cache_size,
                             gdouble *cache_hit_rate);

G_END_DECLS

#endif /* __css_theme_h */
