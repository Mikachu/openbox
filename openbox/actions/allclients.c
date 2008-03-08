#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/client.h"

static gpointer setup_func(xmlNodePtr node);
static void     free_func(gpointer acts);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_allclients_startup(void)
{
    actions_register("AllClients", setup_func, free_func, run_func, NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    GSList *acts = NULL;

    n = obt_parse_find_node(node, "action");
    while (n) {
        ObActionsAct *action = actions_parse(n);
        if (action) acts = g_slist_prepend(acts, action);
        n = obt_parse_find_node(n->next, "action");
    }

    return acts;
}

static void free_func(gpointer acts)
{
    GSList *a = acts;

    while (a) {
        actions_act_unref(a->data);
        a = g_slist_delete_link(a, a);
    }
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer acts)
{
    GList *it;
    GSList *a = acts;

    if (a)
        for (it = client_list; it; it = g_list_next(it)) {
            ObClient *c = it->data;
            if (actions_run_acts(a, data->uact, data->state,
                                 data->x, data->y, data->button,
                                 data->context, c))
                return TRUE;
        }

    return FALSE;
}
