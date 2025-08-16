#ifndef X11_WINX_H
#define X11_WINX_H

#include <X11/Xlib.h>

#include "shl_defs.h"

struct WinxNative {
  Display *display;
  i32      screen;
  Atom     wm_delete_window;
  XIM      im;
};

struct WinxNativeWindow {
  Window window;
  GC     graphic_context;
  XIC    ic;
};

#endif // X11_WINX_H
