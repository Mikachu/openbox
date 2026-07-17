/* Openbox xdg-desktop-portal implementation
 * Provides portal interfaces for sandboxed applications
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#define PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_INTERFACE "org.freedesktop.portal.Settings"
#define PORTAL_REQUEST_INTERFACE "org.freedesktop.portal.Request"
#define PORTAL_SESSION_INTERFACE "org.freedesktop.portal.Session"

typedef struct {
    GDBusConnection *connection;
    guint owner_id;
    GHashTable *sessions;
    GHashTable *requests;
} OpenboxPortal;

typedef struct {
    gchar *handle;
    gchar *app_id;
    guint timeout_id;
    GVariant *results;
} PortalRequest;

typedef struct {
    gchar *session_handle;
    gchar *app_id;
    guint signal_id;
} PortalSession;

static OpenboxPortal *global_portal = NULL;

/* Settings portal - expose Openbox theme and settings to sandboxed apps */
static GVariant *get_setting(const gchar *namespace, const gchar *key) {
    if (g_strcmp0(namespace, "org.freedesktop.appearance") == 0) {
        if (g_strcmp0(key, "color-scheme") == 0) {
            /* Return current theme variant: 0=default, 1=prefer-dark */
            const gchar *variant = ob_theme_get_variant();
            return g_variant_new_uint32(
                g_strcmp0(variant, "dark") == 0 ? 1 : 0
            );
        }
        if (g_strcmp0(key, "accent-color") == 0) {
            /* Return accent color as (r, g, b) tuple */
            GdkRGBA accent;
            ob_theme_get_accent_color(&accent);
            return g_variant_new("(ddd)", accent.red, accent.green, accent.blue);
        }
    }
    
    if (g_strcmp0(namespace, "org.openbox") == 0) {
        if (g_strcmp0(key, "theme-name") == 0) {
            return g_variant_new_string(ob_theme_get_name());
        }
        if (g_strcmp0(key, "num-desktops") == 0) {
            return g_variant_new_uint32(ob_screen_num_desktops());
        }
    }
    
    return NULL;
}

static gboolean handle_read_all(GDBusConnection *connection,
                                   const gchar *sender,
                                   const gchar *object_path,
                                   const gchar *interface_name,
                                   const gchar *method_name,
                                   GVariant *parameters,
                                   GDBusMethodInvocation *invocation,
                                   gpointer user_data) {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{ssv}"));
    
    /* Add Openbox settings */
    g_variant_builder_add(&builder, "{ssv}", 
        "org.freedesktop.appearance", "color-scheme",
        get_setting("org.freedesktop.appearance", "color-scheme"));
    
    g_variant_builder_add(&builder, "{ssv}", 
        "org.openbox", "theme-name",
        get_setting("org.openbox", "theme-name"));
    
    g_dbus_method_invocation_return_value(invocation, 
        g_variant_new("(a{ssv})", &builder));
    
    return TRUE;
}

static gboolean handle_read(GDBusConnection *connection,
                              const gchar *sender,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *method_name,
                              GVariant *parameters,
                              GDBusMethodInvocation *invocation,
                              gpointer user_data) {
    const gchar *namespace, *key;
    g_variant_get(parameters, "(&s&s)", &namespace, &key);
    
    GVariant *value = get_setting(namespace, key);
    if (value) {
        g_dbus_method_invocation_return_value(invocation, 
            g_variant_new("(v)", value));
    } else {
        g_dbus_method_invocation_return_dbus_error(invocation,
            "org.freedesktop.portal.Error.NotFound",
            "Setting not found");
    }
    
    return TRUE;
}

/* Screenshot portal */
static gboolean handle_screenshot(GDBusConnection *connection,
                                    const gchar *sender,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *method_name,
                                    GVariant *parameters,
                                    GDBusMethodInvocation *invocation,
                                    gpointer user_data) {
    const gchar *app_id;
    const gchar *parent_window;
    GVariant *options;
    
    g_variant_get(parameters, "(&s&sa{sv})", &app_id, &parent_window, &options);
    
    /* Generate handle */
    gchar *handle = g_strdup_printf("/org/freedesktop/portal/desktop/request/%s/%u",
        sender, g_random_int());
    
    /* Create request object */
    PortalRequest *request = g_new0(PortalRequest, 1);
    request->handle = g_strdup(handle);
    request->app_id = g_strdup(app_id);
    
    g_hash_table_insert(global_portal->requests, g_strdup(handle), request);
    
    /* Return handle */
    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(o)", handle));
    
    /* Perform screenshot */
    gchar *filename = g_strdup_printf("/tmp/screenshot-%u.png", g_random_int());
    
    if (ob_screenshot_take(filename)) {
        GVariantBuilder results;
        g_variant_builder_init(&results, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&results, "{sv}", "uri",
            g_variant_new_string(filename));
        
        /* Emit Response signal */
        g_dbus_connection_emit_signal(connection,
            sender,
            handle,
            PORTAL_REQUEST_INTERFACE,
            "Response",
            g_variant_new("(ua{sv})", 0, &results),
            NULL);
    } else {
        /* Emit error */
        g_dbus_connection_emit_signal(connection,
            sender,
            handle,
            PORTAL_REQUEST_INTERFACE,
            "Response",
            g_variant_new("(ua{sv})", 1, NULL),
            NULL);
    }
    
    g_free(filename);
    
    return TRUE;
}

