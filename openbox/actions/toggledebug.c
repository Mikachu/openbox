#include "openbox/actions.h"
#include "openbox/stacking.h"
#include "openbox/debug.h"

static gboolean run_func(ObActionsData *data, gpointer options);
extern gboolean debug_show;

void action_toggledebug_startup(void)
{
    actions_register("toggledebug",
                     NULL, NULL,
                     run_func,
                     NULL, NULL);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    if (debug_show) ob_debug("Debug output turned %s\n", "off");
    ob_debug_show_output(!debug_show);
    if (debug_show) ob_debug("Debug output turned %s\n", "on");

    return FALSE;
}
