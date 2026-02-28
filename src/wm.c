#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xatom.h>

#include "wm.h"
#include "layout.h"
#include "keys.h"
#include "forward.h"

bool subwin_unmapped = false;

void OnMapRequest(WM *wm, XMapRequestEvent* ev){
    XWindowAttributes wa;
    XGetWindowAttributes(wm->dpy, ev->window, &wa);
    if(wa.override_redirect)
        return;

    manage(wm, ev->window);
}

void OnConfigureRequest(WM *wm, XConfigureRequestEvent* ev){
    XWindowChanges changes = {0};

    changes.x = ev->x;
    changes.y = ev->y;
    changes.width = ev->width;
    changes.height = ev->height;
    changes.border_width = ev->border_width;
    changes.sibling = ev->above;
    changes.stack_mode = ev->detail;

    XConfigureWindow(wm->dpy, ev->window, ev->value_mask, &changes);
}

void OnDestroyNotify(WM *wm, XDestroyWindowEvent *ev){
    unmanage(wm, ev->window);
}

void OnKeyPress(WM *wm, XKeyEvent *ev){
    handle_keypress(wm, ev);
}

void handle_XEvent(WM *wm, XEvent *ev){
    switch(ev->type){
        case KeyPress:
            OnKeyPress(wm, &ev->xkey);
            break;

        case ButtonPress:
            if(ev->xbutton.subwindow != None){
                focus(wm, wintoclient(wm, ev->xbutton.subwindow));
            }
            break;

        case ConfigureRequest:
            OnConfigureRequest(wm, &ev->xconfigurerequest);
            break;

        case MapRequest:
            OnMapRequest(wm, &ev->xmaprequest);
            break;

        case DestroyNotify:
            OnDestroyNotify(wm, &ev->xdestroywindow);
            break;
    }
}

void focus_direction(WM *wm, const Arg *arg) {
    int dir = arg->i;
    if (!wm->focused) return;

    Client *best = NULL;
    int bestdist = INT_MAX;

    XWindowAttributes fa;
    XGetWindowAttributes(wm->dpy, wm->focused->win, &fa);

    //get the center of the focused window
    int fx = fa.x + fa.width / 2;
    int fy = fa.y + fa.height / 2;

    for (Client *c = wm->clients; c; c = c->next) {
        if (c == wm->focused) continue;

        XWindowAttributes wa;
        XGetWindowAttributes(wm->dpy, c->win, &wa);

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
            best = c;
            bestdist = dist;
        }
    }

    if (best)
        focus(wm, best);
}

void unmap(WM *wm, const Arg *arg){
    (void)arg;
    if(wm->nclients < 1) return;

    if(subwin_unmapped == false){
        XUnmapSubwindows(wm->dpy, wm->root);
        subwin_unmapped = true;
    }
    else{
        XMapSubwindows(wm->dpy, wm->root);
        subwin_unmapped = false;
    }
}

void kill_window(WM *wm, const Arg *arg){
    (void)arg;
    if(wm->focused == NULL) return;

    Atom *protocols;
    int n; //Number of protocols the client has
    Atom wm_delete = XInternAtom(wm->dpy, "WM_DELETE_WINDOW", False);
    Atom wm_protocols = XInternAtom(wm->dpy, "WM_PROTOCOLS", False);

    if(XGetWMProtocols(wm->dpy, wm->focused->win, &protocols, &n)){
        for(int i = 0; i < n; i++){
            if(protocols[i] == wm_delete){
                XEvent ev = {0};
                ev.type = ClientMessage;
                ev.xclient.window = wm->focused->win;
                ev.xclient.message_type = wm_protocols;
                ev.xclient.format = 32;
                // l type because format is 32 bits
                ev.xclient.data.l[0] = wm_delete;
                ev.xclient.data.l[1] = CurrentTime;

                XSendEvent(wm->dpy, wm->focused->win, False, NoEventMask, &ev);
                XFree(protocols);
                return;
            }
        }
        XFree(protocols);
    }
    //If client is not ICCCM compliant
    XKillClient(wm->dpy, wm->focused->win);
}

void manage(WM *wm, Window win){
    Wintype type = classify_window(wm, win);

    if(type == WIN_DOCK){
        handle_dock(wm, win);
        return;
    }

    Client *c = malloc(sizeof(Client));
    if(!c) return;

    c->win = win;
    c->next = wm->clients;
    c->floating = false;

    wm->clients = c;
    wm->nclients++;

    switch(type){
        case WIN_DIALOG:
            c->floating = true;
            break;

        case WIN_SPLASH:
            c->floating = true;
            break;

        case WIN_DOCK:
        case WIN_MENU:
            c->floating = true;

        case WIN_NORMAL:
        default:
            break;
    }

    XMapWindow(wm->dpy, win);

    if(c->floating){
        Window parent = get_transient(wm, win);
        if(parent) parent_center(wm, parent, win);
        else screen_center(wm, win);
    }

    focus(wm, c);

    if(!c->floating){
        if(wm->nclients == 1)
            wm->master = c;

        tile(wm);
    }
}


