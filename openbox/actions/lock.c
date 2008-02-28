#include "openbox/actions.h"
#include "openbox/client.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_lock_startup(void)
{
    actions_register("Lock", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (data->client)
        client_set_locked(data->client, !data->client->locked);

    return FALSE;
}
