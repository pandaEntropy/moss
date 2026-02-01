#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>

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
    MASTER_BOTTOM,
    MASTER_RIGHT,
    MASTER_TOP
} MasterPosition;

void grab_key(KeySym keysym, unsigned int mod);

void OnMapRequest(XMapRequestEvent *ev);

void OnConfigureRequest(XConfigureRequestEvent *ev);

void horizontal_tile();

void tile(int mode);

void master_tile();

void manage(Window w);

void unmanage(Window w);

void focus(Window w);

void focus_direction(const Arg *arg);

void rotate(const Arg *arg);

void unmap(const Arg *arg);

void kill_window(const Arg *arg);

void spawn(const Arg *arg);

void master_rotate();

void horizontal_rotate();

void resize(const Arg *arg);

void set_master(const Arg *arg);

#endif
