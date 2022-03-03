#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>

#include "header_files/palette.h"

int main(int argc, char *argv[]) {

    Display *displ;
    int screen;
    Window win;
    XWindowAttributes winattr;
    XEvent event;

    // Global window constants.
    winattr.width = 800;
    winattr.height = 800;

    // Locale optimisation.
    if (setlocale(LC_ALL, "") == NULL) {
        fprintf(stderr, "setlocale(LC_ALL, "") is NULL.\n");
        exit(1);
    }
    if (!XSupportsLocale()) {
        fprintf(stderr, "Locale is not supported.Exiting.\n");
        exit(1);
    }
    if (XSetLocaleModifiers("") == NULL) {
        fprintf(stderr, "XSetLocaleModifiers is NULL.\n");
        exit(1);
    }

    displ = XOpenDisplay(NULL);
    if (displ == NULL) {
        fprintf(stderr, "Failed to open Display.\n");
        exit(1);
    }

    screen = DefaultScreen(displ);
    printf("Default screen value: %d\n", screen);

    /*  Root main Window */
    win = XCreateSimpleWindow(displ, XRootWindow(displ, screen), 0, 0, winattr.width, winattr.height, 0, XWhitePixel(displ, screen), XBlackPixel(displ, screen));
    XSelectInput(displ, win, ExposureMask | KeyPressMask | ButtonPressMask /*| PointerMotionMask*/);
    XMapWindow(displ, win);

    /* Delete window initializer area */
    Atom wm_delete_window = XInternAtom(displ, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(displ, win, &wm_delete_window, 1);

    /* Change main window Title */
    Atom new_attr = XInternAtom(displ, "WM_NAME", False);
    Atom type =  XInternAtom(displ, "STRING", False);
    XChangeProperty(displ, win, new_attr, type, 8, PropModeReplace, (unsigned char*)"Mandelbrot Set", 14);

    /* Get user text input *******************************************************************/
    XIM xim;
    XIC xic;
    char *failed_arg;
    XIMStyles *styles;
    //XIMStyle xim_requested_style;
    xim = XOpenIM(displ, NULL, NULL, NULL);
    if (xim == NULL) {
        fprintf(stderr, "Failed to open Input Method.\n");
        exit(2);
    }
    failed_arg = XGetIMValues(xim, XNQueryInputStyle, &styles, NULL);
    if (failed_arg != NULL) {
        fprintf(stderr, "Failed to obtain input method's styles.\n");
        exit(3);
    }
    for (int i = 0; i < styles->count_styles; i++) {
        printf("Styles supported %lu.\n", styles->supported_styles[i]);
    }
    xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, win, NULL);
    if (xic == NULL) {
        fprintf(stderr, "Could not open xic.\n");
        exit(4);
    }
    XSetICFocus(xic);

    /* Add grafical context to window */
    XGCValues values;
    values.line_width = 0;
    values.line_style = LineSolid;
    values.fill_style = FillSolid;
    values.fill_rule =  WindingRule;
    GC gc = XCreateGC(displ, win, GCLineWidth | GCLineStyle | GCFillStyle | GCFillRule, &values);

    double horiz = 2.00;
    double vert = 2.00;
    double zoom = 4.00;
    int max_iter = 100;

    double init_x = 0;
    double init_y = 0;

    while (1) {
        while (XPending(displ) > 0) {
            XNextEvent(displ, &event);
            
            if (event.type == ClientMessage) {
                if (event.xclient.data.l[0] == wm_delete_window) {
                    printf("WM_DELETE_WINDOW");
                    if (gc != NULL) {
                        XFreeGC(displ, gc);
                    }
                    XCloseDisplay(displ);
                    return 0;
                }
            } else if (event.type == Expose && event.xclient.window == win) {
                /* Get window attributes */
                XGetWindowAttributes(displ, win, &winattr);

                for (int x = 0; x <= winattr.width; x++) {
                    for (int y = 0; y <= winattr.height; y++) {

                        double a = (x - (winattr.width / horiz)) / (winattr.width / zoom);
                        double b = (y - (winattr.height / vert)) / (winattr.height / zoom);
                        double curr_a = a;
                        double curr_b = b;
                        int n = 0;

                        while (n < max_iter) {
                            double iter_a = (a * a) - (b * b);
                            double iter_b = 2 * a * b;
                            a = iter_a + curr_a;
                            b = iter_b + curr_b;

                            if (abs(a + b) > 16) {
                                break;
                            }
                            n++;
                        }
                        if (n == max_iter) {
                            values.foreground = 0;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                                
                        } else if (n < max_iter && n >= 10) {
                            values.foreground = n * n;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                         
                        } else if (n < max_iter && n < 10) {
                            values.foreground = n * n;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                              
                        } else {
                            values.foreground = 0;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);  
                        }
                    }
                }
            } else if (event.type == ButtonPress && event.xclient.window == win) {

                if (init_x == 0.00 && init_y == 0.00) {
                    init_x = (((double)event.xbutton.x - (winattr.width / horiz)) / (winattr.width / zoom));
                    init_y = (((double)event.xbutton.y - (winattr.height / vert)) / (winattr.height / zoom));
                } else {
                    init_x = init_x + (((double)event.xbutton.x - (winattr.width / horiz)) / (winattr.width / zoom));
                    init_y = init_y + (((double)event.xbutton.y - (winattr.height / vert)) / (winattr.height / zoom));
                }
                
                if (event.xkey.keycode == 1) {
                    zoom *= 0.50;
                } else if (event.xkey.keycode == 3) {
                    zoom /= 0.50;
                }
                
                for (int x = 0; x <= winattr.width; x++) {
                    for (int y = 0; y <= winattr.height; y++) {

                        double a = ((x - (winattr.width / horiz)) / (winattr.width / zoom)) + init_x;
                        double b = ((y - (winattr.height / vert)) / (winattr.height / zoom)) + init_y;
                        double curr_a = a;
                        double curr_b = b;
                        int n = 0;

                        while (n < max_iter) {;
                            double iter_a = (a * a) - (b * b);
                            double iter_b = 2 * a * b;
                            a = iter_a + curr_a;
                            b = iter_b + curr_b;

                            if (abs(a + b) > 16.00) {
                                break;
                            }
                            n++;
                        }
                        if (n == max_iter) {
                            values.foreground = 0;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                                
                        } else if (n < max_iter && n >= 10) {
                            values.foreground = pallete[n];
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                         
                        } else if (n < max_iter && n < 10) {
                            values.foreground = n * n;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                              
                        } else {
                            values.foreground = pallete[270 - n];
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);
                        }
                    }
                }
            } else if (event.type == KeyPress && event.xclient.window == win) {
                int count = 0;  
                KeySym keysym = 0;
                char buffer[32];
                Status status = 0;   
                count = Xutf8LookupString(xic, &event.xkey, buffer, 32, &keysym, &status);
                printf("Button pressed.\n");
                printf("Count %d.\n", count);
                if (status == XBufferOverflow) {
                    printf("Buffer Overflow...\n");
                }
                if (count) {
                    printf("The Button that was pressed is %s.\n", buffer);
                }
                if (status == XLookupKeySym || status == XLookupBoth) {
                    printf("Status: %d\n", status);
                }
                printf("Pressed key: %lu.\n", keysym);
                if (keysym == 65361) {
                    horiz *= 0.50;
                } else if (keysym == 65363) {
                    horiz /= 0.50;
                } else if (keysym == 65362) {
                    vert *= 0.50; 
                } else if (keysym == 65364) {
                    vert /= 0.50;
                } else if (keysym == 65293) {
                    zoom *= 0.50;
                }

                for (int x = 0; x <= winattr.width; x++) {
                    for (int y = 0; y <= winattr.height; y++) {

                        double a = (x - (winattr.width / horiz)) / (winattr.width / zoom);
                        double b = (y - (winattr.height / vert)) / (winattr.height / zoom);
                        double curr_a = a;
                        double curr_b = b;
                        int n = 0;

                        while (n < max_iter) {
                            double iter_a = (a * a) - (b * b);
                            double iter_b = 2 * a * b;
                            a = iter_a + curr_a;
                            b = iter_b + curr_b;

                            if (abs(a + b) > 16) {
                                break;
                            }
                            n++;
                        }
                        if (n == max_iter) {
                            values.foreground = 0;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                                
                        } else if (n < max_iter && n >= 10) {
                            values.foreground = n * n;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                         
                        } else if (n < max_iter && n < 10) {
                            values.foreground = n * n;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);                              
                        } else {
                            values.foreground = 0;
                            XChangeGC(displ, gc, GCForeground, &values);
                            XDrawPoint(displ, win, gc, x, y);  
                        }
                    }
                }
            } else {
                printf("Main Window Event.\n");
                printf("Event Type: %d\n", event.type);
            }
        }
    }

    return 0;
}

