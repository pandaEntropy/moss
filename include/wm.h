#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>
#include <stdbool.h>

#include "forward.h"

#define PROTO_DELETE (1 << 0)
#define PROTO_TAKE_FOCUS (1 << 1)

struct Client{
    Window win;
    Client *next;
    bool floating;
    bool monocle;
    unsigned int protocols;
};

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

typedef enum{
    WIN_DIALOG,
    WIN_SPLASH,
    WIN_MENU,
    WIN_DOCK,
    WIN_NORMAL
}Wintype;

typedef struct{
    Atom net_wm_win_type;
    Atom net_wm_win_type_dock;
    Atom net_strut_partial;

    Atom net_wm_win_type_dialog;
    Atom net_wm_win_type_menu;
    Atom net_wm_win_type_splash;
    Atom net_wm_win_type_normal;

    Atom net_active_window;

    Atom wm_protocols;
    Atom wm_take_focus;
    Atom wm_delete_window;
 
}Atoms;

struct WM{
    Display *dpy;
    Window root;
    int sw;
    int sh;

    Client *focused;
    Client *master;
    Client *clients;
    int nclients;
    int usable_height;
    int usable_width;

    Atoms atoms;

    Dock docks[16];
    int ndocks;
};

void OnMapRequest(WM *wm, XMapRequestEvent *ev);

void OnConfigureRequest(WM *wm, XConfigureRequestEvent *ev);

void OnKeyPress(WM *wm, XKeyEvent *ev);

void OnDestroyNotify(WM *wm, XDestroyWindowEvent *ev);

void handle_XEvent(WM *wm, XEvent *ev);

void manage(WM *wm, Window w);

void unmanage(WM *wm, Window win);

void focus(WM *wm, Client *c);

void focus_direction(WM *wm, const Arg *arg);

void unmap(WM *wm, const Arg *arg);

void kill_window(WM *wm, const Arg *arg);

void set_master(WM *wm, const Arg *arg);

Client* wintoclient(WM *wm, Window win);

int has_wintype(int nitems, Atom *atoms, Atom type);

int get_strut(WM *wm, Dock *dock);

void recalc_usable_area(WM *wm);

int unmanage_dock(WM *wm, Window win);

Client* get_client(WM *wm, Window win); 

Wintype classify_window(WM *wm, Window win);

void handle_dock(WM *wm, Window win);

Window get_transient(WM *wm, Window win);

void set_protocols(WM *wm, Client *c);

void init_atoms(WM *wm);

#endif
