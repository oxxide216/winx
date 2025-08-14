#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

#include "event.h"
#include "winx.h"
#include "./winx.h"
#include "shl_defs.h"
#include "shl_log.h"
#include "wstr.h"

WinxEvent winx_native_get_event(WinxNative *winx, bool wait) {
  xcb_generic_event_t *xcb_event = NULL;
  if (wait)
    xcb_event = xcb_wait_for_event(winx->connection);
  else
    xcb_event = xcb_poll_for_event(winx->connection);

  WinxEvent winx_event = { WinxEventKindNone, {} };

  if (!xcb_event)
    return winx_event;

  switch (XCB_EVENT_RESPONSE_TYPE(xcb_event)) {
  case XCB_KEY_PRESS: {
    xcb_key_press_event_t *key_press_event =
      (xcb_key_press_event_t *) xcb_event;

    xcb_keysym_t keysym = xcb_key_press_lookup_keysym(winx->key_symbols,
                                                      key_press_event, 0);
    WChar _char = xkb_keysym_to_utf32(keysym);

    winx_event.kind = WinxEventKindKeyPress;
    winx_event.as.key_press = (WinxEventKeyPress) {
      keysym_to_key_code(keysym),
      _char,
    };
  } break;

  case XCB_KEY_RELEASE: {
    xcb_key_release_event_t *key_release_event =
      (xcb_key_release_event_t *) xcb_event;

    xcb_keysym_t keysym = xcb_key_release_lookup_keysym(winx->key_symbols,
                                                        key_release_event, 0);
    WChar _char = xkb_keysym_to_utf32(keysym);

    winx_event.kind = WinxEventKindKeyRelease;
    winx_event.as.key_release = (WinxEventKeyRelease) {
      keysym_to_key_code(keysym),
      _char,
    };
  } break;

  case XCB_BUTTON_PRESS: {
    xcb_button_press_event_t *button_press_event =
      (xcb_button_press_event_t *) xcb_event;

    WinxEventButtonPress button_press;
    if (button_press_event->state | XCB_BUTTON_MASK_1)
      button_press.button = MouseButtonLeft;
    else if (button_press_event->state | XCB_BUTTON_MASK_2)
      button_press.button = MouseButtonMiddle;
    else if (button_press_event->state | XCB_BUTTON_MASK_3)
      button_press.button = MouseButtonRight;
    else if (button_press_event->state | XCB_BUTTON_MASK_4)
      button_press.button = MouseButtonSide1;
    else if (button_press_event->state | XCB_BUTTON_MASK_5)
      button_press.button = MouseButtonSide2;

    winx_event.kind = WinxEventKindButtonPress;
    winx_event.as.button_press = button_press;
  } break;

  case XCB_BUTTON_RELEASE: {
    xcb_button_release_event_t *button_release_event =
      (xcb_button_release_event_t *) xcb_event;

    WinxEventButtonRelease button_release = {0};
    if (button_release_event->state & XCB_BUTTON_MASK_1)
      button_release.button = MouseButtonLeft;
    else if (button_release_event->state & XCB_BUTTON_MASK_2)
      button_release.button = MouseButtonMiddle;
    else if (button_release_event->state & XCB_BUTTON_MASK_3)
      button_release.button = MouseButtonRight;
    else if (button_release_event->state & XCB_BUTTON_MASK_4)
      button_release.button = MouseButtonSide1;
    else if (button_release_event->state & XCB_BUTTON_MASK_5)
      button_release.button = MouseButtonSide2;

    winx_event.kind = WinxEventKindButtonRelease;
    winx_event.as.button_release = button_release;
  } break;

  case XCB_ENTER_NOTIFY: {
    winx_event.kind = WinxEventKindFocus;
  } break;

  case XCB_LEAVE_NOTIFY: {
    winx_event.kind = WinxEventKindUnfocus;
  } break;

  case XCB_RESIZE_REQUEST: {
    xcb_resize_request_event_t *resize_request_event =
      (xcb_resize_request_event_t *) xcb_event;

    winx_event.kind = WinxEventKindResize;
    winx_event.as.resize = (WinxEventResize) {
      resize_request_event->width,
      resize_request_event->height,
    };
  } break;

    case XCB_CLIENT_MESSAGE: {
    xcb_client_message_event_t *client_message_event =
      (xcb_client_message_event_t *) xcb_event;

    if (client_message_event->data.data32[0] == winx->wm_delete_window)
      winx_event.kind = WinxEventKindQuit;
  } break;
  }

  return winx_event;
}
