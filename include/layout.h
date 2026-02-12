#ifndef LAYOUT_H
#define LAYOUT_H

#include "wm.h"

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

void horizontal_tile();

void tile();

void master_tile();

void rotate(const Arg *arg);

void master_rotate();

void horizontal_rotate();

void resize(const Arg *arg);

#endif
