/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   xml2json.c - Openbox Configuration Migration Tool
   Copyright (c) 2026  Openbox Development Team

   Converts rc.xml and menu.xml to JSON format with validation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <json-glib/json-glib.h>

#define PROGRAM_NAME "openbox-xml2json"
#define PROGRAM_VERSION "4.0.0"

typedef struct {
    gboolean verbose;
    gboolean validate;
    gboolean backup;
    gboolean dry_run;
    gchar *output_dir;
    gchar *input_file;
} Options;

static Options opts = {
    .verbose = FALSE,
    .validate = TRUE,
    .backup = TRUE,
    .dry_run = FALSE,
    .output_dir = NULL,
    .input_file = NULL
};

static void print_usage(const gchar *program)
{
    g_print("Usage: %s [OPTIONS] <input.xml>\\n", program);
    g_print("\\n");
    g_print("Convert Openbox XML configuration to JSON format.\\n");
    g_print("\\n");
    g_print("Options:\\n");
    g_print("  -o, --output DIR      Output directory (default: same as input)\\n");
    g_print("  -n, --dry-run         Show what would be done without writing files\\n");
    g_print("  -v, --verbose         Verbose output\\n");
    g_print("  --no-validate         Skip JSON Schema validation\\n");
    g_print("  --no-backup           Don't create .xml.backup files\\n");
    g_print("  -h, --help            Show this help\\n");
    g_print("  --version             Show version\\n");
    g_print("\\n");
    g_print("Examples:\\n");
    g_print("  %s ~/.config/openbox/rc.xml\\n", program);
    g_print("  %s -o ./converted ~/.config/openbox/rc.xml\\n", program);
    g_print("  %s --dry-run -v ~/.config/openbox/menu.xml\\n", program);
}

static gboolean parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            g_print("%s version %s\\n", PROGRAM_NAME, PROGRAM_VERSION);
            exit(0);
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            opts.verbose = TRUE;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--dry-run") == 0) {
            opts.dry_run = TRUE;
        } else if (strcmp(argv[i], "--no-validate") == 0) {
            opts.validate = FALSE;
        } else if (strcmp(argv[i], "--no-backup") == 0) {
            opts.backup = FALSE;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (++i >= argc) {
                g_printerr("Error: --output requires an argument\\n");
                return FALSE;
            }
            opts.output_dir = argv[i];
        } else if (argv[i][0] == '-') {
            g_printerr("Error: Unknown option %s\\n", argv[i]);
            return FALSE;
        } else {
            if (opts.input_file) {
                g_printerr("Error: Multiple input files not supported\\n");
                return FALSE;
            }
            opts.input_file = argv[i];
        }
    }
    
    if (!opts.input_file) {
        g_printerr("Error: No input file specified\\n");
        return FALSE;
    }
    
    return TRUE;
}

/* ============================================================================
 * XML Parsing Helpers
 * ============================================================================ */

static gchar *get_prop(xmlNodePtr node, const gchar *name)
{
    return (gchar *)xmlGetProp(node, (const xmlChar *)name);
}

static gchar *get_content(xmlNodePtr node)
{
    return (gchar *)xmlNodeGetContent(node);
}

static xmlNodePtr find_child(xmlNodePtr parent, const gchar *name)
{
    for (xmlNodePtr child = parent->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE &&
            xmlStrcmp(child->name, (const xmlChar *)name) == 0) {
            return child;
        }
    }
    return NULL;
}

static gboolean parse_boolean(const gchar *str, gboolean default_val)
{
    if (!str) return default_val;
    if (strcmp(str, "yes") == 0 || strcmp(str, "true") == 0) return TRUE;
    if (strcmp(str, "no") == 0 || strcmp(str, "false") == 0) return FALSE;
    return default_val;
}

static gint parse_int(const gchar *str, gint default_val)
{
    if (!str) return default_val;
    gchar *end;
    gint val = strtol(str, &end, 10);
    if (*end != '\\0') return default_val;
    return val;
}

/* ============================================================================
 * RC.XML Conversion
 * ============================================================================ */

