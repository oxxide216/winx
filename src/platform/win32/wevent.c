#include <windowsx.h>

#include "wwinx.h"
#include "wkey_code.h"
#include "winx/winx.h"
#include "winx/event.h"
#include "../../shl_defs.h"
#include "../../wstr.h"

static Da(WinxEvent) winx_events = {0};

WinxEvent winx_native_get_event(WinxNativeWindow *window, bool wait) {
  WinxEvent winx_event = { WinxEventKindNone, {}, false };

  MSG msg;

  if (wait)
    GetMessageW(&msg, window->window, 0, 0);
  else if (!PeekMessageW(&msg, window->window, 0, 0, PM_REMOVE))
    return winx_event;

  TranslateMessage(&msg);
  DispatchMessageW(&msg);

  if (winx_events.len > 0) {
    winx_event = winx_events.items[--winx_events.len];

    if (winx_event.kind == WinxEventKindKeyPress) {
      if (window->is_key_pressed[winx_event.as.key.key_code]) {
        winx_event.kind = WinxEventKindKeyHold;
      } else {
        window->is_key_pressed[winx_event.as.key.key_code] = true;
        winx_event.kind = WinxEventKindKeyPress;
      }
    } else if (winx_event.kind == WinxEventKindKeyRelease) {
      window->is_key_pressed[winx_event.as.key.key_code] = false;
    }
  }

  return winx_event;
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
  WinxEvent winx_event = { WinxEventKindNone, {}, false };

  switch (message) {
  case WM_KEYDOWN: {
    winx_event.kind = WinxEventKindKeyPress;
    winx_event.as.key.key_code = virtual_key_to_key_code(w_param);
  } break;

  case WM_KEYUP: {
    winx_event.kind = WinxEventKindKeyRelease;
    winx_event.as.key.key_code = virtual_key_to_key_code(w_param);
  } break;

  case WM_CHAR: {
    winx_event.kind = WinxEventKindChar;
    winx_event.as._char._char = w_param;
  } break;

  case WM_LBUTTONDOWN: {
    winx_event.kind = WinxEventKindButtonPress;
    winx_event.as.button.button = WinxMouseButtonLeft;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_MBUTTONDOWN: {
    winx_event.kind = WinxEventKindButtonPress;
    winx_event.as.button.button = WinxMouseButtonMiddle;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_RBUTTONDOWN: {
    winx_event.kind = WinxEventKindButtonPress;
    winx_event.as.button.button = WinxMouseButtonRight;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_LBUTTONUP: {
    winx_event.kind = WinxEventKindButtonRelease;
    winx_event.as.button.button = WinxMouseButtonLeft;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_MBUTTONUP: {
    winx_event.kind = WinxEventKindButtonRelease;
    winx_event.as.button.button = WinxMouseButtonMiddle;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_RBUTTONUP: {
    winx_event.kind = WinxEventKindButtonRelease;
    winx_event.as.button.button = WinxMouseButtonRight;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_MOUSEWHEEL: {
    winx_event.kind = WinxEventKindButtonPress;
    winx_event.as.button.button = GET_WHEEL_DELTA_WPARAM(w_param) > 0 ?
                                  WinxMouseButtonWheelUp :
                                  WinxMouseButtonWheelDown;
    winx_event.as.button.x = GET_X_LPARAM(l_param);
    winx_event.as.button.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_MOUSEMOVE: {
    winx_event.kind = WinxEventKindMouseMove;
    winx_event.as.mouse_move.x = GET_X_LPARAM(l_param);
    winx_event.as.mouse_move.y = GET_Y_LPARAM(l_param);
  } break;

  case WM_SETFOCUS: {
    winx_event.kind = WinxEventKindFocus;
  } break;

  case WM_KILLFOCUS: {
    winx_event.kind = WinxEventKindUnfocus;
  } break;

  case WM_SIZE: {
    winx_event.kind = WinxEventKindResize;
    winx_event.as.resize.width = LOWORD(l_param);
    winx_event.as.resize.height = HIWORD(l_param);
  } break;

  case WM_DESTROY: {
    winx_event.kind = WinxEventKindQuit;
  } break;

  default: {
    return DefWindowProc(window, message, w_param, l_param);
  }
  }

  if (winx_event.kind != WinxEventKindNone)
    DA_APPEND(winx_events, winx_event);

  return 0;
}
