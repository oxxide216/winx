#include <X11/Xlib.h>
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
                    StructureNotifyMask)

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  winx->display = XOpenDisplay(NULL);
  winx->screen = DefaultScreen(winx->display);
  winx->im = XOpenIM(winx->display, NULL, NULL, NULL);

  return winx;
}

WinxNativeWindow *winx_create_native_window(WinxNative *winx, Str name, u32 width, u32 height) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));

  Window root_window = RootWindow(winx->display, winx->screen);
  i32 depth = DefaultDepth(winx->display, winx->screen);
  Visual *visual = DefaultVisual(winx->display, winx->screen);

  XSetWindowAttributes attributes = {
    .background_pixel = BlackPixel(winx->display, winx->screen),
    .event_mask = EVENT_MASK,
  };

  window->window = XCreateWindow(winx->display, root_window,
                                 0, 0, width, height, 0, depth,
                                 InputOutput, visual,
                                 CWBackPixel | CWEventMask,
                                 &attributes);

  char *cstr_name = name.ptr;
  if (strlen(cstr_name) != name.len) {
    u8 *cstr_name = malloc(name.len + 1);
    memcpy(cstr_name, name.ptr, name.len);
    cstr_name[name.len] = '\0';
  }

  XStoreName(winx->display, window->window, cstr_name);

  XMapWindow(winx->display, window->window);

  winx->wm_delete_window = XInternAtom(winx->display, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(winx->display, window->window, &winx->wm_delete_window, 1);

  window->ic = XCreateIC(winx->im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, window->window, XNFocusWindow, window->window, NULL);

  XSetICFocus(window->ic);

  setlocale(LC_ALL, "");

  return window;
}

void winx_native_cleanup(WinxNative *winx) {
  XCloseDisplay(winx->display);
}

void winx_destroy_native_window(WinxNative *winx, WinxNativeWindow *window) {
  XUnmapWindow(winx->display, window->window);
  XDestroyWindow(winx->display, window->window);
}
