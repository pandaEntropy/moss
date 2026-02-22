#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>

#include "forward.h"

typedef struct{
    Window win;
}Client;

typedef struct{
    Window win;
    int left;
    int right;
    int top;
    int bottom;
}Dock;

typedef struct{
    int i;
    const char **cparr;
}Arg;

typedef enum{
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_UP
}Direction;

struct WM{
    Display *dpy;
    Window root;
    int sw;
    int sh;

    Client *focused;
    Client *master;
    Client clients[128];
    int nclients;
    int usable_height;
    int usable_width;

    Atom net_win_type;
    Atom net_win_type_dock;
    Atom net_strut_partial;

    Dock docks[16];
    int ndocks;
};

void OnMapRequest(WM *wm, XMapRequestEvent *ev);

void OnConfigureRequest(WM *wm, XConfigureRequestEvent *ev);

void OnKeyPress(WM *wm, XKeyEvent *ev);

void OnDestroyNotify(WM *wm, XDestroyWindowEvent *ev);

void handle_XEvent(WM *wm, XEvent *ev);

void manage(WM *wm, Window w);

void unmanage(WM *wm, Window w);

void focus(WM *wm, Client *c);

void focus_direction(WM *wm, const Arg *arg);

void unmap(WM *wm, const Arg *arg);

void kill_window(WM *wm, const Arg *arg);

void set_master(WM *wm, const Arg *arg);

Client* wintoclient(WM *wm, Window w);

int is_dock(WM *wm, Window win);

int get_strut(WM *wm, Dock *dock);

void recalc_usable_area(WM *wm);

int unmanage_dock(WM *wm, Window win);
#endif
