#include "mywm.h"
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <limits.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define DIR_LEFT 1
#define DIR_DOWN 2
#define DIR_RIGHT 3
#define DIR_UP 4

Display* dpy;
int sw; //screen width
int sh; //screen height
int screen;  
XWindowAttributes attr;
XButtonEvent start;
XEvent ev;
Window root;
Window focused = None;
Window clients[128];
int nclients = 0;
int nkeys = 0;

//TODO -> Maybe just dump the whole arg thing and hard code if's into OnKeyPress
//Dont forget, structs are in the header
Key keys[] = {
    {XK_r, Mod1Mask, Tile, {0}},
    {XK_l, Mod1Mask, focus_direction, {.i = 3}},
    {XK_h, Mod1Mask, focus_direction, {.i = 1}}
};

void grab_key(KeySym keysym, unsigned int mod){
    KeyCode code = XKeysymToKeycode(dpy, keysym);
    XGrabKey(dpy, code, mod, DefaultRootWindow(dpy), false, GrabModeAsync, GrabModeAsync);
}

void OnMapRequest(XMapRequestEvent* ev){
    XMapWindow(dpy, ev->window);
    manage(ev->window);
}

void OnConfigureRequest(XConfigureRequestEvent* ev){
    XWindowChanges changes;

    changes.x = ev->x;
    changes.y = ev->y;
    changes.width = ev->width;
    changes.height = ev->height;
    changes.border_width = ev->border_width;
    changes.sibling = ev->above;
    changes.stack_mode = ev->detail;

    XConfigureWindow(dpy, ev->window, ev->value_mask, &changes);
}

void OnDestroyNotify(XDestroyWindowEvent* ev){
    unmanage(ev->window);        
}

void OnKeyPress(XKeyEvent* ev){
    nkeys = sizeof(keys) / sizeof(keys[0]);

    for(int i = 0; i < nkeys; i++){
        if(ev->keycode == XKeysymToKeycode(dpy, keys[i].keysym)
                && (ev->state & keys[i].mod)){
            keys[i].func(&keys[i].arg);
        }                
    }
}

//Tile for keypress commands
void Tile(const Arg *arg){
    (void)arg;
    tile();    
}

//tile for logic and basic usage
void tile(){
for(int i = 0; i < nclients; i++){
        int h = sh;
        int w = sw / nclients;

        int x = w * i;
        int y = 0;

        XMoveResizeWindow(dpy, clients[i], x, y, w, h);
        printf("Window %d: x%d, width%d\n", i, x, w);
    }
}

void focus_direction(const Arg *arg) {
    int dir = arg->i;
    if (!focused) return;

    Window best = None;
    int best_dist = INT_MAX;

    XWindowAttributes fa;
    XGetWindowAttributes(dpy, focused, &fa);

    //get the center of the focused window
    int fx = fa.x + fa.width / 2;
    int fy = fa.y + fa.height / 2;

    for (int i = 0; i < nclients; i++) {
        Window w = clients[i];
        if (w == focused) continue;

        XWindowAttributes wa;
        XGetWindowAttributes(dpy, w, &wa);

        //get the center of the window that we are comparing
        int wx = wa.x + wa.width / 2;
        int wy = wa.y + wa.height / 2;

        //distance from the focused window center to the other window center
        int dx = wx - fx;
        int dy = wy - fy;

        bool valid = false;
        int dist = 0;

        //evaluate each calculated distance based on direction
        switch (dir) {
            case DIR_LEFT:  valid = dx < 0; dist = -dx; break;
            case DIR_RIGHT: valid = dx > 0; dist = dx; break;
            case DIR_UP:    valid = dy < 0; dist = -dy; break;
            case DIR_DOWN:  valid = dy > 0; dist = dy; break;
        }

        //get the records
        if (valid && dist < best_dist) {
            best = w;
            best_dist = dist;
        }
    }

    if (best != None)
        focus(best);
}


void manage(Window w){
    clients[nclients] = w;
    nclients++;        
    tile();
    focus(w);
}

void unmanage(Window w){
    for(int i = 0; i < nclients; i++){
        if(clients[i] == w){
            if(nclients > 1) focus(clients[i-1]);
            else focus(root);

            for(int j = i; j < nclients-1; j++){
                clients[j] = clients[j+1];    
            }
            nclients--;
            break;
        }
    }
    tile();
}

void focus(Window w){
    focused = w;
    XSetInputFocus(dpy, w, RevertToParent, CurrentTime);
}
 


int main(void)
{
    
    if(!(dpy = XOpenDisplay(0x0))) return 1;
    
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = DefaultRootWindow(dpy);

    XSetInputFocus(dpy, None, RevertToParent, CurrentTime);

    XSelectInput(dpy, root, SubstructureRedirectMask | 
            SubstructureNotifyMask |
            EnterWindowMask |
            LeaveWindowMask |
            FocusChangeMask);
    
    grab_key(XK_r, Mod1Mask);
    grab_key(XK_l, Mod1Mask);
    grab_key(XK_h, Mod1Mask);
    XGrabButton(dpy, 1, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    
    //Create and define the cursor
    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, DefaultRootWindow(dpy), cursor);

    XSync(dpy, false);
    start.subwindow = None;
    for(;;)
    {
        XNextEvent(dpy, &ev);
        //printf("event: %d\n", ev.type);
        switch(ev.type){    
            case KeyPress:
                OnKeyPress(&ev.xkey);
                break;

            case ButtonPress:
                if(ev.xbutton.subwindow != None){
                    //XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
                    //start = ev.xbutton;
                    focus(ev.xbutton.subwindow);
                }
                break;

            case MotionNotify:
               /*
                if(start.subwindow != None){
                    int xdiff = ev.xbutton.x_root - start.x_root;
                    int ydiff = ev.xbutton.y_root - start.y_root;
                    XMoveResizeWindow(dpy, start.subwindow,
                        attr.x + (start.button==1 ? xdiff : 0),
                        attr.y + (start.button==1 ? ydiff : 0),
                        MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                        MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
                }
                */
                break;

            case ButtonRelease:
                start.subwindow = None;
                break;

            case ConfigureRequest:
                OnConfigureRequest(&ev.xconfigurerequest);
                break;    

            case MapRequest:
                OnMapRequest(&ev.xmaprequest);
                break;

            case DestroyNotify:
                OnDestroyNotify(&ev.xdestroywindow);
                break;
        }
    }
    XCloseDisplay(dpy);
}


