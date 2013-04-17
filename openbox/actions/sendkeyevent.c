#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/window.h"
#include "openbox/translate.h"
#include "obt/display.h"
#include "obt/prop.h"
#include "openbox/openbox.h"
#include "gettext.h"

typedef struct {
    guint key;
    guint state;
    gboolean target;
} Options;

static gpointer setup_sendkey_func(xmlNodePtr node);
static gboolean sendkey(ObActionsData *data, gpointer options);
static gboolean settarget(ObActionsData *data, gpointer options);

static Window target;

void action_sendkeyevent_startup(void)
{
    actions_register("SendKeyEvent",
                     setup_sendkey_func, g_free,
                     sendkey);
    actions_register("SetKeyTarget",
                     NULL, NULL,
                     settarget);
    OBT_PROP_GET32(obt_root(ob_screen), OB_TARGET_WINDOW, WINDOW, (guint32 *)&target);
}

static gpointer setup_sendkey_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->target = TRUE;

    if ((n = obt_xml_find_node(node, "key"))) {
        gchar *s = obt_xml_node_string(n);
        translate_key(s, &o->state, &o->key);
        g_free(s);
    } else
        translate_key("space", &o->state, &o->key);
    if ((n = obt_xml_find_node(node, "usetarget")))
        o->target = obt_xml_node_bool(n);

    return o;
}

/* Always return FALSE because its not interactive */
static gboolean sendkey(ObActionsData *data, gpointer options)
{
    Options *o = options;
    XEvent ev;
    Window win;

    if (!o->key) /* the key couldn't be parsed */
        return FALSE;

    if (o->target)
        win = target;
    else if (data->client)
        win = data->client->window;
    else
        return FALSE;

    ev.xkey.window = win;
    ev.xkey.state = o->state;
    ev.xkey.keycode = o->key;
    obt_display_ignore_errors(TRUE);
    ev.type = KeyPress;
    XSendEvent(obt_display, win, False, 0, &ev);
    ev.type = KeyRelease;
    XSendEvent(obt_display, win, False, 0, &ev);
    obt_display_ignore_errors(FALSE);

    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean settarget(ObActionsData *data, gpointer options)
{
    if (data->client) {
      target = data->client->window;
      OBT_PROP_SET32(obt_root(ob_screen), OB_TARGET_WINDOW, WINDOW, target);
    }

    return FALSE;
}
