#include "mywm.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

/*
TODO:
1. Add master switching
*/

#define DIR_LEFT 1
#define DIR_DOWN 2
#define DIR_RIGHT 3
#define DIR_UP 4

#define HORIZONTAL 1
#define MASTER 2

int tile_mode = MASTER;
static MasterPosition master_pos = MASTER_LEFT;

Display* dpy;
int sw; //screen width
int sh; //screen height
int screen;  
XEvent ev;
Window root;
Window focused = None;
Window master = None;
float mfactor = 0.5;
int nmaster = 1;
bool master_stack = true;
bool horizontal_mode = false;
Window clients[128];
int nclients = 0;
bool subwin_unmapped = false;

static const char *termcmd[] = {"xterm", NULL};

Key keys[] = {
    {XK_r, Mod1Mask, rotate, {.i = MASTER}},
    {XK_l, Mod1Mask, focus_direction, {.i = DIR_RIGHT}},
    {XK_h, Mod1Mask, focus_direction, {.i = DIR_LEFT}},
    {XK_d, Mod1Mask, unmap, {0}},
    {XK_k, Mod1Mask, kill_window, {0}},
    {XK_Return, Mod1Mask, spawn, {.cparr = termcmd}},
    {XK_l, Mod1Mask | ControlMask, resize, {.i = DIR_RIGHT}},
    {XK_h, Mod1Mask | ControlMask, resize, {.i = DIR_LEFT}},
    {XK_k, Mod1Mask | ControlMask, resize, {.i = DIR_UP}},
    {XK_j, Mod1Mask | ControlMask, resize, {.i = DIR_DOWN}}};

