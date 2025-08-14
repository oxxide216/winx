#include <xcb/xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <stdlib.h>

#include "./winx.h"
#include "shl_defs.h"
#include "shl_str.h"
#include "shl_log.h"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

#define EVENT_MASK (XCB_EVENT_MASK_KEY_PRESS |        \
                    XCB_EVENT_MASK_KEY_RELEASE |      \
                    XCB_EVENT_MASK_BUTTON_PRESS |     \
                    XCB_EVENT_MASK_BUTTON_RELEASE |   \
                    XCB_EVENT_MASK_ENTER_WINDOW |     \
                    XCB_EVENT_MASK_LEAVE_WINDOW |     \
                    XCB_EVENT_MASK_POINTER_MOTION |   \
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                    XCB_EVENT_MASK_RESIZE_REDIRECT)

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  winx->connection = xcb_connect(NULL, NULL);
  winx->screen = xcb_setup_roots_iterator(xcb_get_setup(winx->connection)).data;
  winx->key_symbols = xcb_key_symbols_alloc(winx->connection);
  winx->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  return winx;
}

WinxNativeWindow *winx_create_native_window(WinxNative *winx, Str name, u32 width, u32 height) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));

  u32 window_mask = XCB_CW_EVENT_MASK;
  u32 window_value[] = { EVENT_MASK };
  window->window = xcb_generate_id(winx->connection);
  xcb_create_window(winx->connection, XCB_COPY_FROM_PARENT,
                    window->window, winx->screen->root,
                    0, 0, width, height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    winx->screen->root_visual,
                    window_mask, window_value);

  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(winx->connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(winx->connection, cookie, NULL);
  winx->wm_protocols = reply->atom;
  if (winx->wm_protocols == 0) {
    ERROR("Could not get `WM_PROTOCOLS` atom\n");
    exit(1);
  }
  free(reply);

  cookie = xcb_intern_atom(winx->connection, 1, 16, "WM_DELETE_WINDOW");
  reply = xcb_intern_atom_reply(winx->connection, cookie, NULL);
  winx->wm_delete_window = reply->atom;
  if (winx->wm_delete_window == 0) {
    ERROR("Could not get `WM_DELETE_WINDOW` atom\n");
    exit(1);
  }
  free(reply);

  xcb_change_property(winx->connection, XCB_PROP_MODE_REPLACE, window->window,
                      winx->wm_protocols, XCB_ATOM_ATOM, 32, 1, &winx->wm_delete_window);

  xcb_change_property(winx->connection, XCB_PROP_MODE_REPLACE,
                      window->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                      8, name.len, name.ptr);

  xcb_map_window(winx->connection, window->window);

  u32 context_mask = XCB_GC_FOREGROUND;
  u32 context_value[] = { winx->screen->black_pixel };
  window->context = xcb_generate_id(winx->connection);
  xcb_create_gc(winx->connection, window->context, winx->screen->root,
                context_mask, context_value);

  xcb_flush(winx->connection);

  return window;
}

void winx_native_cleanup(WinxNative *winx) {
  xkb_context_unref(winx->xkb_context);
  xcb_key_symbols_free(winx->key_symbols);
  xcb_disconnect(winx->connection);
}

void winx_destroy_native_window(WinxNative *winx, WinxNativeWindow *window) {
  xcb_destroy_window(winx->connection, window->window);
}