static void convert_resistance(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "resistance");
    json_builder_begin_object(builder);
    
    xmlNodePtr strength = find_child(node, "strength");
    if (strength) {
        gchar *content = get_content(strength);
        json_builder_set_member_name(builder, "strength");
        json_builder_add_int_value(builder, parse_int(content, 10));
        xmlFree(content);
    }
    
    xmlNodePtr screen_edge = find_child(node, "screen_edge_strength");
    if (screen_edge) {
        gchar *content = get_content(screen_edge);
        json_builder_set_member_name(builder, "screen_edge_strength");
        json_builder_add_int_value(builder, parse_int(content, 20));
        xmlFree(content);
    }
    
    json_builder_end_object(builder);
}

static void convert_focus(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "focus");
    json_builder_begin_object(builder);
    
    xmlNodePtr focus_new = find_child(node, "focusNew");
    if (focus_new) {
        gchar *content = get_content(focus_new);
        json_builder_set_member_name(builder, "focus_new");
        json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
        xmlFree(content);
    }
    
    xmlNodePtr follow_mouse = find_child(node, "followMouse");
    if (follow_mouse) {
        gchar *content = get_content(follow_mouse);
        json_builder_set_member_name(builder, "follow_mouse");
        json_builder_add_boolean_value(builder, parse_boolean(content, FALSE));
        xmlFree(content);
    }
    
    xmlNodePtr focus_last = find_child(node, "focusLast");
    if (focus_last) {
        gchar *content = get_content(focus_last);
        json_builder_set_member_name(builder, "focus_last");
        json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
        xmlFree(content);
    }
    
    xmlNodePtr under_mouse = find_child(node, "underMouse");
    if (under_mouse) {
        gchar *content = get_content(under_mouse);
        json_builder_set_member_name(builder, "under_mouse");
        json_builder_add_boolean_value(builder, parse_boolean(content, FALSE));
        xmlFree(content);
    }
    
    xmlNodePtr focus_delay = find_child(node, "focusDelay");
    if (focus_delay) {
        gchar *content = get_content(focus_delay);
        json_builder_set_member_name(builder, "focus_delay");
        json_builder_add_int_value(builder, parse_int(content, 200));
        xmlFree(content);
    }
    
    xmlNodePtr raise_on_focus = find_child(node, "raiseOnFocus");
    if (raise_on_focus) {
        gchar *content = get_content(raise_on_focus);
        json_builder_set_member_name(builder, "raise_on_focus");
        json_builder_add_boolean_value(builder, parse_boolean(content, FALSE));
        xmlFree(content);
    }
    
    json_builder_end_object(builder);
}

