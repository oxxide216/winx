#ifndef X11_WINX_H
#define X11_WINX_H

#include <X11/Xlib.h>

#include "../winx.h"
#include "shl_defs.h"

struct WinxNative {
  Display *display;
  i32      screen;
  Atom     wm_delete_window;
  XIM      im;
};

struct WinxNativeWindow {
  WinxNative *winx;
  Window      window;
  GC          graphic_context;
  Visual     *visual;
  i32         depth;
  XIC         ic;
  u32        *framebuffer;
  XImage     *image;
};

#endif // X11_WINX_H
