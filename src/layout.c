#include "wm.h"
#include "layout.h"

#include <X11/Xlib.h>

LayoutMode tile_mode = LAYOUT_MASTER;
MasterPosition master_pos = MASTER_LEFT;
float mfactor = 0.5;
int nmaster = 1;
int ntiled = 0;

void tile(WM *wm){
    ntiled = 0;

    for(Client *c = wm->clients; c; c = c->next){
        if(!c->floating)
            ntiled++;
    }

    switch(tile_mode){
        case LAYOUT_HORIZONTAL:
            horizontal_tile(wm);
            break;
        case LAYOUT_MASTER:
            master_tile(wm);
            break;
    }
}

void horizontal_tile(WM *wm){
    int i = 0;
    for(Client *c = wm->clients; c; c = c->next){
        int h = wm->usable_height;
        int w = wm->usable_width / ntiled;

        int x = w * i;
        int y = 0;

        XMoveResizeWindow(wm->dpy, c->win, x, y, w, h);
        i++;
    }
}

void master_tile(WM *wm){
    if(ntiled == 0 || !wm->master) return;

    if(ntiled <= nmaster){
        XMoveResizeWindow(wm->dpy, wm->master->win, 0, 0, wm->usable_width, wm->usable_height);
        return;
    }

    int mw, mh, mx, my;
    int ww, wh, wx, wy;

    switch(master_pos){
        case MASTER_TOP:
            mw = wm->usable_width;
            mh = wm->usable_height * mfactor;
            mx = 0;
            my = 0;

            ww = wm->usable_width / (ntiled - nmaster);
            wh = wm->usable_height - mh;
            wx = 0;
            wy = mh;
            break;

        case MASTER_RIGHT:
            mw = wm->usable_width * mfactor;
            mh = wm->usable_height;
            mx = wm->usable_width - mw;
            my = 0;

            ww = wm->usable_width - mw;
            wh = wm->usable_height / (ntiled - nmaster);
            wx = 0;
            wy = 0;
            break;

        case MASTER_BOTTOM:
            mw = wm->usable_width;
            mh = wm->usable_height * mfactor;
            mx = 0;
            my = wm->usable_height - mh;

            ww = wm->usable_width / (ntiled - nmaster);
            wh = wm->usable_height - mh;
            wx = 0;
            wy = 0;
            break;

        case MASTER_LEFT:
            mw = wm->usable_width * mfactor;
            mh = wm->usable_height;
            mx = 0;
            my = 0;

            ww = wm->usable_width - mw;
            wh = wm->usable_height / (ntiled - nmaster);
            wx = mw;
            wy = 0;
            break;
    }

    XMoveResizeWindow(wm->dpy, wm->master->win, mx, my, mw, mh);

    for(Client *c = wm->clients; c; c = c->next){
        if(c == wm->master || c->floating) continue;

        XMoveResizeWindow(wm->dpy, c->win, wx, wy, ww, wh);
        if(master_pos == MASTER_TOP || master_pos == MASTER_BOTTOM)
            wx += ww;
        else
            wy += wh;
    }
}

void resize(WM *wm, const Arg *arg){
    int dir = arg->i;
    float change = 0;

    if(ntiled < 2)
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
    if(mfactor + change > 0.95 || mfactor + change < 0.05)
        return;

    mfactor += change;
    tile(wm);
}

void rotate(WM *wm, const Arg *arg){
    if(ntiled < 2) return;

    int mode = arg->i;

    switch(mode){
        case LAYOUT_HORIZONTAL:
            horizontal_rotate(wm);
            break;

        case LAYOUT_MASTER:
            master_rotate(wm);
            break;
    }
}

void horizontal_rotate(WM *wm){
    if(!wm->clients) return;

    XWindowAttributes first_attr;
    XGetWindowAttributes(wm->dpy, wm->clients->win, &first_attr);

    for(Client *c = wm->clients; c; c = c->next){
        if(c->next == NULL){
            XMoveWindow(wm->dpy, c->win, first_attr.x, first_attr.y);
            break;  
        }

        XWindowAttributes next_attr;
        XGetWindowAttributes(wm->dpy, c->next->win, &next_attr);

        XMoveWindow(wm->dpy, c->win, next_attr.x, next_attr.y);  
    }
}

void master_rotate(WM *wm){
    //Cycle the enum
    master_pos = (master_pos+1) % 4;
    tile(wm);
}

void parent_center(WM *wm, Window parent, Window child){
    XWindowAttributes pattr;
    XWindowAttributes cattr;

    XGetWindowAttributes(wm->dpy, parent, &pattr);
    XGetWindowAttributes(wm->dpy, child, &cattr);

    int x = pattr.x + (pattr.width - cattr.width) / 2;
    int y = pattr.y + (pattr.height - cattr.height) / 2;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    XMoveWindow(wm->dpy, child, x, y);
}

void screen_center(WM *wm, Window child){
    XWindowAttributes cattr;

    XGetWindowAttributes(wm->dpy, child, &cattr);

    int x = (wm->usable_width - cattr.width) / 2;
    int y = (wm->usable_height - cattr.height) / 2;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    XMoveWindow(wm->dpy, child, x, y);
}