static void convert_theme(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "theme");
    json_builder_begin_object(builder);
    
    xmlNodePtr name = find_child(node, "name");
    if (name) {
        gchar *content = get_content(name);
        json_builder_set_member_name(builder, "name");
        json_builder_add_string_value(builder, content ? content : "Clearlooks");
        xmlFree(content);
    }
    
    xmlNodePtr title_layout = find_child(node, "titleLayout");
    if (title_layout) {
        gchar *content = get_content(title_layout);
        json_builder_set_member_name(builder, "title_layout");
        json_builder_add_string_value(builder, content ? content : "NLIMC");
        xmlFree(content);
    }
    
    xmlNodePtr keep_border = find_child(node, "keepBorder");
    if (keep_border) {
        gchar *content = get_content(keep_border);
        json_builder_set_member_name(builder, "keep_border");
        json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
        xmlFree(content);
    }
    
    xmlNodePtr animate = find_child(node, "animateIconify");
    if (animate) {
        gchar *content = get_content(animate);
        json_builder_set_member_name(builder, "animate_iconify");
        json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
        xmlFree(content);
    }
    
    /* Fonts */
    json_builder_set_member_name(builder, "fonts");
    json_builder_begin_object(builder);
    
    const gchar *font_types[] = {
        "activeWindow", "inactiveWindow", "menuHeader", 
        "menuItem", "activeOnScreenDisplay", "inactiveOnScreenDisplay", NULL
    };
    const gchar *font_keys[] = {
        "active_window", "inactive_window", "menu_header",
        "menu_item", "active_osd", "inactive_osd", NULL
    };
    
    for (int i = 0; font_types[i]; i++) {
        xmlNodePtr font = find_child(node, font_types[i]);
        if (font) {
            json_builder_set_member_name(builder, font_keys[i]);
            json_builder_begin_object(builder);
            
            xmlNodePtr font_node = find_child(font, "font");
            if (font_node) {
                gchar *family = get_prop(font_node, "name");
                gchar *size = get_prop(font_node, "size");
                gchar *weight = get_prop(font_node, "weight");
                gchar *slant = get_prop(font_node, "slant");
                
                if (family) {
                    json_builder_set_member_name(builder, "family");
                    json_builder_add_string_value(builder, family);
                    xmlFree(family);
                }
                if (size) {
                    json_builder_set_member_name(builder, "size");
                    json_builder_add_int_value(builder, parse_int(size, 9));
                    xmlFree(size);
                }
                if (weight) {
                    json_builder_set_member_name(builder, "weight");
                    json_builder_add_string_value(builder, weight);
                    xmlFree(weight);
                }
                if (slant) {
                    json_builder_set_member_name(builder, "slant");
                    json_builder_add_string_value(builder, slant);
                    xmlFree(slant);
                }
            }
            
            json_builder_end_object(builder);
        }
    }
    
    json_builder_end_object(builder); /* fonts */
    json_builder_end_object(builder); /* theme */
}

static void convert_desktops(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "desktops");
    json_builder_begin_object(builder);
    
    xmlNodePtr number = find_child(node, "number");
    if (number) {
        gchar *content = get_content(number);
        json_builder_set_member_name(builder, "count");
        json_builder_add_int_value(builder, parse_int(content, 4));
        xmlFree(content);
    }
    
    xmlNodePtr first = find_child(node, "firstdesk");
    if (first) {
        gchar *content = get_content(first);
        json_builder_set_member_name(builder, "first");
        json_builder_add_int_value(builder, parse_int(content, 1));
        xmlFree(content);
    }
    
    xmlNodePtr names = find_child(node, "names");
    if (names) {
        json_builder_set_member_name(builder, "names");
        json_builder_begin_array(builder);
        
        for (xmlNodePtr name = names->children; name; name = name->next) {
            if (name->type == XML_ELEMENT_NODE &&
                xmlStrcmp(name->name, (const xmlChar *)"name") == 0) {
                gchar *content = get_content(name);
                json_builder_add_string_value(builder, content);
                xmlFree(content);
            }
        }
        
        json_builder_end_array(builder);
    }
    
    xmlNodePtr popup_time = find_child(node, "popupTime");
    if (popup_time) {
        gchar *content = get_content(popup_time);
        json_builder_set_member_name(builder, "popup_time");
        json_builder_add_int_value(builder, parse_int(content, 875));
        xmlFree(content);
    }
    
    json_builder_end_object(builder);
}

static void convert_resize(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "resize");
    json_builder_begin_object(builder);
    
    xmlNodePtr draw_contents = find_child(node, "drawContents");
    if (draw_contents) {
        gchar *content = get_content(draw_contents);
        json_builder_set_member_name(builder, "draw_contents");
        json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
        xmlFree(content);
    }
    
    xmlNodePtr popup_show = find_child(node, "popupShow");
    if (popup_show) {
        gchar *content = get_content(popup_show);
        json_builder_set_member_name(builder, "popup_show");
        json_builder_add_string_value(builder, content ? content : "nonpixel");
        xmlFree(content);
    }
    
    xmlNodePtr popup_pos = find_child(node, "popupPosition");
    if (popup_pos) {
        gchar *content = get_content(popup_pos);
        json_builder_set_member_name(builder, "popup_position");
        json_builder_add_string_value(builder, content ? content : "center");
        xmlFree(content);
    }
    
    json_builder_end_object(builder);
}

