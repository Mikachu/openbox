#include "openbox/actions.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/focus_cycle.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_raisetemp_startup(void)
{
    actions_register("RaiseTemp",
                     NULL, NULL,
                     run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (focus_cycle_target) {
        actions_client_move(data, TRUE);
        stacking_temp_raise(CLIENT_AS_WINDOW(data->client));
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