void unmanage(WM *wm, Window win){
    if(unmanage_dock(wm, win)){
        recalc_usable_area(wm);
        tile(wm);
        return;
    }

    Client *prev = NULL;
    Client *cur = wm->clients;

    while(cur){
        if(cur->win == win)
            break;

        prev = cur;
        cur = cur->next;
    }

    if(!cur) return; //client not found 

    bool was_master = (cur == wm->master);
    bool was_focused = (cur == wm->focused);

    if(prev)
        prev->next = cur->next;
    else
        wm->clients = cur->next;

    if(was_master)
        wm->master = wm->clients;
    if(was_focused)
        wm->focused = wm->master;

    wm->nclients--;

    tile(wm);
    free(cur); //free the removed client
}

void focus(WM *wm, Client *c){
    if(!c) return;
    wm->focused = c;
    XSetInputFocus(wm->dpy, c->win, RevertToParent, CurrentTime);
}

void set_master(WM *wm, const Arg *arg){
    (void)arg;
    if(!wm->focused) return;
    wm->master = wm->focused;
    tile(wm);
}

Client* wintoclient(WM *wm, Window win){
    for(Client *c = wm->clients; c; c = c->next){
        if(c->win == win){
            return c;
        }
    }
    return NULL;
}

int has_wintype(int nitems, Atom *atoms, Atom type){
    int res = 0;

    for(unsigned long i = 0; i < nitems; i++){
        if(atoms[i] == type){
            res = 1;
            break;
        }
    }
    return res;
}

int get_strut(WM *wm, Dock *dock){
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;

    if(XGetWindowProperty(
                wm->dpy,
                dock->win,
                wm->net_strut_partial,
                0,
                12,
                False,
                XA_CARDINAL,
                &actual_type,
                &actual_format,
                &nitems,
                &bytes_after,
                &data)!= Success)
        return 0;

    if(!data || nitems < 4){
        if(data)
            XFree(data);
        return 0;
    }

    long *strut = (long *)data;

    dock->left = strut[0];
    dock->right = strut[1];
    dock->top = strut[2];
    dock->bottom = strut[3];

    XFree(data);
    return 1;
}

void recalc_usable_area(WM *wm){
    int occ_height, occ_width;
    for(int i = 0; i < wm->ndocks; i++){
        occ_height = wm->docks[i].bottom + wm->docks[i].top;
        occ_width = wm->docks[i].right + wm->docks[i].left;
    }

    wm->usable_height = wm->sh - occ_height;
    wm->usable_width = wm->sw - occ_width;
}

int unmanage_dock(WM *wm, Window win){
    int idx = -1;

    for(int i = 0; i < wm->ndocks; i++){
        if(wm->docks[i].win == win){
            idx = i;
            break;
        }
    }
    if(idx == -1) return 0;

    for(int i = idx; i < wm->ndocks-1; i++){
        wm->docks[i] = wm->docks[i + 1];
    }

    wm->ndocks--;
    return 1;
}

Client* get_client(WM *wm, Window win){
    for(Client *c = wm->clients; c; c = c->next){
        if(c->win == win)
            return c;
    }

    return NULL;
}

Wintype classify_window(WM *wm, Window win){
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;

    Wintype type = WIN_NORMAL;

    if(XGetWindowProperty(
                wm->dpy,
                win,
                wm->net_win_type,
                0,
                32,
                False,
                XA_ATOM,
                &actual_type,
                &actual_format,
                &nitems,
                &bytes_after,
                &data) != Success)
        return WIN_NORMAL;

    if(actual_type != XA_ATOM || actual_format != 32 || data == NULL)
        goto cleanup;

    //Recast to atom ptr because data is unsigned char initially
    Atom *atoms = (Atom *)data;

    if(has_wintype(nitems, atoms, wm->net_wintype_dialog)){
        type = WIN_DIALOG;
        goto cleanup;
    }

    if(has_wintype(nitems, atoms, wm->net_wintype_menu)){
        type = WIN_MENU;
        goto cleanup;
    }

    if(has_wintype(nitems, atoms, wm->net_win_type_dock)){
        type = WIN_DOCK;
        goto cleanup;
    }

    if(has_wintype(nitems, atoms, wm->net_wintype_splash)){
        type = WIN_SPLASH;
        goto cleanup;
    }

    Window parent;
    if(XGetTransientForHint(wm->dpy, win, &parent)){
        type = WIN_DIALOG;
    }

cleanup:
    if(data)
        XFree(data);

    return type;
}

void handle_dock(WM *wm, Window win){
    Dock *dock = &wm->docks[wm->ndocks];
    wm->ndocks++;
    dock->win = win;

    get_strut(wm, dock);
    recalc_usable_area(wm);

    XMapWindow(wm->dpy, win);
    tile(wm);
    return;
}

Window get_transient(WM *wm, Window win){
    Window transient = None;

    if(XGetTransientForHint(wm->dpy, win, &transient))
        return transient;

    return None;
}