void grab_key(KeySym keysym, unsigned int mod){
    KeyCode code = XKeysymToKeycode(dpy, keysym);
    XGrabKey(dpy, code, mod, DefaultRootWindow(dpy), False, GrabModeAsync, GrabModeAsync);
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

void OnDestroyNotify(XDestroyWindowEvent *ev){
    unmanage(ev->window);        
}

void OnKeyPress(XKeyEvent *ev){
    int nkeys = sizeof(keys) / sizeof(keys[0]);

    for(int i = 0; i < nkeys; i++){
        if(ev->keycode == XKeysymToKeycode(dpy, keys[i].keysym) && ev->state == keys[i].mod){
            keys[i].func(&keys[i].arg);
            return;
        }
    }
}

void tile(int mode){
    switch(mode){
        case HORIZONTAL:
            horizontal_tile();
            break;
        case MASTER:
            master_tile();
            break;
    }
}

void horizontal_tile(){
    for(int i = 0; i < nclients; i++){
        int h = sh;
        int w = sw / nclients;

        int x = w * i;
        int y = 0;

        XMoveResizeWindow(dpy, clients[i], x, y, w, h);
    }
}

void master_tile(){
    if(nclients == 0) return;

    if(nclients <= nmaster){
        XMoveResizeWindow(dpy, master, 0, 0, sw, sh);
        return;
    }

    int mw, mh, mx, my;
    int ww, wh, wx, wy;

    switch(master_pos){
        case MASTER_TOP:
            mw = sw;
            mh = sh * mfactor;
            mx = 0;
            my = 0;

            ww = sw / (nclients - nmaster);
            wh = sh - mh;
            wx = 0;
            wy = mh;
            break;

        case MASTER_RIGHT:
            mw = sw * mfactor;
            mh = sh;
            mx = sw - mw;
            my = 0;

            ww = sw - mw;
            wh = sh / (nclients - nmaster);
            wx = 0;
            wy = 0;
            break;

        case MASTER_BOTTOM:
            mw = sw;
            mh = sh * mfactor;
            mx = 0;
            my = sh - mh;

            ww = sw / (nclients - nmaster);
            wh = sh - mh;
            wx = 0;
            wy = 0;
            break;

        case MASTER_LEFT:
            mw = sw * mfactor;
            mh = sh;
            mx = 0;
            my = 0;

            ww = sw - mw;
            wh = sh / (nclients - nmaster);
            wx = mw;
            wy = 0;
            break;
    }

    XMoveResizeWindow(dpy, master, mx, my, mw, mh);

    for(int i = 0; i < nclients; i++){
        if(clients[i] == master) continue;

        XMoveResizeWindow(dpy, clients[i], wx, wy, ww, wh);
        if(master_pos == MASTER_TOP || master_pos == MASTER_BOTTOM)
            wx += ww;
        else
            wy += wh;
    }
}

void resize(const Arg *arg){
    int dir = arg->i;
    float change = 0;
    
    if(nclients < 2)
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
    if(mfactor + change > 1 || mfactor + change < 0)
        return;
    
    mfactor += change;
    tile(tile_mode);
}

void rotate(const Arg *arg){
    if(nclients < 2) return;

    int mode = arg->i;

    switch(mode){
        case HORIZONTAL:
            horizontal_rotate();
            break;

        case MASTER:
            master_rotate();
            break;
    }
}

void horizontal_rotate(){
    XWindowAttributes first_attr;
    XGetWindowAttributes(dpy, clients[0], &first_attr);

    for(int i = 0; i < nclients; i++){
        if(nclients-1 == i){
            XMoveWindow(dpy, clients[i], first_attr.x, first_attr.y);
            break;  
        }

        XWindowAttributes next_attr;
        XGetWindowAttributes(dpy, clients[i+1], &next_attr);

        XMoveWindow(dpy, clients[i], next_attr.x, next_attr.y);  
    }
}

void master_rotate(){
    //Cycle the enum
    master_pos = (master_pos+1) % 4;
    tile(tile_mode);
}

void spawn(const Arg *arg){
    if(fork() == 0){
        setsid(); // Create a new session for the child
        execvp(arg->cparr[0], (char *const *)arg->cparr);
        
        //If child fails
        perror("execvp failed");
        _exit(1); // A safe way to terminate the forked child
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

void unmap(const Arg *arg){
    (void)arg;
    if(nclients < 1) return;
    
    if(subwin_unmapped == false){
        XUnmapSubwindows(dpy, root);
        subwin_unmapped = true;
    }
    else{
        XMapSubwindows(dpy, root);
        subwin_unmapped = false;
    }
}

void kill_window(const Arg *arg){
    (void)arg;
    if(focused == None) return;
    
    Atom *protocols;
    int n; //Number of protocols the client has
    Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", false);

    if(XGetWMProtocols(dpy, focused, &protocols, &n)){
        for(int i = 0; i < n; i++){
            if(protocols[i] == wm_delete){
                XEvent ev = {0};
                ev.type = ClientMessage;
                ev.xclient.window = focused;
                ev.xclient.message_type = wm_protocols;
                ev.xclient.format = 32;
                // l type because format is 32 bits
                ev.xclient.data.l[0] = wm_delete;
                ev.xclient.data.l[1] = CurrentTime;

                XSendEvent(dpy, focused, False, NoEventMask, &ev);
                XFree(protocols);
                return;
            }
        }
        XFree(protocols);
    }
    //If client is not ICCCM compliant
    XKillClient(dpy, focused);
}


void manage(Window w){
    clients[nclients] = w;
    nclients++;        
    focus(w);
    if(nclients == 1) 
        master = w;
    tile(tile_mode);
}

void unmanage(Window w){
    for(int i = 0; i < nclients; i++){
        if(clients[i] == w){
            for(int j = i; j < nclients-1; j++){
                clients[j] = clients[j+1];    
            }
            nclients--;
            break;
        }
    }

    if (master == w) {
        if (nclients > 0)
            master = clients[0];
        else
            master = None;
    }

    if (nclients > 0)
        focus(master);
    else
        focus(root);

    tile(tile_mode);
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
    
    //basically disables focus on hover
    XSetInputFocus(dpy, None, RevertToParent, CurrentTime);

    XSelectInput(dpy, root, SubstructureRedirectMask | 
            SubstructureNotifyMask |
            EnterWindowMask |
            LeaveWindowMask |
            FocusChangeMask);
    
    grab_key(XK_r, Mod1Mask);
    grab_key(XK_l, Mod1Mask);
    grab_key(XK_h, Mod1Mask);
    grab_key(XK_d, Mod1Mask);
    grab_key(XK_k, Mod1Mask);
    grab_key(XK_Return, Mod1Mask);
    
    //resize keys
    grab_key(XK_h, Mod1Mask | ControlMask);
    grab_key(XK_l, Mod1Mask | ControlMask);
    grab_key(XK_k, Mod1Mask | ControlMask);
    grab_key(XK_j, Mod1Mask | ControlMask);

    XGrabButton(dpy, 1, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    
    //Create and define the cursor
    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, DefaultRootWindow(dpy), cursor);

    XSync(dpy, false);
    for(;;)
    {
        XNextEvent(dpy, &ev);
        switch(ev.type){    
            case KeyPress:
                OnKeyPress(&ev.xkey);
                break;

            case ButtonPress:
                if(ev.xbutton.subwindow != None){
                    focus(ev.xbutton.subwindow);
                }
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


