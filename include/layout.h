#ifndef LAYOUT_H
#define LAYOUT_H

#include "wm.h"
#include "forward.h"

typedef enum{
    MASTER_LEFT,
    MASTER_TOP,
    MASTER_RIGHT,
    MASTER_BOTTOM
} MasterPosition;

typedef enum{
    LAYOUT_MASTER,
    LAYOUT_HORIZONTAL
}LayoutMode;

void horizontal_tile(WM *wm);

void tile(WM *wm);

void master_tile(WM *wm);

void rotate(WM *wm, const Arg *arg);

void master_rotate(WM *wm);

void horizontal_rotate(WM *wm);

void resize(WM *wm, const Arg *arg);

#endif
