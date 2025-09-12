#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <wctype.h>

#include "xwinx.h"
#include "xkey_code.h"
#include "winx.h"
#include "event.h"
#include "shl_defs.h"
#include "wstr.h"

static u32 non_printable_wchars[] = {
  32512, // Special keys
  32539, // Caps Lock
  32520, // Backspace
  32639, // Delete
};

WinxEvent winx_native_get_event(WinxNativeWindow *window, bool wait) {
  WinxEvent winx_event = { WinxEventKindNone, {} };

  if (!wait && XPending(window->winx->display) == 0)
    return winx_event;

  XEvent x_event;
  XNextEvent(window->winx->display, &x_event);

  switch (x_event.type) {
  case KeyPress:
  case KeyRelease: {
    XSetICFocus(window->ic);

    char key_name[4];
    KeySym keysym;
    Status status;
    u32 key_name_len = Xutf8LookupString(window->ic, &x_event.xkey, key_name,
                                        ARRAY_LEN(key_name), &keysym, &status);

    WChar wchar = '\0';
    if (status == XLookupChars || status == XLookupBoth)
      wchar = *(WChar *) key_name;

    if (!iswprint(wchar)) {
      switch (wchar) {
      case 32525:  // Enter
      case 32521:  // Tab
      case 32520:  // Backspace
      case 32639: // Delete
      case 32539: // Escape
        break;

      default: {
        wchar = '\0';
      } break;
      }
    }

    if (x_event.type == KeyPress) {
      winx_event.kind = WinxEventKindKeyPress;
      winx_event.as.key_press = (WinxEventKeyPress) {
        keysym_to_key_code(keysym),
        wchar,
      };
    } else {
      bool is_repeat = window->prev_x_event.xkey.time == x_event.xkey.time &&
                       window->prev_x_event.xkey.keycode == x_event.xkey.keycode;

      if (is_repeat) {
        winx_event.kind = WinxEventKindKeyHold;
        winx_event.as.key_hold = (WinxEventKeyHold) {
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
    }
  } break;

  case ButtonPress:
  case ButtonRelease: {
    WinxMouseButton button = x_event.xbutton.button - 1;
    if (button >= WinxMouseButtonCount)
      button = WinxMouseButtonUnknown;

    if (x_event.type == ButtonPress) {
      winx_event.kind = WinxEventKindButtonPress;
      winx_event.as.button_press = (WinxEventButtonPress) {
        button,
        x_event.xbutton.x,
        x_event.xbutton.y,
      };
    } else {
      winx_event.kind = WinxEventKindButtonRelease;
      winx_event.as.button_release = (WinxEventButtonRelease) {
        button,
        x_event.xbutton.x,
        x_event.xbutton.y,
      };
    }
  } break;

  case MotionNotify: {
    winx_event.kind = WinxEventKindMouseMove;
    winx_event.as.mouse_move = (WinxEventMouseMove) {
      x_event.xmotion.x,
      x_event.xmotion.y,
    };
  } break;

  case EnterNotify: {
    winx_event.kind = WinxEventKindFocus;
  } break;

  case LeaveNotify: {
    winx_event.kind = WinxEventKindUnfocus;
  } break;

  case ConfigureNotify: {
    u32 new_width = x_event.xconfigure.width;
    u32 new_height = x_event.xconfigure.height;

    XResizeWindow(window->winx->display, window->window,
                  new_width, new_height);

    winx_event.kind = WinxEventKindResize;
    winx_event.as.resize = (WinxEventResize) { new_width, new_height };
  } break;

  case ClientMessage: {
    if (x_event.xclient.data.l[0] == (i64) window->winx->wm_delete_window)
      winx_event.kind = WinxEventKindQuit;
  } break;
  }

  window->prev_x_event = x_event;
  return winx_event;
}
