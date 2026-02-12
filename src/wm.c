#include "wm.h"
#include "layout.h"
#include "keys.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>



Display* dpy;
int sw; //screen width
int sh; //screen height
int screen;  
Window root;

Client *focused = NULL;
Client *master = NULL;
Client clients[128];
int nclients = 0;

bool subwin_unmapped = false;


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
    handle_keypress(ev);
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

    Client *best = NULL;
    int bestdist = INT_MAX;

    XWindowAttributes fa;
    XGetWindowAttributes(dpy, focused->win, &fa);

    //get the center of the focused window
    int fx = fa.x + fa.width / 2;
    int fy = fa.y + fa.height / 2;

    for (int i = 0; i < nclients; i++) {
        Client *w = &clients[i];
        if (w == focused) continue;

        XWindowAttributes wa;
        XGetWindowAttributes(dpy, w->win, &wa);

        //get the center of the window that we are comparing
        int wx = wa.x + wa.width / 2;
        int wy = wa.y + wa.height / 2;

        //distance from the focused window center to the other window center
        int dx = wx - fx;
        int dy = wy - fy;

        bool valid = false;
        int dist = 0;

        //evaluate each calculated distance based on direction and penalize offset on the unfocused direction
        switch (dir) {
            case DIR_LEFT:  
                valid = dx < 0; 
                dist = -dx + abs(dy); 
                break;
            case DIR_RIGHT: 
                valid = dx > 0; 
                dist = dx + abs(dy); 
                break;
            case DIR_UP:    
                valid = dy < 0; 
                dist = -dy + abs(dx); 
                break;
            case DIR_DOWN:  
                valid = dy > 0; 
                dist = dy + abs(dx); 
                break;
        }

        //get the records
        if (valid && dist < bestdist) {
            best = w;
            bestdist = dist;
        }
    }

    if (best)
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
    if(focused == NULL) return;
    
    Atom *protocols;
    int n; //Number of protocols the client has
    Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", false);

    if(XGetWMProtocols(dpy, focused->win, &protocols, &n)){
        for(int i = 0; i < n; i++){
            if(protocols[i] == wm_delete){
                XEvent ev = {0};
                ev.type = ClientMessage;
                ev.xclient.window = focused->win;
                ev.xclient.message_type = wm_protocols;
                ev.xclient.format = 32;
                // l type because format is 32 bits
                ev.xclient.data.l[0] = wm_delete;
                ev.xclient.data.l[1] = CurrentTime;

                XSendEvent(dpy, focused->win, False, NoEventMask, &ev);
                XFree(protocols);
                return;
            }
        }
        XFree(protocols);
    }
    //If client is not ICCCM compliant
    XKillClient(dpy, focused->win);
}

void manage(Window w){
    Client *c = &clients[nclients];
    c->win = w;
    nclients++;
    focus(c);
    if(nclients == 1)
        master = c;
    tile();
}


void unmanage(Window w){
    int idx = -1;

    for (int i = 0; i < nclients; i++) {
        if (clients[i].win == w) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return;

    Client *removed = &clients[idx];

    bool was_focused = (focused == removed);
    bool was_master  = (master  == removed);

    for (int i = idx; i < nclients - 1; i++)
        clients[i] = clients[i + 1];

    nclients--;

    if (nclients == 0) {
        focused = NULL;
        master  = NULL;
        XSetInputFocus(dpy, root, RevertToParent, CurrentTime);
        return;
    }

    if (was_master)
        master = &clients[0];

    if (was_focused)
        focus(master);
    else
        focus(focused);

    tile();
}

void focus(Client *c){
    if(!c) return;
    focused = c;
    XSetInputFocus(dpy, c->win, RevertToParent, CurrentTime);
}

void set_master(const Arg *arg){
    (void)arg;
    if(!focused) return;
    master = focused;
    tile();
}

Client* wintoclient(Window w){
    for(int i = 0; i < nclients; i++){
        if(clients[i].win == w)
            return &clients[i];
    }
    return NULL;
}

