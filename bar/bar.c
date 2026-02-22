#include <X11/Xlib.h>
#include <X11/Xatom.h>

typedef struct{
    Window bar;
    int bar_height;
    int bar_width;
    GC gc;
}Bar;

void draw_bar(Display *dpy, Bar *bar){
    XFillRectangle(dpy, bar->bar, bar->gc, 0, 0, bar->bar_width, bar->bar_height);
}

void set_dock(Display *dpy, Bar *bar){
    Atom net_wm_win_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

    XChangeProperty(
            dpy,
            bar->bar,
            net_wm_win_type,
            XA_ATOM,
            32,
            PropModeReplace,
            (unsigned char *)&net_wm_type_dock,
            1);
}

void set_strut(Display *dpy, Bar *bar, int screen){
    Atom net_wm_strut_partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);

    long strut[12] = {0};
    strut[2] = bar->bar_height;
    strut[8] = 0;
    strut[9] = DisplayWidth(dpy, screen);


    XChangeProperty(dpy,
            bar->bar,
            net_wm_strut_partial,
            XA_CARDINAL,
            32,
            PropModeReplace,
            (unsigned char *)strut,
            12);
}

int main()
{
    Display *dpy = XOpenDisplay(0x0);
    if(!(dpy)) return 1;

    Bar bar;

    Window root = DefaultRootWindow(dpy);
    int screen = DefaultScreen(dpy);
    bar.bar_width = DisplayWidth(dpy, screen);
    bar.bar_height = 60;

    bar.bar = XCreateSimpleWindow(dpy, 
            root,
            0,0,
            bar.bar_width, bar.bar_height,
            0,
            WhitePixel(dpy, screen),
            WhitePixel(dpy, screen));

    bar.gc = XCreateGC(dpy, bar.bar, 0, NULL);
    XSetForeground(dpy, bar.gc, BlackPixel(dpy, screen));

    set_dock(dpy, &bar);
    set_strut(dpy, &bar, screen);

    XSelectInput(dpy, bar.bar, ExposureMask);

    XMapWindow(dpy, bar.bar);

    XFlush(dpy);
    for(;;){   
        XEvent ev;
        XNextEvent(dpy, &ev);

        if(ev.type == Expose){
            draw_bar(dpy, &bar);
        }
    }
    XFreeGC(dpy, bar.gc);
}
