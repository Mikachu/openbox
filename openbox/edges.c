#include "openbox.h"
#include "config.h"
#include "screen.h"
#include "edges.h"

#include <X11/Xlib.h>
#include <glib.h>

#warning Do something clever with xinerama
ObEdge *edge[OB_NUM_EDGES];
#warning put in config.c and parse configs of course
gboolean config_edge_enabled[OB_NUM_EDGES] = {1, 1, 1, 1, 1, 1, 1, 1};

#define EDGE_WIDTH 10
#define CORNER_SIZE 20
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
                     EDGE_WIDTH, screen.height - CORNER_SIZE);
            break;
        case OB_EDGE_TOPLEFT:
            RECT_SET(*rect, 0, 0, CORNER_SIZE, CORNER_SIZE);
            break;
    }
}

void edges_startup(gboolean reconfigure)
{
    gint i;
    Rect r;
    XSetWindowAttributes xswa;
    Rect *screen = screen_physical_area_all_monitors();

    xswa.override_redirect = True;

    for (i=0; i < OB_NUM_EDGES; i++) {
        if (!config_edge_enabled[i])
            continue;

        edge[i] = g_new(ObEdge, 1);
        edge[i]->obwin.type = OB_WINDOW_CLASS_INTERNAL;

        get_position(i, *screen, &r);
        edge[i]->win = XCreateWindow(obt_display, RootWindow(obt_display, ob_screen),
                                     r.x, r.y, r.width, r.height, 0, 0, InputOnly,
                                     CopyFromParent, CWOverrideRedirect, &xswa);
        XMapWindow(obt_display, edge[i]->win);

        stacking_add(INTERNAL_AS_WINDOW(edge[i]));
        window_add(&edge[i]->win, INTERNAL_AS_WINDOW(edge[i]));

        printf("mapped edge window %i at %03i %03i %02i %02i\n", i, r.x, r.y, r.width, r.height);
        fflush(stdout);
    }

    XFlush(obt_display);

    g_free(screen);
}

void edges_shutdown(gboolean reconfigure)
{
    gint i;

    for (i=0; i < OB_NUM_EDGES; i++) {
        window_remove(&edge[i]->win);
        stacking_remove(INTERNAL_AS_WINDOW(edge[i]));
        XDestroyWindow(obt_display, edge[i]->win);
        g_free(edge[i]);
    }
}
