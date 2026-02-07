#include "wm.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>


int main(void)
{
    XEvent ev;
    if(!(dpy = XOpenDisplay(0x0))) return 1;
    
    int screen = DefaultScreen(dpy);
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
    grab_key(XK_j, Mod1Mask);
    grab_key(XK_k, Mod1Mask);
    
    grab_key(XK_d, Mod1Mask);
    grab_key(XK_x, Mod1Mask);
    grab_key(XK_Return, Mod1Mask);

    grab_key(XK_Return, Mod1Mask | ControlMask);
    
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

    XSync(dpy, False);
    for(;;)
    {
        XNextEvent(dpy, &ev);
        switch(ev.type){    
            case KeyPress:
                OnKeyPress(&ev.xkey);
                break;

            case ButtonPress:
                if(ev.xbutton.subwindow != None){
                    focus(wintoclient(ev.xbutton.subwindow));
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

