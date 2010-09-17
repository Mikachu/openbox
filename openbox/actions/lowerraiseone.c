#include "openbox/actions.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/client.h"

static gboolean run_func(ObActionsData *data, gpointer options);
static gpointer setup_raise(xmlNodePtr node);
static gpointer setup_lower(xmlNodePtr node);

void action_lowerraiseone_startup(void)
{
    actions_register("RaiseOne",
                     setup_raise, NULL,
                     run_func);
    actions_register("LowerOne",
                     setup_lower, NULL,
                     run_func);
}

static gpointer setup_raise(xmlNodePtr node)
{
    return GINT_TO_POINTER(1);
}

static gpointer setup_lower(xmlNodePtr node)
{
    return GINT_TO_POINTER(0);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    gint dir = GPOINTER_TO_INT(options) ? 1 : -1;

    if (data->client) {
        ObClient *client = data->client;
        ObClient *sibling;
        actions_client_move(data, TRUE);
        if (dir == 1) {
          sibling = stacking_occluded(client, NULL);
          if (sibling)
              stacking_above(client, sibling);
        } else {
          sibling = stacking_occludes(client, NULL);
          if (sibling)
              stacking_below(client, sibling);
        }
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
