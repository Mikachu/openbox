#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/window.h"
#include "obt/display.h"
#include "obt/prop.h"
#include "openbox/openbox.h"
#include "gettext.h"

typedef struct {
    KeyCode key;
} Options;

static gpointer setup_sendkey_func(xmlNodePtr node);
static void free_sendkey_func(gpointer options);
static gboolean sendkey(ObActionsData *data, gpointer options);
static gboolean settarget(ObActionsData *data, gpointer options);

static Window target;

void action_sendkeyevent_startup(void)
{
    actions_register("SendKeyEvent",
                     setup_sendkey_func, g_free,
                     sendkey,
                     NULL, NULL);
    actions_register("SetKeyTarget",
                     NULL, NULL,
                     settarget,
                     NULL, NULL);
    OBT_PROP_GET32(obt_root(ob_screen), OB_TARGET_WINDOW, WINDOW, (guint32 *)&target);
}

static KeyCode parse_key(gchar *s)
{
    KeySym sym;

    sym = XStringToKeysym(s);
    if (sym == NoSymbol) {
        g_warning(_("Invalid key name '%s' in SendKeyEvent action."), s);
        return 0;
    }

    return XKeysymToKeycode(obt_display, sym);
}

static gpointer setup_sendkey_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "key"))) {
        gchar *s = obt_parse_node_string(n);
        o->key = parse_key(s);
        g_free(s);
    } else
        o->key = parse_key("space");

    return o;
}

/* Always return FALSE because its not interactive */
static gboolean sendkey(ObActionsData *data, gpointer options)
{
    Options *o = options;
    XEvent ev;

    if (!o->key) /* the key couldn't be parsed */
        return FALSE;

    ev.xkey.window = target;
    ev.xkey.state = 0;
    ev.xkey.keycode = o->key;
    obt_display_ignore_errors(TRUE);
    ev.type = KeyPress;
    XSendEvent(obt_display, target, False, 0, &ev);
    ev.type = KeyRelease;
    XSendEvent(obt_display, target, False, 0, &ev);
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
