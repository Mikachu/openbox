/* Openbox Extension API
 * Stable C ABI for plugin development
 */

#ifndef OPENBOX_EXTENSION_H
#define OPENBOX_EXTENSION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define OB_EXTENSION_API_VERSION_MAJOR 1
#define OB_EXTENSION_API_VERSION_MINOR 0
#define OB_EXTENSION_API_VERSION ((OB_EXTENSION_API_VERSION_MAJOR << 16) | OB_EXTENSION_API_VERSION_MINOR)

/* Opaque handles */
typedef struct ObExtension ObExtension;
typedef struct ObWindow ObWindow;
typedef struct ObClient ObClient;
typedef struct ObScreen ObScreen;
typedef struct ObConfig ObConfig;

/* Event types */
typedef enum {
    OB_EVENT_WINDOW_CREATE = 0,
    OB_EVENT_WINDOW_DESTROY,
    OB_EVENT_WINDOW_FOCUS,
    OB_EVENT_WINDOW_UNFOCUS,
    OB_EVENT_WINDOW_MAP,
    OB_EVENT_WINDOW_UNMAP,
    OB_EVENT_WINDOW_CONFIGURE,
    OB_EVENT_DESKTOP_SWITCH,
    OB_EVENT_THEME_CHANGE,
    OB_EVENT_CONFIG_RELOAD,
    OB_EVENT_SHUTDOWN,
    OB_EVENT_COUNT
} ObEventType;

/* Window states */
typedef enum {
    OB_WINDOW_STATE_MODAL = 1 << 0,
    OB_WINDOW_STATE_STICKY = 1 << 1,
    OB_WINDOW_STATE_MAXIMIZED_VERT = 1 << 2,
    OB_WINDOW_STATE_MAXIMIZED_HORZ = 1 << 3,
    OB_WINDOW_STATE_SHADED = 1 << 4,
    OB_WINDOW_STATE_SKIP_TASKBAR = 1 << 5,
    OB_WINDOW_STATE_SKIP_PAGER = 1 << 6,
    OB_WINDOW_STATE_HIDDEN = 1 << 7,
    OB_WINDOW_STATE_FULLSCREEN = 1 << 8,
    OB_WINDOW_STATE_ABOVE = 1 << 9,
    OB_WINDOW_STATE_BELOW = 1 << 10,
    OB_WINDOW_STATE_DEMANDS_ATTENTION = 1 << 11
} ObWindowState;

/* Window types */
typedef enum {
    OB_WINDOW_TYPE_UNKNOWN = 0,
    OB_WINDOW_TYPE_NORMAL,
    OB_WINDOW_TYPE_DIALOG,
    OB_WINDOW_TYPE_UTILITY,
    OB_WINDOW_TYPE_MENU,
    OB_WINDOW_TYPE_TOOLBAR,
    OB_WINDOW_TYPE_SPLASH,
    OB_WINDOW_TYPE_DIALOG_MODAL,
    OB_WINDOW_TYPE_NORMAL_DIALOG,
    OB_WINDOW_TYPE_DROPDOWN_MENU,
    OB_WINDOW_TYPE_POPUP_MENU,
    OB_WINDOW_TYPE_TOOLTIP,
    OB_WINDOW_TYPE_NOTIFICATION,
    OB_WINDOW_TYPE_COMBO,
    OB_WINDOW_TYPE_DND,
    OB_WINDOW_TYPE_NORMAL_NORMAL
} ObWindowType;

/* Gravity */
typedef enum {
    OB_GRAVITY_NORTH_WEST = 1,
    OB_GRAVITY_NORTH = 2,
    OB_GRAVITY_NORTH_EAST = 3,
    OB_GRAVITY_WEST = 4,
    OB_GRAVITY_CENTER = 5,
    OB_GRAVITY_EAST = 6,
    OB_GRAVITY_SOUTH_WEST = 7,
    OB_GRAVITY_SOUTH = 8,
    OB_GRAVITY_SOUTH_EAST = 9,
    OB_GRAVITY_STATIC = 10
} ObGravity;

/* Extension metadata */
typedef struct {
    uint32_t api_version;
    const char *name;
    const char *version;
    const char *author;
    const char *description;
    const char *license;
    uint32_t min_ob_version;
    uint32_t max_ob_version;
} ObExtensionInfo;

/* Window geometry */
typedef struct {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
} ObRect;

/* Window attributes */
typedef struct {
    const char *title;
    const char *class;
    const char *name;
    const char *role;
    const char *icon_name;
    ObWindowType type;
    ObWindowState states;
    ObRect area;
    ObRect client_area;
    uint32_t desktop;
    bool shaded;
    bool maximized;
    bool minimized;
    bool fullscreen;
    bool above;
    bool below;
    bool urgent;
} ObWindowAttributes;

/* Event data */
typedef struct {
    ObEventType type;
    uint64_t timestamp;
    union {
        struct {
            ObWindow *window;
            ObWindowAttributes attrs;
        } window;
        struct {
            uint32_t from;
            uint32_t to;
        } desktop;
        struct {
            const char *old_theme;
            const char *new_theme;
        } theme;
    } data;
} ObEvent;

/* Callback types */
typedef void (*ObEventCallback)(ObExtension *ext, const ObEvent *event, void *user_data);
typedef void (*ObLogCallback)(const char *domain, int level, const char *message);

/* Extension entry points */
typedef bool (*ObExtensionInitFunc)(ObExtension *ext);
typedef void (*ObExtensionFiniFunc)(ObExtension *ext);
typedef void (*ObExtensionReloadFunc)(ObExtension *ext);

