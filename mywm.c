#include "mywm.h"
#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdbool.h>
#include <X11/cursorfont.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

Display* dpy;
int sw; //screen width
int sh; //screen height
int screen;  
XWindowAttributes attr;
XButtonEvent start;
XEvent ev;
Window root;
Window clients[128];
int nclients = 0;


void grabKey(char* key, unsigned int mod){
    KeySym sym = XStringToKeysym(key);
    KeyCode code = XKeysymToKeycode(dpy, sym);
    XGrabKey(dpy, code, mod, DefaultRootWindow(dpy), false, GrabModeAsync, GrabModeAsync);
}
/*
void Frame(Window w){
    unsigned int BORDER_WIDTH = 3;
    unsigned long BORDER_COLOR = 0xff0000;
    unsigned long BG_COLOR = 0x0000ff;

    XWindowAttributes xwinattrs;
    XGetWindowAttributes(dpy, w, &xwinattrs);

    Window frame = XCreateSimpleWindow(
        dpy,
        root,
        xwinattrs.x,
        xwinattrs.y,
        xwinattrs.width,
        xwinattrs.height,
        BORDER_WIDTH,
        BORDER_COLOR,
        BG_COLOR);

    XReparentWindow(dpy, w, frame, 0, 0);

    XMapWindow(dpy, frame);
}
*/
void OnMapRequest(XMapRequestEvent* ev){
    //Frame(ev->window);

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

void Tile(){
    for(int i = 0; i < nclients; i++){
        int h = sh;
        int w = sw / nclients;

        int x = w * i;
        int y = 0;

        XMoveResizeWindow(dpy, clients[i], x, y, w, h);
        printf("Window %d: x%d, width%d\n", i, x, w);
    }    
}

void manage(Window w){
    clients[nclients] = w;
    nclients++;        
    Tile();
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
    Tile();
}
      
int main(void)
{
    
    if(!(dpy = XOpenDisplay(0x0))) return 1;
    
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = DefaultRootWindow(dpy);

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
    
    //Recieve KeyPress event when "F1" + alt is pressed
    grabKey("R", Mod1Mask);
    XGrabButton(dpy, 1, Mod1Mask, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, root, True,
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
                Tile();
                break;

            case ButtonPress:
                if(ev.xbutton.subwindow != None){
                    XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
                    start = ev.xbutton;
                }
                break;

            case MotionNotify:
                if(start.subwindow != None){
                    int xdiff = ev.xbutton.x_root - start.x_root;
                    int ydiff = ev.xbutton.y_root - start.y_root;
                    XMoveResizeWindow(dpy, start.subwindow,
                        attr.x + (start.button==1 ? xdiff : 0),
                        attr.y + (start.button==1 ? ydiff : 0),
                        MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                        MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
                }
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


