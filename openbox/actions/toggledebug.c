#include "openbox/actions.h"
#include "openbox/stacking.h"

static gboolean run_func(ObActionsData *data, gpointer options);
extern gboolean debug_show;

void action_toggledebug_startup()
{
    actions_register("toggledebug",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    ob_debug_show_output(!debug_show);
    g_message("Debug output turned %s", debug_show ? "on" : "off");

    return FALSE;
}
