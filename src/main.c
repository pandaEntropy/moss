#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <errno.h>

#include "wm.h"
#include "keys.h"
#include "ipc.h"

int main(void)
{
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

    key_setup(); //grabs all the necessary keys

    XGrabButton(dpy, 1, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, None, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    
    //Create and define the cursor
    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, DefaultRootWindow(dpy), cursor);

    XSync(dpy, False);

    ipc_init();

    int xfd = ConnectionNumber(dpy);

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
            ipc_handle();
            XFlush(dpy);
        }

        if(FD_ISSET(xfd, &fds)){
            while(XPending(dpy)){
               XEvent ev;
               XNextEvent(dpy, &ev);
               handle_XEvent(&ev);
            }
        }
    }
    XCloseDisplay(dpy);
}

