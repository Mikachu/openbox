/* Openbox Extension API - IMPROVED v4.0.1
 * Stable C ABI for plugin development
 * 
 * IMPROVEMENTS:
 * - Added missing window state queries (urgent, demands_attention)
 * - Added window geometry queries with screen coordinates
 * - Improved event delivery with filtering and priorities
 * - Added extension-to-extension communication
 * - Added configuration change notifications
 * - Better error reporting through return codes
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
#define OB_EXTENSION_API_VERSION_MINOR 1  /* Bumped for 4.0.1 improvements */
#define OB_EXTENSION_API_VERSION ((OB_EXTENSION_API_VERSION_MAJOR << 16) | OB_EXTENSION_API_VERSION_MINOR)

/* Error codes for better error reporting */
typedef enum {
    OB_SUCCESS = 0,
    OB_ERROR_INVALID_ARGUMENT = -1,
    OB_ERROR_NOT_FOUND = -2,
    OB_ERROR_NO_MEMORY = -3,
    OB_ERROR_PERMISSION_DENIED = -4,
    OB_ERROR_NOT_IMPLEMENTED = -5,
    OB_ERROR_BUSY = -6,
    OB_ERROR_DISCONNECTED = -7,
    OB_ERROR_TIMEOUT = -8,
} ObErrorCode;

/* Opaque handles */
typedef struct ObExtension ObExtension;
typedef struct ObWindow ObWindow;
typedef struct ObClient ObClient;
typedef struct ObScreen ObScreen;
typedef struct ObConfig ObConfig;

/* Event types - EXTENDED */
typedef enum {
    OB_EVENT_WINDOW_CREATE = 0,
    OB_EVENT_WINDOW_DESTROY,
    OB_EVENT_WINDOW_FOCUS,
    OB_EVENT_WINDOW_UNFOCUS,
    OB_EVENT_WINDOW_MAP,
    OB_EVENT_WINDOW_UNMAP,
    OB_EVENT_WINDOW_CONFIGURE,      /* Geometry changed */
    OB_EVENT_WINDOW_PROPERTY,        /* Property changed (NEW) */
    OB_EVENT_WINDOW_RAISE,           /* Window raised (NEW) */
    OB_EVENT_WINDOW_LOWER,           /* Window lowered (NEW) */
    OB_EVENT_WINDOW_ICONIFY,         /* Window iconified (NEW) */
    OB_EVENT_WINDOW_DEICONIFY,       /* Window deiconified (NEW) */
    OB_EVENT_WINDOW_MAXIMIZE,        /* Window maximized (NEW) */
    OB_EVENT_WINDOW_UNMAXIMIZE,      /* Window unmaximized (NEW) */
    OB_EVENT_WINDOW_FULLSCREEN,      /* Window fullscreened (NEW) */
    OB_EVENT_WINDOW_UNFULLSCREEN,    /* Window unfullscreened (NEW) */
    OB_EVENT_DESKTOP_SWITCH,
    OB_EVENT_DESKTOP_ADD,              /* Desktop added (NEW) */
    OB_EVENT_DESKTOP_REMOVE,           /* Desktop removed (NEW) */
    OB_EVENT_DESKTOP_RENAME,           /* Desktop renamed (NEW) */
    OB_EVENT_THEME_CHANGE,
    OB_EVENT_CONFIG_RELOAD,
    OB_EVENT_CONFIG_CHANGE,            /* Single config value changed (NEW) */
    OB_EVENT_EXTENSION_LOAD,           /* Extension loaded (NEW) */
    OB_EVENT_EXTENSION_UNLOAD,         /* Extension unloaded (NEW) */
    OB_EVENT_SHUTDOWN,
    OB_EVENT_COUNT
} ObEventType;

/* Event priorities for filtering */
typedef enum {
    OB_PRIORITY_LOW = 0,
    OB_PRIORITY_NORMAL = 1,
    OB_PRIORITY_HIGH = 2,
    OB_PRIORITY_CRITICAL = 3
} ObEventPriority;

/* Window states - EXTENDED */
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
    OB_WINDOW_STATE_DEMANDS_ATTENTION = 1 << 11,
    OB_WINDOW_STATE_URGENT = 1 << 12,           /* NEW */
    OB_WINDOW_STATE_FOCUSED = 1 << 13,          /* NEW */
    OB_WINDOW_STATE_ICONIFIED = 1 << 14,        /* NEW */
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

/* Window properties that can be queried (NEW) */
typedef enum {
    OB_PROP_TITLE = 0,
    OB_PROP_CLASS,
    OB_PROP_NAME,
    OB_PROP_ROLE,
    OB_PROP_ICON_NAME,
    OB_PROP_WINDOW_TYPE,
    OB_PROP_STATES,
    OB_PROP_DESKTOP,
    OB_PROP_GEOMETRY,      /* Client geometry */
    OB_PROP_FRAME_GEOMETRY, /* Frame geometry (NEW) */
    OB_PROP_SCREEN,         /* Screen number (NEW) */
    OB_PROP_WORKSPACE,      /* Workspace area (NEW) */
    OB_PROP_PID,            /* Process ID (NEW) */
    OB_PROP_CLIENT_MACHINE, /* Hostname (NEW) */
    OB_PROP_COUNT
} ObWindowProperty;

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
    /* NEW: Optional dependencies */
    const char **depends_on;  /* NULL-terminated list of extension names */
} ObExtensionInfo;

