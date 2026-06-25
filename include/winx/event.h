#ifndef EVENT_H
#define EVENT_H

#include "winx.h"
#include "key_code.h"
#include "../../src/wstr.h"

typedef enum {
  WinxEventKindNone = 0,
  WinxEventKindKeyPress,
  WinxEventKindKeyRelease,
  WinxEventKindKeyHold,
  WinxEventKindChar,
  WinxEventKindButtonPress,
  WinxEventKindButtonRelease,
  WinxEventKindMouseMove,
  WinxEventKindFocus,
  WinxEventKindUnfocus,
  WinxEventKindResize,
  WinxEventKindQuit,
} WinxEventKind;

typedef u32 WChar;

typedef struct {
  WinxKeyCode key_code;
} WinxEventKey;

typedef struct {
  WChar _char;
} WinxEventChar;

typedef enum {
  WinxMouseButtonLeft = 0,
  WinxMouseButtonMiddle,
  WinxMouseButtonRight,
  WinxMouseButtonWheelUp,
  WinxMouseButtonWheelDown,
  WinxMouseButtonCount,
  WinxMouseButtonUnknown,
} WinxMouseButton;

typedef struct {
  WinxMouseButton button;
  u32             x, y;
} WinxEventButton;

typedef struct {
  u32 x, y;
} WinxEventMouseMove;

typedef struct {
  u32 width, height;
} WinxEventResize;

typedef union {
  WinxEventKey       key;
  WinxEventChar      _char;
  WinxEventButton    button;
  WinxEventMouseMove mouse_move;
  WinxEventResize    resize;
} WinxEventAs;

typedef struct {
  WinxEventKind kind;
  WinxEventAs   as;
  bool          was_processed;
} WinxEvent;

WinxEvent winx_get_event(WinxWindow *window, bool wait);

#endif // EVENT_H
