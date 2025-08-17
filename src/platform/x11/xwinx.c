#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "xwinx.h"
#include "shl_defs.h"
#include "shl_str.h"
#include "shl_log.h"

#define EVENT_MASK (KeyPressMask |      \
                    KeyReleaseMask |    \
                    ButtonPressMask |   \
                    ButtonReleaseMask | \
                    PointerMotionMask | \
                    EnterWindowMask |   \
                    LeaveWindowMask |   \
                    StructureNotifyMask)

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  winx->display = XOpenDisplay(NULL);
  winx->screen = DefaultScreen(winx->display);
  winx->wm_delete_window = XInternAtom(winx->display, "WM_DELETE_WINDOW", false);
  winx->im = XOpenIM(winx->display, NULL, NULL, NULL);

  XSync(winx->display, false);

  return winx;
}

WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));
  window->winx = winx;
  window->visual = XDefaultVisual(winx->display, winx->screen);
  window->depth = XDefaultDepth(winx->display, winx->screen);

  Window root_window = RootWindow(winx->display, winx->screen);

  XSetWindowAttributes attributes = {
    .colormap = XCreateColormap(winx->display, root_window, window->visual, AllocNone),
    .background_pixel = 0,
    .border_pixel = 0,
    .event_mask = EVENT_MASK,
  };

  window->window = XCreateWindow(winx->display, root_window,
                                 0, 0, width, height, 0, window->depth,
                                 InputOutput, window->visual,
                                 CWBackPixel | CWColormap |
                                 CWBorderPixel | CWEventMask,
                                 &attributes);

  char *cstr_name = name.ptr;
  if (strlen(cstr_name) != name.len) {
    u8 *cstr_name = malloc(name.len + 1);
    memcpy(cstr_name, name.ptr, name.len);
    cstr_name[name.len] = '\0';
  }

  XStoreName(winx->display, window->window, cstr_name);
  XMapWindow(winx->display, window->window);
  XSetWMProtocols(winx->display, window->window, &winx->wm_delete_window, 1);

  u64 gcm = GCGraphicsExposures;
  XGCValues gcv;
  gcv.graphics_exposures = 0;
  window->graphic_context = XCreateGC(winx->display, window->window, gcm, &gcv);

  window->ic = XCreateIC(winx->im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, window->window, XNFocusWindow, window->window, NULL);
  XSetICFocus(window->ic);

  setlocale(LC_ALL, "");

  return window;
}

void winx_native_init_framebuffer(WinxNativeWindow *window, u32 width,
                                  u32 height, u32 **framebuffer) {
  if (*framebuffer)
    free(*framebuffer);

  u32 len = width * height;
  *framebuffer = malloc(len * sizeof(u32));
  window->image = XCreateImage(window->winx->display, window->visual, window->depth, ZPixmap,
                               0, (char *) *framebuffer, width, height, 8, width * sizeof(u32));
}

void winx_native_draw(WinxNativeWindow *window, u32 width, u32 height) {
  XPutImage(window->winx->display, window->window, window->graphic_context,
            window->image, 0, 0, 0, 0, width, height);
}

void winx_native_destroy_window(WinxNativeWindow *window) {
  XFreeGC(window->winx->display, window->graphic_context);
  XDestroyImage(window->image);
  XUnmapWindow(window->winx->display, window->window);
  XDestroyWindow(window->winx->display, window->window);
}

void winx_native_cleanup(WinxNative *winx) {
  XCloseDisplay(winx->display);
}