/* Window geometry - EXTENDED with screen coordinates */
typedef struct {
    int32_t x;              /* Client area X */
    int32_t y;              /* Client area Y */
    uint32_t width;         /* Client width */
    uint32_t height;        /* Client height */
    /* NEW: Frame geometry (including borders) */
    int32_t frame_x;
    int32_t frame_y;
    uint32_t frame_width;
    uint32_t frame_height;
    /* NEW: Border widths */
    uint32_t border_top;
    uint32_t border_bottom;
    uint32_t border_left;
    uint32_t border_right;
} ObRect;

/* Window attributes - EXTENDED */
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
    bool demands_attention;  /* NEW: separate from urgent */
    bool focused;            /* NEW */
    uint32_t screen;         /* NEW: screen number */
    uint32_t pid;          /* NEW: process ID */
    const char *client_machine; /* NEW: hostname */
} ObWindowAttributes;

/* Event data - EXTENDED */
typedef struct {
    ObEventType type;
    uint64_t timestamp;
    ObEventPriority priority;  /* NEW */
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
        struct {
            const char *key;       /* Config key that changed */
            const char *old_value;
            const char *new_value;
        } config;  /* NEW */
        struct {
            const char *extension_name;
            bool success;
            const char *error_message;
        } extension;  /* NEW */
    } data;
} ObEvent;

/* Event filter for selective subscription (NEW) */
typedef struct {
    ObEventType *types;      /* NULL = all types */
    uint32_t n_types;
    ObWindow **windows;      /* NULL = all windows */
    uint32_t n_windows;
    uint32_t min_priority;   /* Minimum priority to receive */
} ObEventFilter;

/* Callback types */
typedef void (*ObEventCallback)(ObExtension *ext, const ObEvent *event, void *user_data);
typedef void (*ObLogCallback)(const char *domain, int level, const char *message);
typedef void (*ObPropertyCallback)(ObWindow *window, ObWindowProperty prop, void *user_data);  /* NEW */

/* Extension entry points */
typedef bool (*ObExtensionInitFunc)(ObExtension *ext);
typedef void (*ObExtensionFiniFunc)(ObExtension *ext);
typedef void (*ObExtensionReloadFunc)(ObExtension *ext);
typedef ObErrorCode (*ObExtensionGetErrorFunc)(void);  /* NEW: Get last error */

