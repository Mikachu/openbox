/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   css_parser.c - CSS3 Subset Parser for Openbox
   Copyright (c) 2026  Openbox Development Team

   Lightweight CSS parser supporting:
   - Selectors: *, element, .class, #id, :pseudo-class, combinators
   - Properties: color, length, number, string values
   - Variables: --* custom properties with var() function
   - Cascade: specificity calculation, !important
*/

#include "css_theme.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Parser state */
typedef struct {
    const gchar *input;
    gsize pos;
    gsize len;
    gint line;
    gint column;
    const gchar *file;
} CssParser;

/* Forward declarations */
static void parser_skip_whitespace(CssParser *p);
static void parser_skip_comments(CssParser *p);
static gboolean parser_match(CssParser *p, gchar c);
static gboolean parser_match_string(CssParser *p, const gchar *str);
static gchar parser_peek(CssParser *p);
static gchar parser_consume(CssParser *p);
static gchar* parser_read_ident(CssParser *p);
static gchar* parser_read_string(CssParser *p, gchar quote);
static CssValue* parser_read_value(CssParser *p);
static CssSelector* parser_read_selector(CssParser *p);
static gboolean parser_read_declaration(CssParser *p, CssDeclaration **decl);

/* ============================================================================
 * Parser Utilities
 * ============================================================================ */

static void parser_init(CssParser *p, const gchar *input, const gchar *file)
{
    p->input = input;
    p->pos = 0;
    p->len = strlen(input);
    p->line = 1;
    p->column = 1;
    p->file = file ? g_strdup(file) : NULL;
}

static void parser_free(CssParser *p)
{
    g_free((gpointer)p->file);
}

static void parser_advance(CssParser *p, gsize n)
{
    while (n-- > 0 && p->pos < p->len) {
        if (p->input[p->pos] == '\\n') {
            p->line++;
            p->column = 1;
        } else {
            p->column++;
        }
        p->pos++;
    }
}

static gchar parser_peek(CssParser *p)
{
    if (p->pos >= p->len) return '\\0';
    return p->input[p->pos];
}

static gchar parser_peek_n(CssParser *p, gsize n)
{
    if (p->pos + n >= p->len) return '\\0';
    return p->input[p->pos + n];
}

static gchar parser_consume(CssParser *p)
{
    gchar c = parser_peek(p);
    if (c != '\\0') parser_advance(p, 1);
    return c;
}

static void parser_skip_whitespace(CssParser *p)
{
    while (p->pos < p->len && g_ascii_isspace(parser_peek(p))) {
        parser_consume(p);
    }
}

static void parser_skip_comments(CssParser *p)
{
    parser_skip_whitespace(p);
    
    /* Skip C-style comments */
    while (parser_peek(p) == '/' && parser_peek_n(p, 1) == '*') {
        parser_advance(p, 2); /* Skip /* */
        while (p->pos < p->len) {
            if (parser_peek(p) == '*' && parser_peek_n(p, 1) == '/') {
                parser_advance(p, 2);
                break;
            }
            parser_consume(p);
        }
        parser_skip_whitespace(p);
    }
}

static gboolean parser_match(CssParser *p, gchar c)
{
    parser_skip_comments(p);
    if (parser_peek(p) == c) {
        parser_consume(p);
        return TRUE;
    }
    return FALSE;
}

static gboolean parser_expect(CssParser *p, gchar c, GError **error)
{
    if (!parser_match(p, c)) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   "%s:%d:%d: Expected '%c' but found '%c'",
                   p->file ? p->file : "<input>", p->line, p->column,
                   c, parser_peek(p));
        return FALSE;
    }
    return TRUE;
}

/* ============================================================================
 * Token Reading
 * ============================================================================ */

static gboolean is_ident_start(gchar c)
{
    return g_ascii_isalpha(c) || c == '_' || c == '-';
}

static gboolean is_ident_char(gchar c)
{
    return g_ascii_isalnum(c) || c == '_' || c == '-';
}

