/* Example Openbox extension: Tiling Window Manager
 * Demonstrates the extension API for creating custom layouts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openbox-extension.h"

/* Extension metadata */
static const ObExtensionInfo extension_info = {
    .api_version = OB_EXTENSION_API_VERSION,
    .name = "tiling",
    .version = "1.0.0",
    .author = "Openbox Team",
    .description = "Simple tiling layout manager for Openbox",
    .license = "GPL-2.0-or-later",
    .min_ob_version = 0x040000,
    .max_ob_version = 0x04FFFF
};

/* Tiling state */
typedef struct {
    uint32_t desktop;
    uint32_t master_count;
    float master_ratio;
    bool active;
} TilingDesktop;

#define MAX_DESKTOPS 32
static TilingDesktop desktops[MAX_DESKTOPS];
static const ObApi *ob_api = NULL;
static ObExtension *extension = NULL;

/* Layout types */
typedef enum {
    LAYOUT_FLOATING = 0,
    LAYOUT_TILED,
    LAYOUT_MONOCLE,
    LAYOUT_GRID
} LayoutType;

static void apply_tiled_layout(uint32_t desktop);
static void apply_monocle_layout(uint32_t desktop);
static void apply_grid_layout(uint32_t desktop);

/* Event handler */
static void on_window_event(ObExtension *ext, const ObEvent *event, void *user_data) {
    (void)ext;
    (void)user_data;
    
    switch (event->type) {
        case OB_EVENT_WINDOW_CREATE:
        case OB_EVENT_WINDOW_UNMAP:
        case OB_EVENT_WINDOW_CONFIGURE: {
            uint32_t desktop = event->data.window.window ? 
                ob_api->window_get_attributes(event->data.window.window)->desktop : 0;
            
            if (desktops[desktop].active) {
                apply_tiled_layout(desktop);
            }
            break;
        }
        
        case OB_EVENT_DESKTOP_SWITCH: {
            uint32_t to = event->data.desktop.to;
            if (desktops[to].active) {
                apply_tiled_layout(to);
            }
            break;
        }
        
        default:
            break;
    }
}

/* Apply tiled layout - master area + stack */
static void apply_tiled_layout(uint32_t desktop) {
    uint32_t count;
    ObWindow **windows = ob_api->desktop_get_windows(desktop, &count);
    
    if (!windows || count == 0) return;
    
    TilingDesktop *td = &desktops[desktop];
    
    /* Get screen geometry */
    ObRect screen = ob_api->screen_get_usable_area(0);
    
    uint32_t master_count = td->master_count;
    if (master_count > count) master_count = count;
    uint32_t stack_count = count - master_count;
    
    uint32_t master_width = screen.width * td->master_ratio;
    uint32_t stack_width = screen.width - master_width;
    
    /* Layout master windows */
    if (master_count > 0) {
        uint32_t master_height = screen.height / master_count;
        for (uint32_t i = 0; i < master_count; i++) {
            ObRect geom = {
                .x = screen.x,
                .y = screen.y + i * master_height,
                .width = master_width,
                .height = master_height
            };
            ob_api->window_set_geometry(windows[i], &geom);
        }
    }
    
    /* Layout stack windows */
    if (stack_count > 0) {
        uint32_t stack_height = screen.height / stack_count;
        for (uint32_t i = 0; i < stack_count; i++) {
            ObRect geom = {
                .x = screen.x + master_width,
                .y = screen.y + i * stack_height,
                .width = stack_width,
                .height = stack_height
            };
            ob_api->window_set_geometry(windows[master_count + i], &geom);
        }
    }
    
    ob_api->window_list_free(windows);
}

/* Apply monocle layout - fullscreen stack */
static void apply_monocle_layout(uint32_t desktop) {
    uint32_t count;
    ObWindow **windows = ob_api->desktop_get_windows(desktop, &count);
    
    if (!windows) return;
    
    ObRect screen = ob_api->screen_get_usable_area(0);
    
    for (uint32_t i = 0; i < count; i++) {
        ObRect geom = {
            .x = screen.x,
            .y = screen.y,
            .width = screen.width,
            .height = screen.height
        };
        ob_api->window_set_geometry(windows[i], &geom);
    }
    
    ob_api->window_list_free(windows);
}

