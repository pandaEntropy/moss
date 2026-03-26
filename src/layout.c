#include "wm.h"
#include "layout.h"

#include <X11/Xlib.h>
#include <stdio.h>

MasterPosition master_pos = MASTER_LEFT;
float mfactor = 0.5;
int nmaster = 1;
int ntiled = 0;

int border_width = 4;


void moveresize_window(WM *wm, Client *c, unsigned int width, unsigned int height, int x, int y);

Layout master_layout(){
    return (Layout){.id = LAYOUT_MASTER, .tile = master_tile, .rotate = master_rotate, .focus = focus_direction};
}

Layout horizontal_layout(){
    return (Layout){.id = LAYOUT_HORIZONTAL, .tile = horizontal_tile, .rotate = horizontal_rotate, .focus = focus_direction};
}

Layout monocle_layout(){
    return (Layout){.id = LAYOUT_MONOCLE, .tile = monocle_tile, .rotate = monocle_rotate, .focus = monocle_focus};
}

void tile(WM *wm){
    ntiled = 0;

    for(Client *c = wm->clients; c; c = c->next){
        if(!c->floating)
            ntiled++;
    }

    wm->layouts[wm->active_layout].tile(wm);
}

void horizontal_tile(WM *wm){
    int i = 0;
    for(Client *c = wm->clients; c; c = c->next){
        int h = wm->usable_height;
        int w = wm->usable_width / ntiled;

        int x = w * i;
        int y = 0;

        moveresize_window(wm, c, w, h, x + wm->usable_x, y + wm->usable_y);
        i++;
    }
}

void master_tile(WM *wm){
    if(ntiled == 0 || !wm->master) return;

    if(ntiled <= nmaster){
        moveresize_window(wm, wm->master, wm->usable_width, wm->usable_height, wm->usable_x, wm->usable_y);
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

    moveresize_window(wm, wm->master, mw, mh, mx + wm->usable_x, my + wm->usable_y);

    for(Client *c = wm->clients; c; c = c->next){
        if(c == wm->master || c->floating) continue;

        moveresize_window(wm, c, ww, wh, wx + wm->usable_x, wy + wm->usable_y);
        if(master_pos == MASTER_TOP || master_pos == MASTER_BOTTOM)
            wx += ww;
        else
            wy += wh;
    }
}

void monocle_tile(WM *wm){
    if(!wm->clients) return;

    for(Client *c = wm->clients; c; c = c->next){
        if(c->floating) continue;

        moveresize_window(wm, c, wm->usable_width, wm->usable_height, wm->usable_x, wm->usable_y);
    }

    if(wm->focused){
        XRaiseWindow(wm->dpy, wm->focused->parent);
        XRaiseWindow(wm->dpy, wm->focused->win);
    }
}

void resize(WM *wm, const Arg *arg){
    if(wm->layouts[wm->active_layout].id == LAYOUT_MONOCLE) return;

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
    (void)arg;
    if(ntiled < 2) return;

    wm->layouts[wm->active_layout].rotate(wm);
}

void horizontal_rotate(WM *wm){
    if(!wm->clients) return;

    XWindowAttributes first_attr;
    XGetWindowAttributes(wm->dpy, wm->clients->parent, &first_attr);

    for(Client *c = wm->clients; c; c = c->next){
        if(c->next == NULL){
            moveresize_window(wm, c, first_attr.width, first_attr.height, first_attr.x, first_attr.y);
            break;
        }

        XWindowAttributes next_attr;
        XGetWindowAttributes(wm->dpy, c->next->parent, &next_attr);

        moveresize_window(wm, c, next_attr.width, next_attr.height, next_attr.x, next_attr.y);
    }
}

void master_rotate(WM *wm){
    //Cycle the enum
    master_pos = (master_pos+1) % 4;
    tile(wm);
}

void monocle_rotate(WM *wm){
    (void)wm;
}

void parent_center(WM *wm, Window parent, Client *c){
    XWindowAttributes pattr;
    XWindowAttributes cattr;

    XGetWindowAttributes(wm->dpy, parent, &pattr);
    XGetWindowAttributes(wm->dpy, c->win, &cattr);

    int x = (pattr.width - cattr.width) / 2;
    int y = (pattr.height - cattr.height) / 2;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    moveresize_window(wm, c, cattr.width, cattr.height, x, y);
}

void screen_center(WM *wm, Client *c){
    XWindowAttributes cattr;

    XGetWindowAttributes(wm->dpy, c->win, &cattr);

    int x = (wm->usable_width - cattr.width) / 2;
    int y = (wm->usable_height - cattr.height) / 2;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    moveresize_window(wm, c, cattr.width, cattr.height, x, y);
}

void moveresize_window(WM *wm, Client *c, unsigned int width, unsigned int height, int x, int y){
    if(c->parent){
        XMoveResizeWindow(wm->dpy, c->parent, x, y, width - 2 * border_width, height - 2 * border_width);
        XMoveResizeWindow(wm->dpy, c->win, 0, 0, width - 2 * border_width, height - 2 * border_width);
    }
    else
        XMoveResizeWindow(wm->dpy, c->win, x, y, width, height);


    send_conf_req(wm, c, width, height, x, y);
}