static gchar* parser_read_ident(CssParser *p)
{
    parser_skip_comments(p);
    
    gsize start = p->pos;
    if (p->pos >= p->len) return NULL;
    
    if (parser_peek(p) == '-') {
        /* Could be --variable or -ident */
        parser_advance(p, 1);
        if (parser_peek(p) == '-') {
            /* CSS custom property --* */
            parser_advance(p, 1);
        }
    } else if (!is_ident_start(parser_peek(p))) {
        return NULL;
    }
    
    while (p->pos < p->len && is_ident_char(parser_peek(p))) {
        parser_advance(p, 1);
    }
    
    if (p->pos == start) return NULL;
    
    return g_strndup(p->input + start, p->pos - start);
}

static gchar* parser_read_string(CssParser *p, gchar quote)
{
    parser_skip_comments(p);
    if (parser_peek(p) != quote) return NULL;
    
    parser_consume(p); /* Skip opening quote */
    GString *str = g_string_new(NULL);
    
    while (p->pos < p->len && parser_peek(p) != quote) {
        if (parser_peek(p) == '\\\\' && p->pos + 1 < p->len) {
            parser_advance(p, 1);
            gchar c = parser_consume(p);
            switch (c) {
                case 'n': g_string_append_c(str, '\\n'); break;
                case 't': g_string_append_c(str, '\\t'); break;
                case 'r': g_string_append_c(str, '\\r'); break;
                case '\\\\': g_string_append_c(str, '\\\\'); break;
                case '\"': g_string_append_c(str, '\"'); break;
                case '\\'': g_string_append_c(str, '\\''); break;
                default: g_string_append_c(str, c); break;
            }
        } else {
            g_string_append_c(str, parser_consume(p));
        }
    }
    
    if (parser_peek(p) == quote) {
        parser_consume(p); /* Skip closing quote */
    }
    
    return g_string_free(str, FALSE);
}

/* ============================================================================
 * Value Parsing
 * ============================================================================ */

static CssValue* css_value_new(CssValueType type)
{
    CssValue *v = g_slice_new0(CssValue);
    v->type = type;
    return v;
}

void css_value_free(CssValue *v)
{
    if (!v) return;
    
    if (v->type == CSS_VALUE_STRING && v->data.string) {
        g_free(v->data.string);
    } else if (v->type == CSS_VALUE_LIST && v->data.list) {
        g_list_free_full(v->data.list, (GDestroyNotify)css_value_free);
    }
    
    g_slice_free(CssValue, v);
}

static CssValue* parse_color(const gchar *str, gsize len)
{
    CssValue *v = css_value_new(CSS_VALUE_COLOR);
    
    if (str[0] == '#') {
        /* Hex color: #fff, #ffffff, #ffffffff */
        guint32 color = 0;
        if (len == 4) { /* #rgb */
            sscanf(str + 1, "%3x", &color);
            v->data.color.r = ((color >> 8) & 0xf) * 17;
            v->data.color.g = ((color >> 4) & 0xf) * 17;
            v->data.color.b = (color & 0xf) * 17;
            v->data.color.a = 255;
        } else if (len == 7) { /* #rrggbb */
            sscanf(str + 1, "%06x", &color);
            v->data.color.r = (color >> 16) & 0xff;
            v->data.color.g = (color >> 8) & 0xff;
            v->data.color.b = color & 0xff;
            v->data.color.a = 255;
        } else if (len == 9) { /* #rrggbbaa */
            sscanf(str + 1, "%08x", &color);
            v->data.color.r = (color >> 24) & 0xff;
            v->data.color.g = (color >> 16) & 0xff;
            v->data.color.b = (color >> 8) & 0xff;
            v->data.color.a = color & 0xff;
        }
    }
    /* TODO: rgb(), rgba(), hsl(), var() functions */
    
    return v;
}

