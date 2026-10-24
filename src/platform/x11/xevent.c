#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <wctype.h>
#include <limits.h>

#include "xwinx.h"
#include "xkey_code.h"
#include "winx/winx.h"
#include "winx/event.h"
#include "shl/shl-defs.h"
#include "../../wstr.h"

WinxEvent winx_native_get_event(WinxNativeWindow *window, bool wait) {
  WinxEvent winx_event = { WinxEventKindNone, {}, false };

  if (window->last_char) {
    winx_event.kind = WinxEventKindChar;
    winx_event.as._char._char = window->last_char;

    window->last_char = 0;

    return winx_event;
  }

  if (!wait && XPending(window->winx->display) == 0)
    return winx_event;

  XEvent x_event;
  XNextEvent(window->winx->display, &x_event);

  switch (x_event.type) {
  case KeyPress:
  case KeyRelease: {
    XSetICFocus(window->ic);

    KeySym keysym;

    if (x_event.type == KeyPress) {
      char key_name[5] = {0};
      x_event.xkey.state &= ~ControlMask;

      Status status;
      Xutf8LookupString(window->ic, &x_event.xkey, key_name,
                        ARRAY_LEN(key_name) - 1, &keysym, &status);

      if (status == XLookupChars || status == XLookupBoth) {
        mbstowcs((wchar_t *) &window->last_char, key_name, 1);

        switch (window->last_char) {
        case 32525:   // Enter
        case 32521:   // Tab
        case 32520:   // Backspace
        case 32639:   // Delete
        case 32539: { // Escape
          window->last_char = 0;
        } break;

        default: {
          if (!iswprint(window->last_char))
            window->last_char = 0;
        } break;
        }
      }
    } else {
      keysym = XLookupKeysym(&x_event.xkey, 0);
    }

    WinxKeyCode key_code = keysym_to_key_code(keysym);

    if (x_event.type == KeyPress) {
      if (window->is_key_pressed[key_code]) {
        winx_event.kind = WinxEventKindKeyHold;
      } else {
        window->is_key_pressed[key_code] = true;
        winx_event.kind = WinxEventKindKeyPress;
      }
    } else {
      window->is_key_pressed[key_code] = false;
      winx_event.kind = WinxEventKindKeyRelease;
    }

    winx_event.as.key = (WinxEventKey) { key_code };
  } break;

  case ButtonPress:
  case ButtonRelease: {
    WinxMouseButton button = x_event.xbutton.button - 1;
    if (button >= WinxMouseButtonCount)
      button = WinxMouseButtonUnknown;

    if (x_event.type == ButtonPress)
      winx_event.kind = WinxEventKindButtonPress;
    else
      winx_event.kind = WinxEventKindButtonRelease;

    winx_event.as.button = (WinxEventButton) {
      button,
      x_event.xbutton.x,
      x_event.xbutton.y,
    };
  } break;

  case MotionNotify: {
    if (window->is_cursor_warping &&
        (u32) x_event.xmotion.x == window->width / 2 &&
        (u32) x_event.xmotion.y == window->height / 2) {
      window->is_cursor_warping = false;
      break;
    }

    winx_event.kind = WinxEventKindMouseMove;
    winx_event.as.mouse_move = (WinxEventMouseMove) {
      x_event.xmotion.x,
      x_event.xmotion.y,
    };

    if (window->is_cursor_captured) {
      XWarpPointer(window->winx->display, None, window->window,
                   0, 0, 0, 0, window->width / 2, window->height / 2);
      window->is_cursor_warping = true;
    }
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

    winx_event.kind = WinxEventKindResize;
    winx_event.as.resize = (WinxEventResize) { new_width, new_height };

    window->width = new_width;
    window->height = new_height;
  } break;

  case ClientMessage: {
    if (x_event.xclient.data.l[0] == (i64) window->winx->wm_delete_window_atom)
      winx_event.kind = WinxEventKindQuit;
  } break;

  case SelectionRequest: {
    XSelectionRequestEvent request = x_event.xselectionrequest;
    if (XGetSelectionOwner(window->winx->display,
                           window->winx->selection_atom) == window->window &&
        request.selection == window->winx->selection_atom) {
      if (request.target == window->winx->targets_atom && request.property != None)
        XChangeProperty(request.display, request.requestor, request.property,
                        XA_ATOM, 32, PropModeReplace,
                        (u8 *) &window->winx->utf8_atom, 1);
      else if ((request.target == XA_STRING ||
                request.target == window->winx->utf8_atom) &&
               request.property != None &&
               window->clipboard_content)
        XChangeProperty(request.display, request.requestor, request.property,
                        request.target, 8, PropModeReplace,
                        (u8 *) window->clipboard_content,
                        window->clipboard_content_len);

      XSelectionEvent send_event;
      send_event.type = SelectionNotify;
      send_event.serial = request.serial;
      send_event.send_event = request.send_event;
      send_event.display = request.display;
      send_event.requestor = request.requestor;
      send_event.selection = request.selection;
      send_event.target = request.target;
      send_event.property = request.property;
      send_event.time = request.time;
      XSendEvent(window->winx->display, request.requestor, 0, 0, (XEvent *) &send_event);
    }
  } break;

  case SelectionNotify: {
    XSelectionEvent selection = x_event.xselection;
    if (selection.property != None) {
      Atom actual_type;
      int actual_format;
      u64 bytes_after;
      u8 *data;
      u64 len;
      XGetWindowProperty(window->winx->display, window->window,
                         window->winx->selection_atom, 0, LONG_MAX,
                         false, AnyPropertyType, &actual_type, &actual_format,
                         &len, &bytes_after, &data);

      if (selection.target == window->winx->targets_atom) {
        Atom* list = (Atom*) data;
        for (unsigned long i = 0; i < len; i++) {
          if (list[i] == XA_STRING) {
            window->selection_target = XA_STRING;
          } else if (list[i] == window->winx->utf8_atom) {
            window->selection_target = window->winx->utf8_atom;
            break;
          }
        }

        if (window->selection_target != None)
          XConvertSelection(window->winx->display, window->winx->selection_atom,
                            window->selection_target, window->winx->selection_atom,
                            window->window, CurrentTime);
      } else if (selection.target == window->selection_target) {
        char *_data = malloc(len);
        memcpy(_data, data, len);

        winx_event.kind = WinxEventKindClipboardData;
        winx_event.as.clipboard_data = (WinxEventClipboardData) { _data, len };
      }

      if (data)
        XFree(data);
    }
  } break;
  }

  window->prev_x_event = x_event;
  return winx_event;
}
