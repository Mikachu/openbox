#include "openbox/actions.h"
#include "openbox/stacking.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_raiselower_startup(void)
{
    actions_register("RaiseLower", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (!actions_client_locked(data)) {
        actions_client_move(data, TRUE);
        stacking_restack_request(data->client, NULL, Opposite);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