/* Background portal - for autostart apps */
static gboolean handle_request_background(GDBusConnection *connection,
                                             const gchar *sender,
                                             const gchar *object_path,
                                             const gchar *interface_name,
                                             const gchar *method_name,
                                             GVariant *parameters,
                                             GDBusMethodInvocation *invocation,
                                             gpointer user_data) {
    const gchar *app_id;
    GVariant *options;
    
    g_variant_get(parameters, "(&sa{sv})", &app_id, &options);
    
    /* Check if app is allowed to run in background */
    gboolean allowed = ob_config_app_allowed_background(app_id);
    
    gchar *handle = g_strdup_printf("/org/freedesktop/portal/desktop/request/%s/%u",
        sender, g_random_int());
    
    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(o)", handle));
    
    GVariantBuilder results;
    g_variant_builder_init(&results, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&results, "{sv}", "result",
        g_variant_new_boolean(allowed));
    
    g_dbus_connection_emit_signal(connection,
        sender,
        handle,
        PORTAL_REQUEST_INTERFACE,
        "Response",
        g_variant_new("(ua{sv})", allowed ? 0 : 1, &results),
        NULL);
    
    g_free(handle);
    
    return TRUE;
}

/* Notification portal - forward to desktop notification daemon */
static gboolean handle_add_notification(GDBusConnection *connection,
                                         const gchar *sender,
                                         const gchar *object_path,
                                         const gchar *interface_name,
                                         const gchar *method_name,
                                         GVariant *parameters,
                                         GDBusMethodInvocation *invocation,
                                         gpointer user_data) {
    const gchar *app_id;
    const gchar *id;
    GVariant *notification;
    
    g_variant_get(parameters, "(&s&sa{sv})", &app_id, &id, &notification);
    
    /* Forward to notification daemon via D-Bus */
    GDBusMessage *msg = g_dbus_message_new_method_call(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "Notify");
    
    /* Build notification */
    const gchar *title = "", *body = "";
    GVariant *title_var = g_variant_lookup_value(notification, "title", NULL);
    GVariant *body_var = g_variant_lookup_value(notification, "body", NULL);
    
    if (title_var) {
        title = g_variant_get_string(title_var, NULL);
    }
    if (body_var) {
        body = g_variant_get_string(body_var, NULL);
    }
    
    g_dbus_message_set_body(msg, g_variant_new("(susssasa{sv}i)",
        app_id,           /* app_name */
        0,                /* replaces_id */
        "",              /* app_icon */
        title,
        body,
        NULL,             /* actions */
        NULL,             /* hints */
        -1));             /* expire_timeout */
    
    g_dbus_connection_send_message(connection, msg, 
        G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
    
    g_object_unref(msg);
    
    g_dbus_method_invocation_return_value(invocation, NULL);
    
    return TRUE;
}

static const GDBusInterfaceVTable settings_vtable = {
    handle_read,
    handle_read_all,
    NULL
};

static const GDBusInterfaceVTable screenshot_vtable = {
    handle_screenshot,
    NULL,
    NULL
};

static const GDBusInterfaceVTable background_vtable = {
    handle_request_background,
    NULL,
    NULL
};

static const GDBusInterfaceVTable notification_vtable = {
    handle_add_notification,
    NULL,
    NULL
};

gboolean openbox_portal_init(void) {
    GError *error = NULL;
    
    global_portal = g_new0(OpenboxPortal, 1);
    global_portal->sessions = g_hash_table_new_full(g_str_hash, g_str_equal,
        g_free, g_free);
    global_portal->requests = g_hash_table_new_full(g_str_hash, g_str_equal,
        g_free, g_free);
    
    global_portal->connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!global_portal->connection) {
        g_warning("Failed to connect to session bus: %s", error->message);
        g_error_free(error);
        g_free(global_portal);
        return FALSE;
    }
    
    /* Export portal interfaces */
    GDBusNodeInfo *node_info = g_dbus_node_info_new_for_xml(
        "<node>"
        "<interface name='org.freedesktop.portal.Settings'>"
        "<method name='Read'>"
        "<arg type='s' name='namespace' direction='in'/>"
        "<arg type='s' name='key' direction='in'/>"
        "<arg type='v' name='value' direction='out'/>"
        "</method>"
        "<method name='ReadAll'>"
        "<arg type='a{ssv}' name='values' direction='out'/>"
        "</method>"
        "<signal name='SettingChanged'>"
        "<arg type='s' name='namespace'/>"
        "<arg type='s' name='key'/>"
        "<arg type='v' name='value'/>"
        "</signal>"
        "</interface>"
        "</node>", &error);
    
    if (node_info) {
        g_dbus_connection_register_object(global_portal->connection,
            PORTAL_OBJECT_PATH,
            node_info->interfaces[0],
            &settings_vtable,
            NULL, NULL, &error);
        g_dbus_node_info_unref(node_info);
    }
    
    /* Own the portal bus name */
    global_portal->owner_id = g_bus_own_name_on_connection(
        global_portal->connection,
        PORTAL_BUS_NAME,
        G_BUS_NAME_OWNER_FLAGS_REPLACE,
        NULL, NULL, NULL, NULL);
    
    g_message("Openbox portal initialized");
    return TRUE;
}

void openbox_portal_shutdown(void) {
    if (!global_portal) return;
    
    if (global_portal->owner_id) {
        g_bus_unown_name(global_portal->owner_id);
    }
    
    g_hash_table_destroy(global_portal->sessions);
    g_hash_table_destroy(global_portal->requests);
    
    if (global_portal->connection) {
        g_object_unref(global_portal->connection);
    }
    
    g_free(global_portal);
    global_portal = NULL;
}

/* Emit setting changed signal */
void openbox_portal_emit_setting_changed(const gchar *namespace,
                                          const gchar *key,
                                          GVariant *value) {
    if (!global_portal || !global_portal->connection) return;
    
    g_dbus_connection_emit_signal(global_portal->connection,
        NULL,
        PORTAL_OBJECT_PATH,
        PORTAL_INTERFACE,
        "SettingChanged",
        g_variant_new("(ssv)", namespace, key, value),
        NULL);
}
