#ifndef X11_WINX_H
#define X11_WINX_H

#include <X11/Xlib.h>
#include <GL/glx.h>

#include "../winx.h"
#include "../../key_code.h"
#include "../../shl_defs.h"

struct WinxNative {
  Display     *display;
  i32          screen;
  Atom         wm_delete_window;
  XIM          im;
  XIC          ic;
  GLXFBConfig *fbc;
  GLXFBConfig  best_fbc;
};

struct WinxNativeWindow {
  WinxNative       *winx;
  Window            window;
  WinxGraphicsMode  graphics_mode;
  XVisualInfo      *visual_info;
  GC                graphic_context;
  u32              *framebuffer;
  XImage           *image;
  GLXContext        gl_context;
  XIC               ic;
  XEvent            prev_x_event;
  u64               is_key_pressed[WinxKeyCodeCount];
};

#endif // X11_WINX_H