static void convert_margins(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "margins");
    json_builder_begin_object(builder);
    
    const gchar *edges[] = {"top", "bottom", "left", "right", NULL};
    for (int i = 0; edges[i]; i++) {
        xmlNodePtr edge = find_child(node, edges[i]);
        if (edge) {
            gchar *content = get_content(edge);
            json_builder_set_member_name(builder, edges[i]);
            json_builder_add_int_value(builder, parse_int(content, 0));
            xmlFree(content);
        }
    }
    
    json_builder_end_object(builder);
}

static void convert_action(JsonBuilder *builder, xmlNodePtr node)
{
    gchar *name = get_prop(node, "name");
    if (!name) return;
    
    json_builder_set_member_name(builder, "name");
    json_builder_add_string_value(builder, name);
    
    /* Action-specific parameters */
    if (strcmp(name, "Execute") == 0) {
        xmlNodePtr cmd = find_child(node, "command");
        if (cmd) {
            gchar *content = get_content(cmd);
            json_builder_set_member_name(builder, "command");
            json_builder_add_string_value(builder, content);
            xmlFree(content);
        }
        
        xmlNodePtr startup = find_child(node, "startupnotify");
        if (startup) {
            json_builder_set_member_name(builder, "startup_notify");
            json_builder_begin_object(builder);
            
            xmlNodePtr enabled = find_child(startup, "enabled");
            if (enabled) {
                gchar *content = get_content(enabled);
                json_builder_set_member_name(builder, "enabled");
                json_builder_add_boolean_value(builder, parse_boolean(content, TRUE));
                xmlFree(content);
            }
            
            xmlNodePtr notify_name = find_child(startup, "name");
            if (notify_name) {
                gchar *content = get_content(notify_name);
                json_builder_set_member_name(builder, "name");
                json_builder_add_string_value(builder, content);
                xmlFree(content);
            }
            
            json_builder_end_object(builder);
        }
    } else if (strcmp(name, "ShowMenu") == 0) {
        gchar *menu = get_prop(node, "menu");
        if (menu) {
            json_builder_set_member_name(builder, "menu");
            json_builder_add_string_value(builder, menu);
            xmlFree(menu);
        }
    } else if (strcmp(name, "GoToDesktop") == 0 ||
               strcmp(name, "SendToDesktop") == 0) {
        gchar *to = get_prop(node, "to");
        if (to) {
            json_builder_set_member_name(builder, "to");
            /* Try to parse as integer first */
            gchar *end;
            gint val = strtol(to, &end, 10);
            if (*end == '\\0') {
                json_builder_add_int_value(builder, val);
            } else {
                json_builder_add_string_value(builder, to);
            }
            xmlFree(to);
        }
        
        gchar *wrap = get_prop(node, "wrap");
        if (wrap) {
            json_builder_set_member_name(builder, "wrap");
            json_builder_add_boolean_value(builder, parse_boolean(wrap, FALSE));
            xmlFree(wrap);
        }
    }
    
    xmlFree(name);
}

static void convert_keybinds(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "keyboard");
    json_builder_begin_object(builder);
    
    xmlNodePtr chain_quit = find_child(node, "chainQuitKey");
    if (chain_quit) {
        gchar *content = get_content(chain_quit);
        json_builder_set_member_name(builder, "chain_quit_key");
        json_builder_add_string_value(builder, content ? content : "C-g");
        xmlFree(content);
    }
    
    json_builder_set_member_name(builder, "keybinds");
    json_builder_begin_array(builder);
    
    for (xmlNodePtr keybind = node->children; keybind; keybind = keybind->next) {
        if (keybind->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(keybind->name, (const xmlChar *)"keybind") != 0) continue;
        
        json_builder_begin_object(builder);
        
        gchar *key = get_prop(keybind, "key");
        if (key) {
            json_builder_set_member_name(builder, "key");
            json_builder_add_string_value(builder, key);
            xmlFree(key);
        }
        
        /* Actions */
        xmlNodePtr action = find_child(keybind, "action");
        if (action) {
            xmlNodePtr next_action = find_child(keybind->next, "action");
            gboolean multi = (next_action != NULL);
            
            if (multi) {
                json_builder_set_member_name(builder, "actions");
                json_builder_begin_array(builder);
            } else {
                json_builder_set_member_name(builder, "action");
            }
            
            for (xmlNodePtr act = keybind->children; act; act = act->next) {
                if (act->type != XML_ELEMENT_NODE) continue;
                if (xmlStrcmp(act->name, (const xmlChar *)"action") != 0) continue;
                
                json_builder_begin_object(builder);
                convert_action(builder, act);
                json_builder_end_object(builder);
            }
            
            if (multi) {
                json_builder_end_array(builder);
            }
        }
        
        json_builder_end_object(builder);
    }
    
    json_builder_end_array(builder); /* keybinds */
    json_builder_end_object(builder); /* keyboard */
}

