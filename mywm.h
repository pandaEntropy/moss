#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>

typedef struct{
    int i; 
}Arg;

typedef struct{
    KeySym keysym;
    unsigned int mod;
    void (*func)(const Arg *arg);
    Arg arg;
} Key;

void grab_key(KeySym keysym, unsigned int mod);

void OnMapRequest(XMapRequestEvent *ev);

void OnConfigureRequest(XConfigureRequestEvent *ev);

void tile();

void Tile(const Arg *arg);

void manage(Window w);

void unmanage(Window w);

void focus(Window w);

void focus_direction(const Arg *arg);

void rotate(const Arg *arg);

void unmap(const Arg *arg);

#endif
