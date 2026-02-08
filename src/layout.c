#include "wm.h"
#include "layout.h"

#include <X11/Xlib.h>

LayoutMode tile_mode = LAYOUT_MASTER;
MasterPosition master_pos = MASTER_LEFT;
float mfactor = 0.5;
int nmaster = 1;

void tile(){
    switch(tile_mode){
        case LAYOUT_HORIZONTAL:
            horizontal_tile();
            break;
        case LAYOUT_MASTER:
            master_tile();
            break;
    }
}

void horizontal_tile(){
    for(int i = 0; i < nclients; i++){
        int h = sh;
        int w = sw / nclients;

        int x = w * i;
        int y = 0;

        XMoveResizeWindow(dpy, clients[i].win, x, y, w, h);
    }
}

void master_tile(){
    if(nclients == 0 || !master) return;

    if(nclients <= nmaster){
        XMoveResizeWindow(dpy, master->win, 0, 0, sw, sh);
        return;
    }

    int mw, mh, mx, my;
    int ww, wh, wx, wy;

    switch(master_pos){
        case MASTER_TOP:
            mw = sw;
            mh = sh * mfactor;
            mx = 0;
            my = 0;

            ww = sw / (nclients - nmaster);
            wh = sh - mh;
            wx = 0;
            wy = mh;
            break;

        case MASTER_RIGHT:
            mw = sw * mfactor;
            mh = sh;
            mx = sw - mw;
            my = 0;

            ww = sw - mw;
            wh = sh / (nclients - nmaster);
            wx = 0;
            wy = 0;
            break;

        case MASTER_BOTTOM:
            mw = sw;
            mh = sh * mfactor;
            mx = 0;
            my = sh - mh;

            ww = sw / (nclients - nmaster);
            wh = sh - mh;
            wx = 0;
            wy = 0;
            break;

        case MASTER_LEFT:
            mw = sw * mfactor;
            mh = sh;
            mx = 0;
            my = 0;

            ww = sw - mw;
            wh = sh / (nclients - nmaster);
            wx = mw;
            wy = 0;
            break;
    }

    XMoveResizeWindow(dpy, master->win, mx, my, mw, mh);

    for(int i = 0; i < nclients; i++){
        if(clients[i].win == master->win) continue;

        XMoveResizeWindow(dpy, clients[i].win, wx, wy, ww, wh);
        if(master_pos == MASTER_TOP || master_pos == MASTER_BOTTOM)
            wx += ww;
        else
            wy += wh;
    }
}

void resize(const Arg *arg){
    int dir = arg->i;
    float change = 0;
    
    if(nclients < 2)
        return;
    switch(master_pos){
        case MASTER_LEFT:
            if(dir == DIR_LEFT) change = -0.05;
            if(dir == DIR_RIGHT) change = 0.05;
            break;
        
        case MASTER_RIGHT:
            if(dir == DIR_LEFT) change = 0.05;
            if(dir == DIR_RIGHT) change = -0.05;
            break;

        case MASTER_TOP:
            if(dir == DIR_UP) change = -0.05;
            if(dir == DIR_DOWN) change = 0.05;
            break;

        case MASTER_BOTTOM:
            if(dir == DIR_UP) change = 0.05;
            if(dir == DIR_DOWN) change = -0.05;
            break;
    }
    if(mfactor + change > 1 || mfactor + change < 0)
        return;
    
    mfactor += change;
    tile();
}

void rotate(const Arg *arg){
    if(nclients < 2) return;

    int mode = arg->i;

    switch(mode){
        case LAYOUT_HORIZONTAL:
            horizontal_rotate();
            break;

        case LAYOUT_MASTER:
            master_rotate();
            break;
    }
}

void horizontal_rotate(){
    XWindowAttributes first_attr;
    XGetWindowAttributes(dpy, clients[0].win, &first_attr);

    for(int i = 0; i < nclients; i++){
        if(nclients-1 == i){
            XMoveWindow(dpy, clients[i].win, first_attr.x, first_attr.y);
            break;  
        }

        XWindowAttributes next_attr;
        XGetWindowAttributes(dpy, clients[i+1].win, &next_attr);

        XMoveWindow(dpy, clients[i].win, next_attr.x, next_attr.y);  
    }
}

void master_rotate(){
    //Cycle the enum
    master_pos = (master_pos+1) % 4;
    tile();
}

