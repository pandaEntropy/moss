#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <errno.h>

#include "wm.h"
#include "keys.h"
#include "ipc.h"

int main(void)
{
    WM wm = {0};

    if(!(wm.dpy = XOpenDisplay(0x0))) return 1;

    int screen = DefaultScreen(wm.dpy);
    wm.sw = DisplayWidth(wm.dpy, screen);
    wm.sh = DisplayHeight(wm.dpy, screen);
    wm.root = DefaultRootWindow(wm.dpy);

    wm.usable_height = wm.sh;
    wm.usable_width = wm.sw;

    //basically disables focus on hover
    XSetInputFocus(wm.dpy, None, RevertToParent, CurrentTime);

    XSelectInput(wm.dpy, wm.root, SubstructureRedirectMask | 
            SubstructureNotifyMask |
            EnterWindowMask |
            LeaveWindowMask |
            FocusChangeMask);

    key_setup(wm.dpy); //grabs all the necessary keys

    XGrabButton(wm.dpy, 1, None, wm.root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(wm.dpy, 3, None, wm.root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    //Create and define the cursor
    Cursor cursor = XCreateFontCursor(wm.dpy, XC_left_ptr);
    XDefineCursor(wm.dpy, DefaultRootWindow(wm.dpy), cursor);

    wm.net_win_type = XInternAtom(wm.dpy, "_NET_WM_WINDOW_TYPE", False);
    wm.net_win_type_dock = XInternAtom(wm.dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    wm.net_strut_partial = XInternAtom(wm.dpy, "_NET_WM_STRUT_PARTIAL", False);

    Atom net_supported = XInternAtom(wm.dpy, "_NET_SUPPORTED", False);

    Atom supported[] = {
        wm.net_strut_partial,
        wm.net_win_type,
        wm.net_win_type_dock
    };

    XChangeProperty(wm.dpy,
            wm.root,
            net_supported,
            XA_ATOM,
            32,
            PropModeReplace,
            (unsigned char *)supported,
            3);

    XSync(wm.dpy, False);

    ipc_init();

    int xfd = ConnectionNumber(wm.dpy);

    for(;;){
        fd_set fds;
        FD_ZERO(&fds); //clear the bitmask
        FD_SET(xfd, &fds); 
        FD_SET(wmfd, &fds);

        int maxfd = (xfd > wmfd ? xfd : wmfd) + 1;

        if(select(maxfd, &fds, NULL, NULL, NULL) < 0){
            if(errno == EINTR) continue;
            break;
        }

        if(FD_ISSET(wmfd, &fds)){
            ipc_handle(&wm);
            XFlush(wm.dpy);
        }

        if(FD_ISSET(xfd, &fds)){
            while(XPending(wm.dpy)){
               XEvent ev;
               XNextEvent(wm.dpy, &ev);
               handle_XEvent(&wm, &ev);
            }
        }
    }
    XCloseDisplay(wm.dpy);
}