static void convert_mousebinds(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "mouse");
    json_builder_begin_object(builder);
    
    xmlNodePtr drag = find_child(node, "dragThreshold");
    if (drag) {
        gchar *content = get_content(drag);
        json_builder_set_member_name(builder, "drag_threshold");
        json_builder_add_int_value(builder, parse_int(content, 1));
        xmlFree(content);
    }
    
    xmlNodePtr dclick = find_child(node, "doubleClickTime");
    if (dclick) {
        gchar *content = get_content(dclick);
        json_builder_set_member_name(builder, "double_click_time");
        json_builder_add_int_value(builder, parse_int(content, 500));
        xmlFree(content);
    }
    
    json_builder_set_member_name(builder, "bindings");
    json_builder_begin_array(builder);
    
    for (xmlNodePtr ctx = node->children; ctx; ctx = ctx->next) {
        if (ctx->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(ctx->name, (const xmlChar *)"context") != 0) continue;
        
        gchar *ctx_name = get_prop(ctx, "name");
        if (!ctx_name) continue;
        
        for (xmlNodePtr mousebind = ctx->children; mousebind; mousebind = mousebind->next) {
            if (mousebind->type != XML_ELEMENT_NODE) continue;
            if (xmlStrcmp(mousebind->name, (const xmlChar *)"mousebind") != 0) continue;
            
            json_builder_begin_object(builder);
            
            json_builder_set_member_name(builder, "context");
            json_builder_add_string_value(builder, ctx_name);
            
            gchar *button = get_prop(mousebind, "button");
            if (button) {
                json_builder_set_member_name(builder, "button");
                json_builder_add_string_value(builder, button);
                xmlFree(button);
            }
            
            gchar *event = get_prop(mousebind, "action");
            if (event) {
                json_builder_set_member_name(builder, "event");
                json_builder_add_string_value(builder, event);
                xmlFree(event);
            }
            
            /* Actions */
            xmlNodePtr action = find_child(mousebind, "action");
            if (action) {
                json_builder_set_member_name(builder, "actions");
                json_builder_begin_array(builder);
                
                for (xmlNodePtr act = mousebind->children; act; act = act->next) {
                    if (act->type != XML_ELEMENT_NODE) continue;
                    if (xmlStrcmp(act->name, (const xmlChar *)"action") != 0) continue;
                    
                    json_builder_begin_object(builder);
                    convert_action(builder, act);
                    json_builder_end_object(builder);
                }
                
                json_builder_end_array(builder);
            }
            
            json_builder_end_object(builder);
        }
        
        xmlFree(ctx_name);
    }
    
    json_builder_end_array(builder); /* bindings */
    json_builder_end_object(builder); /* mouse */
}

