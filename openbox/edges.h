#ifndef __edges_h
#define __edges_h

#include "window.h"

typedef enum
{
    OB_EDGE_TOP,
    OB_EDGE_TOPRIGHT,
    OB_EDGE_RIGHT,
    OB_EDGE_BOTTOMRIGHT,
    OB_EDGE_BOTTOM,
    OB_EDGE_BOTTOMLEFT,
    OB_EDGE_LEFT,
    OB_EDGE_TOPLEFT,
    OB_NUM_EDGES
} ObEdgeLocation;

typedef struct _ObEdge ObEdge;

struct _ObEdge
{
    ObWindow obwin;
    Window win;
    ObEdgeLocation location;
};

void edges_startup(gboolean reconfigure);
void edges_shutdown(gboolean reconfigure);
void edges_configure(void);

#endif
