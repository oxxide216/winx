#ifndef X11_WINX_H
#define X11_WINX_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>

struct WinxNative {
  xcb_connection_t   *connection;
  xcb_screen_t       *screen;
  xcb_atom_t          wm_protocols;
  xcb_atom_t          wm_delete_window;
  xcb_key_symbols_t  *key_symbols;
  struct xkb_context *xkb_context;
};

struct WinxNativeWindow {
  xcb_window_t      window;
  xcb_gcontext_t    context;
};

#endif // X11_WINX_H