/* API functions table */
typedef struct {
    /* Version checking */
    uint32_t (*get_api_version)(void);
    bool (*check_version)(uint32_t required);
    
    /* Logging */
    void (*log_debug)(const char *fmt, ...);
    void (*log_info)(const char *fmt, ...);
    void (*log_warning)(const char *fmt, ...);
    void (*log_error)(const char *fmt, ...);
    void (*set_log_callback)(ObLogCallback callback);
    
    /* Window management */
    ObWindow* (*window_get_focused)(void);
    ObWindow* (*window_get_under_mouse)(void);
    ObWindow** (*window_get_all)(uint32_t *count);
    void (*window_list_free)(ObWindow **windows);
    
    ObWindowAttributes* (*window_get_attributes)(ObWindow *window);
    void (*window_attributes_free)(ObWindowAttributes *attrs);
    
    uint32_t (*window_get_id)(ObWindow *window);
    bool (*window_is_focused)(ObWindow *window);
    bool (*window_is_visible)(ObWindow *window);
    
    void (*window_focus)(ObWindow *window);
    void (*window_raise)(ObWindow *window);
    void (*window_lower)(ObWindow *window);
    void (*window_close)(ObWindow *window);
    void (*window_kill)(ObWindow *window);
    void (*window_iconify)(ObWindow *window);
    void (*window_deiconify)(ObWindow *window);
    void (*window_maximize)(ObWindow *window, bool horz, bool vert);
    void (*window_unmaximize)(ObWindow *window);
    void (*window_shade)(ObWindow *window);
    void (*window_unshade)(ObWindow *window);
    void (*window_set_fullscreen)(ObWindow *window, bool fullscreen);
    void (*window_set_above)(ObWindow *window, bool above);
    void (*window_set_below)(ObWindow *window, bool below);
    void (*window_set_desktop)(ObWindow *window, uint32_t desktop);
    void (*window_set_geometry)(ObWindow *window, const ObRect *geometry);
    void (*window_move)(ObWindow *window, int32_t x, int32_t y);
    void (*window_resize)(ObWindow *window, uint32_t width, uint32_t height);
    
    /* Desktop management */
    uint32_t (*desktop_get_count)(void);
    uint32_t (*desktop_get_current)(void);
    void (*desktop_set_current)(uint32_t desktop);
    void (*desktop_add)(void);
    void (*desktop_remove)(uint32_t desktop);
    const char* (*desktop_get_name)(uint32_t desktop);
    void (*desktop_set_name)(uint32_t desktop, const char *name);
    ObWindow** (*desktop_get_windows)(uint32_t desktop, uint32_t *count);
    
    /* Screen/Monitor management */
    uint32_t (*screen_get_count)(void);
    ObRect (*screen_get_geometry)(uint32_t screen);
    ObRect (*screen_get_usable_area)(uint32_t screen);
    uint32_t (*screen_get_current)(void);
    
    /* Configuration */
    const char* (*config_get_string)(const char *key, const char *default_val);
    int (*config_get_int)(const char *key, int default_val);
    bool (*config_get_bool)(const char *key, bool default_val);
    double (*config_get_double)(const char *key, double default_val);
    void (*config_set)(const char *key, const char *value);
    void (*config_reload)(void);
    
    /* Theme */
    const char* (*theme_get_name)(void);
    const char* (*theme_get_variant)(void);
    bool (*theme_get_color)(const char *element, double *r, double *g, double *b);
    void (*theme_reload)(void);
    
    /* Event subscription */
    bool (*event_subscribe)(ObExtension *ext, ObEventType type, ObEventCallback callback, void *user_data);
    bool (*event_unsubscribe)(ObExtension *ext, ObEventType type);
    void (*event_unsubscribe_all)(ObExtension *ext);
    
    /* Timers */
    uint32_t (*timer_add)(uint32_t interval_ms, void (*callback)(void *), void *user_data);
    void (*timer_remove)(uint32_t timer_id);
    
    /* Menus */
    void (*menu_show)(const char *menu_id, int x, int y);
    void (*menu_hide)(void);
    
    /* Actions */
    void (*action_execute)(const char *command);
    void (*action_show_menu)(const char *menu);
    void (*action_next_window)(void);
    void (*action_previous_window)(void);
    void (*action_restart)(void);
    void (*action_exit)(void);
    
    /* Extension management */
    ObExtension* (*extension_get)(const char *name);
    void* (*extension_get_symbol)(ObExtension *ext, const char *symbol);
    void (*extension_emit_signal)(const char *signal, void *data, size_t size);
    bool (*extension_connect_signal)(const char *signal, void (*callback)(void *data, size_t size));
    
    /* Memory */
    void* (*mem_alloc)(size_t size);
    void (*mem_free)(void *ptr);
    char* (*mem_strdup)(const char *str);
    
} ObApi;

/* Extension entry point - must be implemented by extension */
OB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void);
OB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api);
OB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext);
OB_EXTENSION_EXPORT void ob_extension_reload(ObExtension *ext);

/* Helper macros */
#define OB_EXTENSION_EXPORT __attribute__((visibility("default")))

#define OB_LOG_DEBUG(...) ob_api->log_debug(__VA_ARGS__)
#define OB_LOG_INFO(...) ob_api->log_info(__VA_ARGS__)
#define OB_LOG_WARNING(...) ob_api->log_warning(__VA_ARGS__)
#define OB_LOG_ERROR(...) ob_api->log_error(__VA_ARGS__)

/* Convenience functions (implemented by extension) */
static inline bool ob_extension_check_version(uint32_t required) {
    return required <= OB_EXTENSION_API_VERSION;
}

#ifdef __cplusplus
}
#endif

#endif /* OPENBOX_EXTENSION_H */