static CssValue* parser_read_value(CssParser *p)
{
    parser_skip_comments(p);
    
    /* Try string */
    if (parser_peek(p) == '\"' || parser_peek(p) == '\\'') {
        gchar *str = parser_read_string(p, parser_peek(p));
        CssValue *v = css_value_new(CSS_VALUE_STRING);
        v->data.string = str;
        return v;
    }
    
    /* Try URL */
    if (parser_match_string(p, "url(")) {
        parser_skip_comments(p);
        gchar quote = parser_peek(p);
        gchar *url;
        if (quote == '\"' || quote == '\\'') {
            url = parser_read_string(p, quote);
        } else {
            /* Unquoted URL */
            gsize start = p->pos;
            while (p->pos < p->len && parser_peek(p) != ')') {
                parser_advance(p, 1);
            }
            url = g_strndup(p->input + start, p->pos - start);
            g_strstrip(url);
        }
        parser_skip_comments(p);
        parser_match(p, ')'); /* Consume ) if present */
        
        CssValue *v = css_value_new(CSS_VALUE_URL);
        v->data.string = url;
        return v;
    }
    
    /* Try color (hex) */
    if (parser_peek(p) == '#') {
        gsize start = p->pos;
        parser_advance(p, 1);
        while (p->pos < p->len && g_ascii_isxdigit(parser_peek(p))) {
            parser_advance(p, 1);
        }
        return parse_color(p->input + start, p->pos - start);
    }
    
    /* Try number/length/percentage */
    if (g_ascii_isdigit(parser_peek(p)) || parser_peek(p) == '.') {
        gsize start = p->pos;
        gboolean has_dot = FALSE;
        
        if (parser_peek(p) == '.') {
            has_dot = TRUE;
            parser_advance(p, 1);
        }
        
        while (p->pos < p->len && (g_ascii_isdigit(parser_peek(p)) || 
                                   (!has_dot && parser_peek(p) == '.'))) {
            if (parser_peek(p) == '.') has_dot = TRUE;
            parser_advance(p, 1);
        }
        
        gchar *num_str = g_strndup(p->input + start, p->pos - start);
        gdouble num = g_ascii_strtod(num_str, NULL);
        g_free(num_str);
        
        /* Check for unit */
        CssUnit unit = CSS_UNIT_NONE;
        if (parser_match_string(p, "px")) {
            unit = CSS_UNIT_PX;
        } else if (parser_match_string(p, "pt")) {
            unit = CSS_UNIT_PT;
        } else if (parser_match_string(p, "em")) {
            unit = CSS_UNIT_EM;
        } else if (parser_match_string(p, "%")) {
            unit = CSS_UNIT_PERCENT;
        }
        
        CssValue *v;
        if (unit == CSS_UNIT_PERCENT) {
            v = css_value_new(CSS_VALUE_PERCENTAGE);
        } else if (unit != CSS_UNIT_NONE) {
            v = css_value_new(CSS_VALUE_LENGTH);
            v->unit = unit;
        } else {
            v = css_value_new(CSS_VALUE_NUMBER);
        }
        v->data.number = num;
        return v;
    }
    
    /* Try identifier/keyword */
    gchar *ident = parser_read_ident(p);
    if (ident) {
        CssValue *v = css_value_new(CSS_VALUE_IDENT);
        v->data.string = ident;
        return v;
    }
    
    return NULL;
}

/* ============================================================================
 * Selector Parsing
 * ============================================================================ */

static CssSelector* css_selector_new(CssSelectorType type, const gchar *name)
{
    CssSelector *s = g_slice_new0(CssSelector);
    s->type = type;
    s->name = g_strdup(name);
    return s;
}

void css_selector_free(CssSelector *s)
{
    if (!s) return;
    g_free(s->name);
    if (s->next) css_selector_free(s->next);
    if (s->child) css_selector_free(s->child);
    g_slice_free(CssSelector, s);
}

