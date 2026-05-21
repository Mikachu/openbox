#include "openbox/actions.h"
#include "openbox/focus.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_unfocus_startup(void)
{
    actions_register("Unfocus", NULL, NULL, run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    actions_interactive_cancel_act();
    focus_nothing();
    return FALSE;
}
