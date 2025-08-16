#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <wctype.h>

#include "xwinx.h"
#include "xkey_code.h"
#include "winx.h"
#include "event.h"
#include "shl_defs.h"
#include "shl_log.h"
#include "wstr.h"

static u32 non_printable_wchars[] = {
  32512, // Special keys
  32539, // Caps Lock
  32520, // Backspace
  32639, // Delete
};

#define WIDE_CHAR_NONE0 32512
#define WIDE_CHAR_NONE1 32539

WinxEvent winx_native_get_event(WinxNative *winx, bool wait) {
  WinxEvent winx_event = { WinxEventKindNone, {} };

  if (!wait && XPending(winx->display) == 0)
    return winx_event;

  XEvent x_event;
  XNextEvent(winx->display, &x_event);

  switch (x_event.type) {
  case KeyPress:
  case KeyRelease: {
    char key_name[4];
    KeySym keysym;
    i32 key_name_len = XLookupString(&x_event.xkey, key_name, ARRAY_LEN(key_name), &keysym, NULL);

    if (key_name_len > 4)
      return winx_event;

    u32 wchar = *(u32 *) key_name;

    if (wchar == 32525) { // Enter
      wchar = '\n';
    } else if (!iswprint(wchar)) {
      wchar = 0;
    } else {
      for (u32 i = 0; i < ARRAY_LEN(non_printable_wchars); ++i) {
        if (wchar == non_printable_wchars[i]) {
          wchar = 0;
          break;
        }
      }
    }

    if (x_event.type == KeyPress) {
      winx_event.kind = WinxEventKindKeyPress;
      winx_event.as.key_press = (WinxEventKeyPress) {
        keysym_to_key_code(keysym),
        wchar,
      };
    } else {
      winx_event.kind = WinxEventKindKeyRelease;
      winx_event.as.key_release = (WinxEventKeyRelease) {
        keysym_to_key_code(keysym),
        wchar,
      };
    }
  } break;

  case ClientMessage: {
    if (x_event.xclient.data.l[0] == (i64) winx->wm_delete_window)
      winx_event.kind = WinxEventKindQuit;
  } break;
  }

  return winx_event;
}
