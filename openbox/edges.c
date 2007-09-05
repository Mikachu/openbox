#include "openbox.h"
#include "config.h"
#include "screen.h"
#include "edges.h"

#include <X11/Xlib.h>
#include <glib.h>

#warning Do something clever with xinerama
Window edge[OB_NUM_EDGES];
#warning put in config.c and parse configs of course
gboolean config_edge_enabled[OB_NUM_EDGES] = {1, 1, 1, 1, 1, 1, 1, 1};

#define EDGE_WIDTH 10
#define CORNER_SIZE 20
static void get_position(ObEdge edge, Rect screen, Rect *rect)
{
    switch (edge) {
        case OB_EDGE_TOP:
            rect->x = CORNER_SIZE;
            rect->y = 0;
            rect->width = screen.width - 2 * CORNER_SIZE;
            rect->height = EDGE_WIDTH;
            break;
        case OB_EDGE_TOPRIGHT:
            rect->x = screen.width - CORNER_SIZE;
            rect->y = 0;
            rect->width = CORNER_SIZE;
            rect->height = CORNER_SIZE;
            break;
        case OB_EDGE_RIGHT:
            rect->x = screen.width - EDGE_WIDTH;
            rect->y = CORNER_SIZE;
            rect->width = EDGE_WIDTH;
            rect->height = screen.height - 2 * CORNER_SIZE;
            break;
        case OB_EDGE_BOTTOMRIGHT:
            rect->x = screen.width - CORNER_SIZE;
            rect->y = screen.height - CORNER_SIZE;
            rect->width = CORNER_SIZE;
            rect->height = CORNER_SIZE;
            break;
        case OB_EDGE_BOTTOM:
            rect->x = CORNER_SIZE;
            rect->y = screen.height - EDGE_WIDTH;
            rect->width = screen.width - 2 * CORNER_SIZE;
            rect->height = EDGE_WIDTH;
            break;
        case OB_EDGE_BOTTOMLEFT:
            rect->x = 0;
            rect->y = screen.height - CORNER_SIZE;
            rect->width = CORNER_SIZE;
            rect->height = CORNER_SIZE;
            break;
        case OB_EDGE_LEFT:
            rect->x = 0;
            rect->y = CORNER_SIZE;
            rect->width = EDGE_WIDTH;
            rect->height = screen.height - CORNER_SIZE;
            break;
        case OB_EDGE_TOPLEFT:
            rect->x = 0;
            rect->y = 0;
            rect->width = CORNER_SIZE;
            rect->height = CORNER_SIZE;
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

#warning insert into window_map and make an obwindow type
/*    g_hash_table_insert(window_map, &dock->frame, dock); */

    for (i=0; i < OB_NUM_EDGES; i++) {
        if (!config_edge_enabled[i])
            continue;
        get_position(i, *screen, &r);
        edge[i] = XCreateWindow(obt_display, RootWindow(obt_display, ob_screen),
                                r.x, r.y, r.width, r.height, 0, 0, InputOnly,
                                CopyFromParent, CWOverrideRedirect, &xswa);
        XMapWindow(obt_display, edge[i]);
        printf("mapped edge window %i at %03i %03i %02i %02i\n", i, r.x, r.y, r.width, r.height);
        fflush(stdout);
    }

    XFlush(obt_display);

    g_free(screen);
}

void edges_shutdown(gboolean reconfigure)
{
    gint i;

    for (i=0; i < OB_NUM_EDGES; i++)
        XDestroyWindow(obt_display, edge[i]);
}
