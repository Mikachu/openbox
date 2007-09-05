#ifndef __edges_h
#define __edges_h

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
} ObEdge;

void edges_startup(gboolean reconfigure);
void edges_shutdown(gboolean reconfigure);

#endif
