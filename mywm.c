#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdbool.h>
#include <X11/cursorfont.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

Display* dpy;
XWindowAttributes attr;
XButtonEvent start;
XEvent ev;
Window root;

void grabKey(char* key, unsigned int mod){
	KeySym sym = XStringToKeysym(key);
	KeyCode code = XKeysymToKeycode(dpy, sym);
	XGrabKey(dpy, code, mod, DefaultRootWindow(dpy), false, GrabModeAsync, GrabModeAsync);
}


int main(void)
{
	
    if(!(dpy = XOpenDisplay(0x0))) return 1;
	
	root = DefaultRootWindow(dpy);
	
	//Recieve KeyPress event when "F1" + alt is pressed
    grabKey("F1", Mod1Mask);
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
        if(ev.type == KeyPress && ev.xkey.subwindow != None)
            XRaiseWindow(dpy, ev.xkey.subwindow);
        else if(ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        else if(ev.type == MotionNotify && start.subwindow != None)
        {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, start.subwindow,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        }
        else if(ev.type == ButtonRelease)
            start.subwindow = None;
    }
	XCloseDisplay(dpy);
}


