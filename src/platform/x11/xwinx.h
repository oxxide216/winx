#ifndef X11_WINX_H
#define X11_WINX_H

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <GL/glx.h>
#include <time.h>

#include "winx/key_code.h"
#include "../winx.h"
#include "../../wstr.h"
#include "shl/shl-defs.h"

struct WinxNative {
  Display     *display;
  i32          screen;
  Atom         wm_delete_window_atom;
  // Begin atoms for clipboard interaction
  Atom         selection_atom;
  Atom         utf8_atom;
  Atom         targets_atom;
  // End atoms for clipboard interaction
  XIM          im;
  GLXFBConfig *fbc;
  GLXFBConfig  best_fbc;
  bool         is_shm_supported;
  bool         is_randr_supported;
};

struct WinxNativeWindow {
  WinxNative       *winx;
  Window            window;
  u32               width;
  u32               height;
  WinxGraphicsMode  graphics_mode;
  XVisualInfo      *visual_info;
  GC                graphic_context;
  u32              *framebuffer;
  XImage           *image;
  XShmSegmentInfo   shm_info;
  GLXContext        gl_context;
  XIC               ic;
  XEvent            prev_x_event;
  u64               is_key_pressed[WinxKeyCodeCount];
  u32               last_char;
  time_t            start_nsecs;
  char             *clipboard_content;
  u64               clipboard_content_len;
  Atom              selection_target;
  bool              is_cursor_captured;
  bool              is_cursor_warping;
};

#endif // X11_WINX_H