static void convert_menu(JsonBuilder *builder, xmlNodePtr node)
{
    json_builder_set_member_name(builder, "menu");
    json_builder_begin_object(builder);
    
    xmlNodePtr file = find_child(node, "file");
    if (file) {
        gchar *content = get_content(file);
        /* Convert .xml to .json */
        gchar *json_name = g_strdup(content);
        gsize len = strlen(json_name);
        if (len > 4 && strcmp(json_name + len - 4, ".xml") == 0) {
            strcpy(json_name + len - 4, ".json");
        }
        json_builder_set_member_name(builder, "file");
        json_builder_add_string_value(builder, json_name);
        g_free(json_name);
        xmlFree(content);
    }
    
    xmlNodePtr hide_delay = find_child(node, "hideDelay");
    if (hide_delay) {
        gchar *content = get_content(hide_delay);
        json_builder_set_member_name(builder, "hide_delay");
        json_builder_add_int_value(builder, parse_int(content, 200));
        xmlFree(content);
    }
    
    xmlNodePtr middle = find_child(node, "middle");
    if (middle) {
        gchar *content = get_content(middle);
        json_builder_set_member_name(builder, "middle");
        json_builder_add_boolean_value(builder, parse_boolean(content, FALSE));
        xmlFree(content);
    }
    
    xmlNodePtr sub_show = find_child(node, "submenuShowDelay");
    if (sub_show) {
        gchar *content = get_content(sub_show);
        json_builder_set_member_name(builder, "submenu_show_delay");
        json_builder_add_int_value(builder, parse_int(content, 100));
        xmlFree(content);
    }
    
    xmlNodePtr sub_hide = find_child(node, "submenuHideDelay");
    if (sub_hide) {
        gchar *content = get_content(sub_hide);
        json_builder_set_member_name(builder, "submenu_hide_delay");
        json_builder_add_int_value(builder, parse_int(content, 400));
        xmlFree(content);
    }
    
    json_builder_end_object(builder);
}

static gchar *convert_rc_xml(const gchar *input_path)
{
    xmlDocPtr doc = xmlReadFile(input_path, NULL, 0);
    if (!doc) {
        g_printerr("Error: Failed to parse %s\\n", input_path);
        return NULL;
    }
    
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || xmlStrcmp(root->name, (const xmlChar *)"openbox_config") != 0) {
        g_printerr("Error: Invalid Openbox config file\\n");
        xmlFreeDoc(doc);
        return NULL;
    }
    
    /* Find theme section */
    xmlNodePtr theme = NULL;
    for (xmlNodePtr child = root->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE &&
            xmlStrcmp(child->name, (const xmlChar *)"theme") == 0) {
            theme = child;
            break;
        }
    }
    
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    
    json_builder_set_member_name(builder, "$schema");
    json_builder_add_string_value(builder, "https://openbox.org/schemas/rc-4.0.json");
    
    json_builder_set_member_name(builder, "version");
    json_builder_add_string_value(builder, "4.0.0");
    
    /* Convert each section */
    for (xmlNodePtr child = root->children; child; child = child->next) {
        if (child->type != XML_ELEMENT_NODE) continue;
        
        if (xmlStrcmp(child->name, (const xmlChar *)"resistance") == 0) {
            convert_resistance(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"focus") == 0) {
            convert_focus(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"theme") == 0) {
            convert_theme(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"desktops") == 0) {
            convert_desktops(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"resize") == 0) {
            convert_resize(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"margins") == 0) {
            convert_margins(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"keyboard") == 0) {
            convert_keybinds(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"mouse") == 0) {
            convert_mousebinds(builder, child);
        } else if (xmlStrcmp(child->name, (const xmlChar *)"menu") == 0) {
            convert_menu(builder, child);
        }
    }
    
    json_builder_end_object(builder);
    
    JsonGenerator *gen = json_generator_new();
    JsonNode *root_node = json_builder_get_root(builder);
    json_generator_set_root(gen, root_node);
    json_generator_set_pretty(gen, TRUE);
    json_generator_set_indent(gen, 2);
    
    gchar *output = json_generator_to_data(gen, NULL);
    
    json_node_free(root_node);
    g_object_unref(gen);
    g_object_unref(builder);
    xmlFreeDoc(doc);
    
    return output;
}

/* ============================================================================
 * Menu.XML Conversion
 * ============================================================================ */

