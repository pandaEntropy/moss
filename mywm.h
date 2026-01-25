#ifndef MYWM_H
#define MYWM_H

#include <X11/Xlib.h>

void grabKey(char* key, unsigned int mod);

void OnMapRequest(XMapRequestEvent* ev);

void OnConfigureRequest(XConfigureRequestEvent* ev);

void Tile();

void manage(Window w);

void unmanage(Window w);

#endif
