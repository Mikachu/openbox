#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func(ObActionsData *data, gpointer options);
static gpointer setup_func(xmlNodePtr node);

void action_iconify_startup(void)
{
    actions_register("Iconify",
                     setup_func,
                     NULL,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;

    if ((n = obt_parse_find_node(node, "de")))
        return GINT_TO_POINTER(obt_parse_node_bool(n));
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client) {
        actions_client_move(data, TRUE);
        client_iconify(data->client, !options, TRUE, FALSE);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