/* Apply grid layout */
static void apply_grid_layout(uint32_t desktop) {
    uint32_t count;
    ObWindow **windows = ob_api->desktop_get_windows(desktop, &count);
    
    if (!windows || count == 0) return;
    
    ObRect screen = ob_api->screen_get_usable_area(0);
    
    /* Calculate grid dimensions */
    uint32_t cols = 1;
    while (cols * cols < count) cols++;
    uint32_t rows = (count + cols - 1) / cols;
    
    uint32_t cell_width = screen.width / cols;
    uint32_t cell_height = screen.height / rows;
    
    for (uint32_t i = 0; i < count; i++) {
        uint32_t col = i % cols;
        uint32_t row = i / cols;
        
        ObRect geom = {
            .x = screen.x + col * cell_width,
            .y = screen.y + row * cell_height,
            .width = cell_width,
            .height = cell_height
        };
        ob_api->window_set_geometry(windows[i], &geom);
    }
    
    ob_api->window_list_free(windows);
}

/* Toggle tiling on/off for desktop */
static void toggle_tiling(uint32_t desktop) {
    if (desktop >= MAX_DESKTOPS) return;
    
    desktops[desktop].active = !desktops[desktop].active;
    
    if (desktops[desktop].active) {
        apply_tiled_layout(desktop);
        ob_api->log_info("Tiling enabled on desktop %u", desktop);
    } else {
        ob_api->log_info("Tiling disabled on desktop %u", desktop);
    }
}

/* Cycle through layouts */
static void cycle_layout(uint32_t desktop) {
    if (desktop >= MAX_DESKTOPS) return;
    
    static LayoutType current = LAYOUT_TILED;
    current = (current + 1) % 4;
    
    desktops[desktop].active = (current != LAYOUT_FLOATING);
    
    switch (current) {
        case LAYOUT_FLOATING:
            ob_api->log_info("Layout: floating");
            break;
        case LAYOUT_TILED:
            apply_tiled_layout(desktop);
            ob_api->log_info("Layout: tiled");
            break;
        case LAYOUT_MONOCLE:
            apply_monocle_layout(desktop);
            ob_api->log_info("Layout: monocle");
            break;
        case LAYOUT_GRID:
            apply_grid_layout(desktop);
            ob_api->log_info("Layout: grid");
            break;
    }
}

/* Increase master ratio */
static void increase_master_ratio(uint32_t desktop) {
    if (desktop >= MAX_DESKTOPS) return;
    desktops[desktop].master_ratio += 0.05f;
    if (desktops[desktop].master_ratio > 0.9f)
        desktops[desktop].master_ratio = 0.9f;
    if (desktops[desktop].active)
        apply_tiled_layout(desktop);
}

/* Decrease master ratio */
static void decrease_master_ratio(uint32_t desktop) {
    if (desktop >= MAX_DESKTOPS) return;
    desktops[desktop].master_ratio -= 0.05f;
    if (desktops[desktop].master_ratio < 0.1f)
        desktops[desktop].master_ratio = 0.1f;
    if (desktops[desktop].active)
        apply_tiled_layout(desktop);
}

/* Extension entry points */
OB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void) {
    return &extension_info;
}

OB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api) {
    ob_api = api;
    extension = ext;
    
    ob_api->log_info("Tiling extension initializing...");
    
    /* Initialize desktop states */
    memset(desktops, 0, sizeof(desktops));
    for (int i = 0; i < MAX_DESKTOPS; i++) {
        desktops[i].master_ratio = 0.5f;
        desktops[i].master_count = 1;
    }
    
    /* Subscribe to events */
    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CREATE, on_window_event, NULL);
    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_UNMAP, on_window_event, NULL);
    ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CONFIGURE, on_window_event, NULL);
    ob_api->event_subscribe(ext, OB_EVENT_DESKTOP_SWITCH, on_window_event, NULL);
    
    ob_api->log_info("Tiling extension loaded. Key bindings:");
    ob_api->log_info("  Mod+Shift+T - Toggle tiling");
    ob_api->log_info("  Mod+Shift+Space - Cycle layouts");
    ob_api->log_info("  Mod+H/L - Adjust master ratio");
    
    return true;
}

OB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext) {
    (void)ext;
    ob_api->log_info("Tiling extension unloading...");
    
    /* Unsubscribe from events */
    ob_api->event_unsubscribe_all(ext);
}

OB_EXTENSION_EXPORT void ob_extension_reload(ObExtension *ext) {
    (void)ext;
    ob_api->log_info("Tiling extension reloading...");
    /* Re-read configuration if needed */
}
