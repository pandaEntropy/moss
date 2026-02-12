#ifndef KEYS_H
#define KEYS_H

#include <X11/Xlib.h>

void grab_key(KeySym keysym, unsigned int mod);

void key_setup();

void handle_keypress(XKeyEvent *ev);

#endif
