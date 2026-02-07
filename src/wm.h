#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>

typedef struct{
    Window win;
}Client;

typedef struct{
    int i;
    const char **cparr;
}Arg;

typedef struct{
    KeySym keysym;
    unsigned int mod;
    void (*func)(const Arg *arg);
    Arg arg;
} Key;

typedef enum{
    MASTER_LEFT,
    MASTER_TOP,
    MASTER_RIGHT,
    MASTER_BOTTOM
} MasterPosition;

typedef enum{
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_UP
}Direction;

typedef enum{
    LAYOUT_MASTER,
    LAYOUT_HORIZONTAL
}LayoutMode;

extern Display *dpy;
extern Window root;
extern Client *focused;
extern Client *master;
extern Client clients[128];
extern int nclients;
extern int sw, sh;

void grab_key(KeySym keysym, unsigned int mod);

void OnMapRequest(XMapRequestEvent *ev);

void OnConfigureRequest(XConfigureRequestEvent *ev);

void OnKeyPress(XKeyEvent *ev);

void OnDestroyNotify(XDestroyWindowEvent *ev);

void manage(Window w);

void unmanage(Window w);

void focus(Client *c);

void focus_direction(const Arg *arg);

void unmap(const Arg *arg);

void kill_window(const Arg *arg);

void spawn(const Arg *arg);

void set_master(const Arg *arg);

Client* wintoclient(Window w);

#endif
