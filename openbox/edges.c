#include "openbox.h"
#include "config.h"
#include "screen.h"
#include "edges.h"
#include "frame.h"

#include <X11/Xlib.h>
#include <glib.h>

/* Array of array of monitors of edges: edge[monitor 2][top edge] */
ObEdge ***edge = NULL;
#warning put in config.c and parse configs of course
gboolean config_edge_enabled[OB_NUM_EDGES] = {1, 1, 1, 1, 1, 1, 1, 1};
/* this could change at runtime, we should hook into that, but for now
 * don't crash on reconfigure/shutdown */
static guint edge_monitors;

#ifdef DEBUG
#define EDGE_WIDTH 10
#define CORNER_SIZE 20
#else
#define EDGE_WIDTH 1
#define CORNER_SIZE 2
#endif
static void get_position(ObEdgeLocation edge, Rect screen, Rect *rect)
{
    switch (edge) {
        case OB_EDGE_TOP:
            RECT_SET(*rect, CORNER_SIZE, 0,
                     screen.width - 2 * CORNER_SIZE, EDGE_WIDTH);
            break;
        case OB_EDGE_TOPRIGHT:
            RECT_SET(*rect, screen.width - CORNER_SIZE, 0,
                     CORNER_SIZE, CORNER_SIZE);
            break;
        case OB_EDGE_RIGHT:
            RECT_SET(*rect, screen.width - EDGE_WIDTH, CORNER_SIZE,
                     EDGE_WIDTH, screen.height - 2 * CORNER_SIZE);
            break;
        case OB_EDGE_BOTTOMRIGHT:
            RECT_SET(*rect, screen.width - CORNER_SIZE,
                     screen.height - CORNER_SIZE,
                     CORNER_SIZE, CORNER_SIZE);
            break;
        case OB_EDGE_BOTTOM:
            RECT_SET(*rect, CORNER_SIZE, screen.height - EDGE_WIDTH,
                     screen.width - 2 * CORNER_SIZE, EDGE_WIDTH);
            break;
        case OB_EDGE_BOTTOMLEFT:
            RECT_SET(*rect, 0, screen.height - CORNER_SIZE,
                     CORNER_SIZE, CORNER_SIZE);
            break;
        case OB_EDGE_LEFT:
            RECT_SET(*rect, 0, CORNER_SIZE,
                     EDGE_WIDTH, screen.height - 2 * CORNER_SIZE);
            break;
        case OB_EDGE_TOPLEFT:
            RECT_SET(*rect, 0, 0, CORNER_SIZE, CORNER_SIZE);
            break;
    }
    rect->x += screen.x;
    rect->y += screen.y;
}

void edges_startup(gboolean reconfigure)
{
    ObEdgeLocation i;
    gint m;
    Rect r;
    XSetWindowAttributes xswa;

    xswa.override_redirect = True;

    edge_monitors = screen_num_monitors;

    edge = g_slice_alloc(sizeof(ObEdge**) * edge_monitors);
    for (m = 0; m < edge_monitors; m++) {
        const Rect *monitor = screen_physical_area_monitor(m);
        edge[m] = g_slice_alloc(sizeof(ObEdge*) * OB_NUM_EDGES);
        for (i=0; i < OB_NUM_EDGES; i++) {
            if (!config_edge_enabled[i])
                continue;

            edge[m][i] = g_slice_new(ObEdge);
            edge[m][i]->obwin.type = OB_WINDOW_CLASS_EDGE;
            edge[m][i]->location = i;

            get_position(i, *monitor, &r);
            edge[m][i]->win = XCreateWindow(obt_display, obt_root(ob_screen),
                                         r.x, r.y, r.width, r.height, 0, 0, InputOnly,
                                         CopyFromParent, CWOverrideRedirect, &xswa);
            XSelectInput(obt_display, edge[m][i]->win, ButtonPressMask | ButtonReleaseMask
                         | EnterWindowMask | LeaveWindowMask);
            XMapWindow(obt_display, edge[m][i]->win);

            stacking_add(EDGE_AS_WINDOW(edge[m][i]));
            window_add(&edge[m][i]->win, EDGE_AS_WINDOW(edge[m][i]));

#ifdef DEBUG
            ob_debug("mapped edge window %i at %03i %03i %02i %02i", i, r.x, r.y, r.width, r.height);
#endif
        }
    }

    XFlush(obt_display);
}

void edges_shutdown(gboolean reconfigure)
{
    gint i, m;

    /* This is in case we get called before startup by screen_resize() */
    if (!edge)
        return;

    for (m = 0; m < edge_monitors; m++) {
        for (i = 0; i < OB_NUM_EDGES; i++) {
            if (!config_edge_enabled[i])
                continue;

            window_remove(edge[m][i]->win);
            stacking_remove(EDGE_AS_WINDOW(edge[m][i]));
            XDestroyWindow(obt_display, edge[m][i]->win);
            g_slice_free(ObEdge, edge[m][i]);
        }
        g_slice_free1(sizeof(ObEdge*) * OB_NUM_EDGES, edge[m]);
    }
    g_slice_free1(sizeof(ObEdge**) * edge_monitors, edge);
}

void edges_configure()
{
    edges_shutdown(TRUE);
    edges_startup(TRUE);
}
