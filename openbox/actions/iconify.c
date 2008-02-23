#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func(ObActionsData *data, gpointer options);
static gpointer setup_func(xmlNodePtr node);

void action_iconify_startup(void)
{
    actions_register("Iconify",
                     setup_func,
                     NULL,
                     run_func);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;

    if ((n = obt_xml_find_node(node, "de")))
        return GINT_TO_POINTER(obt_xml_node_bool(n));

    return GINT_TO_POINTER(0);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (!actions_client_locked(data)) {
        actions_client_move(data, TRUE);
        client_iconify(data->client, !options, FALSE, FALSE);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