static CssSelector* parser_read_simple_selector(CssParser *p)
{
    CssSelector *first = NULL;
    CssSelector *last = NULL;
    
    parser_skip_comments(p);
    
    /* Universal selector */
    if (parser_match(p, '*')) {
        first = css_selector_new(CSS_SELECTOR_UNIVERSAL, NULL);
        last = first;
    }
    
    /* Type or class/id/pseudo selectors */
    while (TRUE) {
        parser_skip_comments(p);
        
        if (parser_match(p, '.')) {
            /* Class selector */
            gchar *name = parser_read_ident(p);
            if (name) {
                CssSelector *s = css_selector_new(CSS_SELECTOR_CLASS, name);
                g_free(name);
                if (!first) first = s;
                else last->next = s;
                last = s;
            }
        } else if (parser_match(p, '#')) {
            /* ID selector */
            gchar *name = parser_read_ident(p);
            if (name) {
                CssSelector *s = css_selector_new(CSS_SELECTOR_ID, name);
                g_free(name);
                if (!first) first = s;
                else last->next = s;
                last = s;
            }
        } else if (parser_match(p, ':')) {
            /* Pseudo-class or pseudo-element */
            gboolean double_colon = parser_match(p, ':');
            gchar *name = parser_read_ident(p);
            if (name) {
                CssSelector *s = css_selector_new(
                    double_colon ? CSS_SELECTOR_PSEUDO_ELEMENT : CSS_SELECTOR_PSEUDO_CLASS,
                    name);
                g_free(name);
                if (!first) first = s;
                else last->next = s;
                last = s;
            }
        } else if (!first || parser_peek(p) == '[') {
            /* Type selector or attribute selector */
            gchar *name = parser_read_ident(p);
            if (name) {
                CssSelector *s = css_selector_new(CSS_SELECTOR_TYPE, name);
                g_free(name);
                if (!first) first = s;
                else last->next = s;
                last = s;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    return first;
}

static CssSelector* parser_read_selector(CssParser *p)
{
    CssSelector *first = parser_read_simple_selector(p);
    if (!first) return NULL;
    
    CssSelector *current = first;
    
    /* Read combinators and subsequent selectors */
    while (TRUE) {
        parser_skip_comments(p);
        
        /* Descendant combinator (space) */
        if (g_ascii_isspace(parser_peek(p))) {
            parser_skip_whitespace(p);
            CssSelector *next = parser_read_simple_selector(p);
            if (next) {
                CssSelector *comb = css_selector_new(CSS_SELECTOR_DESCENDANT, NULL);
                comb->child = next;
                current->child = comb;
                current = next;
                continue;
            }
        }
        
        /* Child combinator */
        if (parser_match(p, '>')) {
            parser_skip_whitespace(p);
            CssSelector *next = parser_read_simple_selector(p);
            if (next) {
                CssSelector *comb = css_selector_new(CSS_SELECTOR_CHILD, NULL);
                comb->child = next;
                current->child = comb;
                current = next;
                continue;
            }
        }
        
        /* Adjacent sibling */
        if (parser_match(p, '+')) {
            parser_skip_whitespace(p);
            CssSelector *next = parser_read_simple_selector(p);
            if (next) {
                CssSelector *comb = css_selector_new(CSS_SELECTOR_ADJACENT, NULL);
                comb->child = next;
                current->child = comb;
                current = next;
                continue;
            }
        }
        
        /* General sibling */
        if (parser_match(p, '~')) {
            parser_skip_whitespace(p);
            CssSelector *next = parser_read_simple_selector(p);
            if (next) {
                CssSelector *comb = css_selector_new(CSS_SELECTOR_SIBLING, NULL);
                comb->child = next;
                current->child = comb;
                current = next;
                continue;
            }
        }
        
        break;
    }
    
    return first;
}

/* ============================================================================
 * Declaration and Rule Parsing
 * ============================================================================ */

static gboolean parser_read_declaration(CssParser *p, CssDeclaration **decl_out)
{
    parser_skip_comments(p);
    
    /* Read property name */
    gchar *property = parser_read_ident(p);
    if (!property) return FALSE;
    
    parser_skip_comments(p);
    if (!parser_expect(p, ':', NULL)) {
        g_free(property);
        return FALSE;
    }
    
    parser_skip_comments(p);
    
    /* Read value */
    CssValue *value = parser_read_value(p);
    if (!value) {
        g_free(property);
        return FALSE;
    }
    
    /* Check for !important */
    parser_skip_comments(p);
    gboolean important = FALSE;
    if (parser_match(p, '!')) {
        parser_skip_whitespace(p);
        gchar *ident = parser_read_ident(p);
        if (ident && g_ascii_strcasecmp(ident, "important") == 0) {
            important = TRUE;
        }
        g_free(ident);
    }
    
    CssDeclaration *decl = g_slice_new0(CssDeclaration);
    decl->property = property;
    decl->value = value;
    decl->important = important;
    
    *decl_out = decl;
    return TRUE;
}

static gint calculate_specificity(CssSelector *selector)
{
    gint a = 0, b = 0, c = 0; /* ID, class/pseudo, type */
    
    CssSelector *s = selector;
    while (s) {
        switch (s->type) {
            case CSS_SELECTOR_ID:
                a++;
                break;
            case CSS_SELECTOR_CLASS:
            case CSS_SELECTOR_PSEUDO_CLASS:
                b++;
                break;
            case CSS_SELECTOR_TYPE:
                c++;
                break;
            default:
                break;
        }
        s = s->next;
    }
    
    /* Specificity: a*100 + b*10 + c */
    return a * 100 + b * 10 + c;
}

static gboolean parser_read_rule(CssParser *p, CssRule **rule_out)
{
    parser_skip_comments(p);
    
    /* Read selectors */
    GList *selectors = NULL;
    
    while (TRUE) {
        CssSelector *sel = parser_read_selector(p);
        if (!sel) {
            g_list_free_full(selectors, (GDestroyNotify)css_selector_free);
            return FALSE;
        }
        selectors = g_list_append(selectors, sel);
        
        parser_skip_comments(p);
        if (!parser_match(p, ',')) break;
        parser_skip_whitespace(p);
    }
    
    if (!parser_expect(p, '{', NULL)) {
        g_list_free_full(selectors, (GDestroyNotify)css_selector_free);
        return FALSE;
    }
    
    /* Read declarations */
    GHashTable *decls = g_hash_table_new_full(g_str_hash, g_str_equal,
                                               g_free,
                                               (GDestroyNotify)css_declaration_free);
    
    while (TRUE) {
        parser_skip_comments(p);
        if (parser_match(p, '}')) break;
        
        CssDeclaration *decl = NULL;
        if (!parser_read_declaration(p, &decl)) {
            /* Skip to next declaration or end of block */
            while (p->pos < p->len && parser_peek(p) != ';' && parser_peek(p) != '}') {
                parser_consume(p);
            }
            parser_match(p, ';');
            continue;
        }
        
        /* Store declaration */
        g_hash_table_insert(decls, g_strdup(decl->property), decl);
        
        parser_skip_comments(p);
        parser_match(p, ';'); /* Optional semicolon */
    }
    
    /* Create rule */
    CssRule *rule = g_slice_new0(CssRule);
    rule->selectors = selectors;
    rule->declarations = decls;
    rule->specificity = selectors ? calculate_specificity(selectors->data) : 0;
    rule->source_file = g_strdup(p->file);
    rule->line_number = p->line;
    
    *rule_out = rule;
    return TRUE;
}

/* ============================================================================
 * Stylesheet API
 * ============================================================================ */

CssStylesheet* css_stylesheet_new(const gchar *path)
{
    CssStylesheet *sheet = g_slice_new0(CssStylesheet);
    sheet->rules = NULL;
    sheet->variables = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              g_free,
                                              (GDestroyNotify)css_value_free);
    sheet->source_path = g_strdup(path);
    return sheet;
}

void css_stylesheet_free(CssStylesheet *sheet)
{
    if (!sheet) return;
    
    g_list_free_full(sheet->rules, (GDestroyNotify)css_rule_free);
    g_hash_table_destroy(sheet->variables);
    g_free(sheet->source_path);
    
    g_slice_free(CssStylesheet, sheet);
}

void css_rule_free(CssRule *rule)
{
    if (!rule) return;
    
    g_list_free_full(rule->selectors, (GDestroyNotify)css_selector_free);
    g_hash_table_destroy(rule->declarations);
    g_free(rule->source_file);
    
    g_slice_free(CssRule, rule);
}

void css_declaration_free(CssDeclaration *decl)
{
    if (!decl) return;
    
    g_free(decl->property);
    css_value_free(decl->value);
    
    g_slice_free(CssDeclaration, decl);
}

gboolean css_stylesheet_load(CssStylesheet *sheet, const gchar *css_data, 
                              GError **error)
{
    CssParser p;
    parser_init(&p, css_data, sheet->source_path);
    
    while (p.pos < p.len) {
        parser_skip_comments(&p);
        if (p.pos >= p.len) break;
        
        /* Try to parse @-rules first */
        if (parser_match(&p, '@')) {
            gchar *at_rule = parser_read_ident(&p);
            if (at_rule) {
                if (g_ascii_strcasecmp(at_rule, "import") == 0) {
                    /* @import - skip for now */
                    while (p.pos < p.len && parser_peek(&p) != ';') {
                        parser_consume(&p);
                    }
                    parser_match(&p, ';');
                } else if (g_ascii_strcasecmp(at_rule, "media") == 0) {
                    /* @media - skip for now */
                    gint depth = 0;
                    while (p.pos < p.len) {
                        if (parser_peek(&p) == '{') depth++;
                        else if (parser_peek(&p) == '}') {
                            if (--depth < 0) break;
                        }
                        parser_consume(&p);
                    }
                    parser_match(&p, '}');
                }
                g_free(at_rule);
            }
            continue;
        }
        
        /* Parse regular rule */
        CssRule *rule = NULL;
        if (parser_read_rule(&p, &rule)) {
            sheet->rules = g_list_append(sheet->rules, rule);
        } else {
            /* Skip invalid rule */
            while (p.pos < p.len && parser_peek(&p) != '}') {
                parser_consume(&p);
            }
            parser_match(&p, '}');
        }
    }
    
    parser_free(&p);
    return TRUE;
}

gboolean css_stylesheet_load_file(CssStylesheet *sheet, const gchar *path,
                                   GError **error)
{
    gchar *contents = NULL;
    gsize length;
    
    if (!g_file_get_contents(path, &contents, &length, error)) {
        return FALSE;
    }
    
    gboolean result = css_stylesheet_load(sheet, contents, error);
    g_free(contents);
    
    return result;
}

/* ============================================================================
 * Property Lookup
 * ============================================================================ */

static gboolean selector_matches(CssSelector *selector, 
                                  const gchar *element,
                                  const gchar *class_name,
                                  const gchar *pseudo_class)
{
    /* Simple matching - can be expanded */
    CssSelector *s = selector;
    
    while (s) {
        gboolean matched = FALSE;
        
        switch (s->type) {
            case CSS_SELECTOR_UNIVERSAL:
                matched = TRUE;
                break;
            case CSS_SELECTOR_TYPE:
                if (s->name && g_ascii_strcasecmp(s->name, element) == 0) {
                    matched = TRUE;
                }
                break;
            case CSS_SELECTOR_CLASS:
                if (class_name && s->name && 
                    g_ascii_strcasecmp(s->name, class_name) == 0) {
                    matched = TRUE;
                }
                break;
            case CSS_SELECTOR_PSEUDO_CLASS:
                if (pseudo_class && s->name &&
                    g_ascii_strcasecmp(s->name, pseudo_class) == 0) {
                    matched = TRUE;
                }
                break;
            default:
                break;
        }
        
        if (!matched && s->type != CSS_SELECTOR_DESCENDANT &&
            s->type != CSS_SELECTOR_CHILD) {
            return FALSE;
        }
        
        s = s->next;
    }
    
    return TRUE;
}

CssValue* css_stylesheet_get_property(CssStylesheet *sheet,
                                       const gchar *element,
                                       const gchar *pseudo_class,
                                       const gchar *property)
{
    if (!sheet || !element || !property) return NULL;
    
    CssValue *best_value = NULL;
    gint best_specificity = -1;
    gboolean best_important = FALSE;
    
    /* Iterate through rules */
    for (GList *l = sheet->rules; l; l = l->next) {
        CssRule *rule = l->data;
        
        /* Check if any selector matches */
        for (GList *sl = rule->selectors; sl; sl = sl->next) {
            CssSelector *sel = sl->data;
            
            if (selector_matches(sel, element, NULL, pseudo_class)) {
                CssDeclaration *decl = g_hash_table_lookup(rule->declarations, 
                                                           property);
                if (decl) {
                    /* Cascade: higher specificity wins, !important overrides */
                    gboolean use_this = FALSE;
                    
                    if (decl->important && !best_important) {
                        use_this = TRUE;
                    } else if (decl->important == best_important) {
                        if (rule->specificity > best_specificity) {
                            use_this = TRUE;
                        }
                    }
                    
                    if (use_this) {
                        best_value = decl->value;
                        best_specificity = rule->specificity;
                        best_important = decl->important;
                    }
                }
            }
        }
    }
    
    return best_value;
}

CssValue* css_variable_get(CssStylesheet *sheet, const gchar *name)
{
    if (!sheet || !name) return NULL;
    return g_hash_table_lookup(sheet->variables, name);
}

void css_variable_set(CssStylesheet *sheet, const gchar *name, CssValue *value)
{
    if (!sheet || !name || !value) return;
    
    CssValue *copy = css_value_copy(value);
    g_hash_table_insert(sheet->variables, g_strdup(name), copy);
}

/* ============================================================================
 * Value Conversion
 * ============================================================================ */

CssValue* css_value_copy(CssValue *val)
{
    if (!val) return NULL;
    
    CssValue *copy = css_value_new(val->type);
    copy->unit = val->unit;
    
    switch (val->type) {
        case CSS_VALUE_NUMBER:
        case CSS_VALUE_PERCENTAGE:
        case CSS_VALUE_LENGTH:
            copy->data.number = val->data.number;
            break;
        case CSS_VALUE_STRING:
        case CSS_VALUE_IDENT:
        case CSS_VALUE_URL:
            copy->data.string = g_strdup(val->data.string);
            break;
        case CSS_VALUE_COLOR:
            copy->data.color = val->data.color;
            break;
        case CSS_VALUE_LIST:
            copy->data.list = NULL;
            for (GList *l = val->data.list; l; l = l->next) {
                copy->data.list = g_list_append(copy->data.list,
                                                css_value_copy(l->data));
            }
            break;
        default:
            break;
    }
    
    return copy;
}

gboolean css_value_to_color(CssValue *val, RrColor **color)
{
    if (!val || val->type != CSS_VALUE_COLOR) return FALSE;
    
    *color = RrColorNew(val->data.color.r, 
                        val->data.color.g,
                        val->data.color.b);
    /* TODO: Handle alpha */
    
    return TRUE;
}

gboolean css_value_to_length(CssValue *val, gint *pixels, gint base_size)
{
    if (!val) return FALSE;
    
    gdouble num = 0;
    
    switch (val->type) {
        case CSS_VALUE_NUMBER:
        case CSS_VALUE_LENGTH:
            num = val->data.number;
            break;
        case CSS_VALUE_PERCENTAGE:
            num = val->data.number * base_size / 100.0;
            break;
        default:
            return FALSE;
    }
    
    /* Convert to pixels based on unit */
    switch (val->unit) {
        case CSS_UNIT_PX:
        case CSS_UNIT_NONE:
            *pixels = (gint)num;
            break;
        case CSS_UNIT_PT:
            *pixels = (gint)(num * 96.0 / 72.0); /* 96 DPI */
            break;
        case CSS_UNIT_EM:
        case CSS_UNIT_REM:
            *pixels = (gint)(num * base_size);
            break;
        case CSS_UNIT_PERCENT:
            *pixels = (gint)(num * base_size / 100.0);
            break;
        default:
            *pixels = (gint)num;
    }
    
    return TRUE;
}

gboolean css_value_to_number(CssValue *val, gdouble *number)
{
    if (!val) return FALSE;
    
    switch (val->type) {
        case CSS_VALUE_NUMBER:
        case CSS_VALUE_LENGTH:
        case CSS_VALUE_PERCENTAGE:
            *number = val->data.number;
            return TRUE;
        default:
            return FALSE;
    }
}

gchar* css_value_to_string(CssValue *val)
{
    if (!val) return NULL;
    
    switch (val->type) {
        case CSS_VALUE_STRING:
        case CSS_VALUE_IDENT:
        case CSS_VALUE_URL:
            return g_strdup(val->data.string);
        case CSS_VALUE_NUMBER:
            return g_strdup_printf("%g", val->data.number);
        case CSS_VALUE_LENGTH:
            return g_strdup_printf("%g%s", val->data.number,
                                  val->unit == CSS_UNIT_PX ? "px" :
                                  val->unit == CSS_UNIT_PT ? "pt" :
                                  val->unit == CSS_UNIT_EM ? "em" : "");
        case CSS_VALUE_PERCENTAGE:
            return g_strdup_printf("%g%%", val->data.number);
        case CSS_VALUE_COLOR:
            return g_strdup_printf("#%02x%02x%02x",
                                  val->data.color.r,
                                  val->data.color.g,
                                  val->data.color.b);
        default:
            return NULL;
    }
}