static void convert_menu_item(JsonBuilder *builder, xmlNodePtr item)
{
    json_builder_begin_object(builder);
    
    gchar *type = (gchar *)item->name;
    json_builder_set_member_name(builder, "type");
    
    if (xmlStrcmp(item->name, (const xmlChar *)"separator") == 0) {
        json_builder_add_string_value(builder, "separator");
        
        xmlNodePtr label = find_child(item, "label");
        if (label) {
            gchar *content = get_content(label);
            json_builder_set_member_name(builder, "label");
            json_builder_add_string_value(builder, content);
            xmlFree(content);
        }
    } else if (xmlStrcmp(item->name, (const xmlChar *)"menu") == 0) {
        /* Menu reference */
        gchar *id = get_prop(item, "id");
        if (id) {
            json_builder_add_string_value(builder, "menu");
            json_builder_set_member_name(builder, "id");
            json_builder_add_string_value(builder, id);
            xmlFree(id);
        }
    } else if (xmlStrcmp(item->name, (const xmlChar *)"item") == 0) {
        json_builder_add_string_value(builder, "item");
        
        xmlNodePtr label = find_child(item, "label");
        if (label) {
            gchar *content = get_content(label);
            json_builder_set_member_name(builder, "label");
            json_builder_add_string_value(builder, content);
            xmlFree(content);
        }
        
        xmlNodePtr icon = find_child(item, "icon");
        if (icon) {
            gchar *content = get_content(icon);
            json_builder_set_member_name(builder, "icon");
            json_builder_add_string_value(builder, content);
            xmlFree(content);
        }
        
        xmlNodePtr action = find_child(item, "action");
        if (action) {
            json_builder_set_member_name(builder, "action");
            json_builder_begin_object(builder);
            convert_action(builder, action);
            json_builder_end_object(builder);
        }
    }
    
    json_builder_end_object(builder);
}

static gchar *convert_menu_xml(const gchar *input_path)
{
    xmlDocPtr doc = xmlReadFile(input_path, NULL, 0);
    if (!doc) {
        g_printerr("Error: Failed to parse %s\\n", input_path);
        return NULL;
    }
    
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || xmlStrcmp(root->name, (const xmlChar *)"openbox_menu") != 0) {
        g_printerr("Error: Invalid Openbox menu file\\n");
        xmlFreeDoc(doc);
        return NULL;
    }
    
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    
    json_builder_set_member_name(builder, "$schema");
    json_builder_add_string_value(builder, "https://openbox.org/schemas/menu-4.0.json");
    
    json_builder_set_member_name(builder, "version");
    json_builder_add_string_value(builder, "4.0.0");
    
    json_builder_set_member_name(builder, "menus");
    json_builder_begin_array(builder);
    
    for (xmlNodePtr menu = root->children; menu; menu = menu->next) {
        if (menu->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(menu->name, (const xmlChar *)"menu") != 0) continue;
        
        json_builder_begin_object(builder);
        
        gchar *id = get_prop(menu, "id");
        if (id) {
            json_builder_set_member_name(builder, "id");
            json_builder_add_string_value(builder, id);
            xmlFree(id);
        }
        
        gchar *label = get_prop(menu, "label");
        if (label) {
            json_builder_set_member_name(builder, "label");
            json_builder_add_string_value(builder, label);
            xmlFree(label);
        }
        
        xmlNodePtr icon = find_child(menu, "icon");
        if (icon) {
            gchar *content = get_content(icon);
            json_builder_set_member_name(builder, "icon");
            json_builder_add_string_value(builder, content);
            xmlFree(content);
        }
        
        json_builder_set_member_name(builder, "items");
        json_builder_begin_array(builder);
        
        for (xmlNodePtr item = menu->children; item; item = item->next) {
            if (item->type != XML_ELEMENT_NODE) continue;
            convert_menu_item(builder, item);
        }
        
        json_builder_end_array(builder); /* items */
        json_builder_end_object(builder); /* menu */
    }
    
    json_builder_end_array(builder); /* menus */
    json_builder_end_object(builder); /* root */
    
    JsonGenerator *gen = json_generator_new();
    JsonNode *root_node = json_builder_get_root(builder);
    json_generator_set_root(gen, root_node);
    json_generator_set_pretty(gen, TRUE);
    json_generator_set_indent(gen, 2);
    
    gchar *output = json_generator_to_data(gen, NULL);
    
    json_node_free(root_node);
    g_object_unref(gen);
    g_object_unref(builder);
    xmlFreeDoc(doc);
    
    return output;
}