/* API functions table - EXTENDED */
typedef struct {
    /* Version checking */
    uint32_t (*get_api_version)(void);
    bool (*check_version)(uint32_t required);
    const char* (*get_openbox_version)(void);  /* NEW */
    
    /* Logging */
    void (*log_debug)(const char *fmt, ...);
    void (*log_info)(const char *fmt, ...);
    void (*log_warning)(const char *fmt, ...);
    void (*log_error)(const char *fmt, ...);
    void (*set_log_callback)(ObLogCallback callback);
    void (*set_log_level)(int level);  /* NEW: 0=debug, 1=info, 2=warn, 3=error */
    
    /* Window management */
    ObWindow* (*window_get_focused)(void);
    ObWindow* (*window_get_under_mouse)(void);
    ObWindow** (*window_get_all)(uint32_t *count);
    void (*window_list_free)(ObWindow **windows);
    
    ObWindowAttributes* (*window_get_attributes)(ObWindow *window);
    void (*window_attributes_free)(ObWindowAttributes *attrs);
    
    /* NEW: Property queries */
    char* (*window_get_property_string)(ObWindow *window, ObWindowProperty prop);
    int64_t (*window_get_property_int)(ObWindow *window, ObWindowProperty prop);
    bool (*window_get_property_bool)(ObWindow *window, ObWindowProperty prop);
    void (*window_property_free)(void *value);
    
    uint32_t (*window_get_id)(ObWindow *window);
    bool (*window_is_focused)(ObWindow *window);
    bool (*window_is_visible)(ObWindow *window);
    bool (*window_is_iconified)(ObWindow *window);  /* NEW */
    bool (*window_is_maximized)(ObWindow *window);  /* NEW */
    bool (*window_is_fullscreen)(ObWindow *window);  /* NEW */
    bool (*window_is_urgent)(ObWindow *window);  /* NEW */
    bool (*window_demands_attention)(ObWindow *window);  /* NEW */
    
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
    void (*window_resize_relative)(ObWindow *window, int32_t dw, int32_t dh);  /* NEW */
    
    /* Desktop management */
    uint32_t (*desktop_get_count)(void);
    uint32_t (*desktop_get_current)(void);
    void (*desktop_set_current)(uint32_t desktop);
    void (*desktop_add)(void);
    void (*desktop_remove)(uint32_t desktop);
    const char* (*desktop_get_name)(uint32_t desktop);
    void (*desktop_set_name)(uint32_t desktop, const char *name);
    ObWindow** (*desktop_get_windows)(uint32_t desktop, uint32_t *count);
    uint32_t (*desktop_get_window_count)(uint32_t desktop);  /* NEW: fast count */
    
    /* Screen/Monitor management */
    uint32_t (*screen_get_count)(void);
    ObRect (*screen_get_geometry)(uint32_t screen);
    ObRect (*screen_get_usable_area)(uint32_t screen);
    uint32_t (*screen_get_current)(void);
    uint32_t (*screen_get_for_window)(ObWindow *window);  /* NEW */
    
    /* Configuration */
    const char* (*config_get_string)(const char *key, const char *default_val);
    int (*config_get_int)(const char *key, int default_val);
    bool (*config_get_bool)(const char *key, bool default_val);
    double (*config_get_double)(const char *key, double default_val);
    void (*config_set)(const char *key, const char *value);
    void (*config_reload)(void);
    void (*config_watch)(const char *key, ObPropertyCallback callback, void *user_data);  /* NEW */
    void (*config_unwatch)(const char *key, ObPropertyCallback callback);  /* NEW */
    
    /* Theme */
    const char* (*theme_get_name)(void);
    const char* (*theme_get_variant)(void);
    bool (*theme_get_color)(const char *element, double *r, double *g, double *b);
    void (*theme_reload)(void);
    char** (*theme_list_available)(uint32_t *count);  /* NEW */
    void (*theme_list_free)(char **themes);  /* NEW */
    
    /* Event subscription - EXTENDED */
    bool (*event_subscribe)(ObExtension *ext, ObEventType type, 
                            ObEventCallback callback, void *user_data);
    bool (*event_subscribe_filtered)(ObExtension *ext, const ObEventFilter *filter,
                                       ObEventCallback callback, void *user_data);  /* NEW */
    bool (*event_unsubscribe)(ObExtension *ext, ObEventType type);
    void (*event_unsubscribe_all)(ObExtension *ext);
    void (*event_set_priority)(ObExtension *ext, ObEventPriority priority);  /* NEW */
    
    /* Timers */
    uint32_t (*timer_add)(uint32_t interval_ms, void (*callback)(void *), void *user_data);
    void (*timer_remove)(uint32_t timer_id);
    uint32_t (*timer_add_once)(uint32_t delay_ms, void (*callback)(void *), void *user_data);  /* NEW */
    
    /* Menus */
    void (*menu_show)(const char *menu_id, int x, int y);
    void (*menu_hide)(void);
    char** (*menu_list)(uint32_t *count);  /* NEW */
    void (*menu_list_free)(char **menus);  /* NEW */
    
    /* Actions */
    void (*action_execute)(const char *command);
    void (*action_show_menu)(const char *menu);
    void (*action_next_window)(void);
    void (*action_previous_window)(void);
    void (*action_restart)(void);
    void (*action_exit)(void);
    void (*action_reconfigure)(void);  /* NEW */
    
    /* Extension management - EXTENDED */
    ObExtension* (*extension_get)(const char *name);
    void* (*extension_get_symbol)(ObExtension *ext, const char *symbol);
    bool (*extension_is_loaded)(const char *name);  /* NEW */
    char** (*extension_list_loaded)(uint32_t *count);  /* NEW */
    void (*extension_list_free)(char **extensions);  /* NEW */
    void (*extension_emit_signal)(const char *signal, void *data, size_t size);
    bool (*extension_connect_signal)(const char *signal, void (*callback)(void *data, size_t size));
    bool (*extension_send_message)(const char *target_ext, const char *message, 
                                    void *data, size_t size);  /* NEW */
    void (*extension_set_data)(ObExtension *ext, const char *key, void *data);  /* NEW */
    void* (*extension_get_data)(ObExtension *ext, const char *key);  /* NEW */
    
    /* Memory */
    void* (*mem_alloc)(size_t size);
    void (*mem_free)(void *ptr);
    void* (*mem_realloc)(void *ptr, size_t new_size);  /* NEW */
    char* (*mem_strdup)(const char *str);
    char* (*mem_strndup)(const char *str, size_t n);  /* NEW */
    
    /* Error handling - NEW */
    ObErrorCode (*get_last_error)(void);
    const char* (*error_string)(ObErrorCode code);
    void (*clear_error)(void);
    
    /* Utility - NEW */
    char* (*get_config_dir)(void);
    char* (*get_cache_dir)(void);
    char* (*get_data_dir)(void);
    void (*path_free)(char *path);
    
} ObApi;

/* Extension entry point - must be implemented by extension */
OB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void);
OB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api);
OB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext);
OB_EXTENSION_EXPORT void ob_extension_reload(ObExtension *ext);
OB_EXTENSION_EXPORT ObErrorCode ob_extension_get_last_error(void);  /* NEW */

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

/* Error checking helper - NEW */
#define OB_CHECK(expr) do { \
    ObErrorCode _err = (expr); \
    if (_err != OB_SUCCESS) { \
        OB_LOG_ERROR(\"Operation failed: %s\", ob_api->error_string(_err)); \
        return false; \
    } \
} while(0)

#ifdef __cplusplus
}
#endif

#endif /* OPENBOX_EXTENSION_H */
