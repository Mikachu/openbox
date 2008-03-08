#include "openbox/actions.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_stop_startup(void)
{
    actions_register("Stop", NULL, NULL, run_func, NULL, NULL);
}

/* return TRUE to stop other actions from running */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    return TRUE;
}