/* ============================================================================
 * Main
 * ============================================================================ */

static gboolean validate_json(const gchar *json_data, const gchar *schema_file)
{
    if (!opts.validate) return TRUE;
    
    /* Simplified validation - in real implementation would use JSON Schema */
    JsonParser *parser = json_parser_new();
    GError *error = NULL;
    
    if (!json_parser_load_from_data(parser, json_data, -1, &error)) {
        g_printerr("JSON validation failed: %s\\n", error->message);
        g_error_free(error);
        g_object_unref(parser);
        return FALSE;
    }
    
    g_object_unref(parser);
    return TRUE;
}

int main(int argc, char **argv)
{
    /* Initialize libraries */
    xmlInitParser();
    LIBXML_TEST_VERSION;
    
    if (!parse_args(argc, argv)) {
        print_usage(argv[0]);
        xmlCleanupParser();
        return 1;
    }
    
    if (opts.verbose) {
        g_print("Converting: %s\\n", opts.input_file);
    }
    
    /* Determine output path */
    gchar *output_path;
    const gchar *basename = g_path_get_basename(opts.input_file);
    
    if (opts.output_dir) {
        gchar *json_name = g_strdup(basename);
        gsize len = strlen(json_name);
        if (len > 4 && strcmp(json_name + len - 4, ".xml") == 0) {
            strcpy(json_name + len - 4, ".json");
        }
        output_path = g_build_filename(opts.output_dir, json_name, NULL);
        g_free(json_name);
    } else {
        output_path = g_strdup(opts.input_file);
        gsize len = strlen(output_path);
        if (len > 4 && strcmp(output_path + len - 4, ".xml") == 0) {
            strcpy(output_path + len - 4, ".json");
        }
    }
    
    /* Determine file type and convert */
    gchar *output = NULL;
    if (strstr(basename, "rc.xml") != NULL) {
        output = convert_rc_xml(opts.input_file);
    } else if (strstr(basename, "menu.xml") != NULL) {
        output = convert_menu_xml(opts.input_file);
    } else {
        g_printerr("Error: Unknown file type (expected rc.xml or menu.xml)\\n");
        xmlCleanupParser();
        return 1;
    }
    
    if (!output) {
        xmlCleanupParser();
        return 1;
    }
    
    /* Validate */
    if (!validate_json(output, NULL)) {
        g_free(output);
        xmlCleanupParser();
        return 1;
    }
    
    /* Create backup */
    if (opts.backup && !opts.dry_run) {
        gchar *backup_path = g_strdup_printf("%s.backup", opts.input_file);
        GFile *src = g_file_new_for_path(opts.input_file);
        GFile *dst = g_file_new_for_path(backup_path);
        GError *error = NULL;
        
        if (!g_file_copy(src, dst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
            g_printerr("Warning: Failed to create backup: %s\\n", error->message);
            g_error_free(error);
        } else if (opts.verbose) {
            g_print("Created backup: %s\\n", backup_path);
        }
        
        g_object_unref(src);
        g_object_unref(dst);
        g_free(backup_path);
    }
    
    /* Write output */
    if (opts.dry_run) {
        g_print("\\n=== Would write to: %s ===\\n", output_path);
        g_print("%s\\n", output);
    } else {
        GError *error = NULL;
        gchar *dir = g_path_get_dirname(output_path);
        g_mkdir_with_parents(dir, 0755);
        g_free(dir);
        
        if (!g_file_set_contents(output_path, output, -1, &error)) {
            g_printerr("Error: Failed to write %s: %s\\n", 
                      output_path, error->message);
            g_error_free(error);
            g_free(output);
            g_free(output_path);
            xmlCleanupParser();
            return 1;
        }
        
        if (opts.verbose) {
            g_print("Wrote: %s\\n", output_path);
        }
    }
    
    g_free(output);
    g_free(output_path);
    
    xmlCleanupParser();
    return 0;
}
