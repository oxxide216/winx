#ifndef EVENT_H
#define EVENT_H

#include "winx.h"
#include "../../src/key_code.h"
#include "../../src/wstr.h"

typedef enum {
  WinxEventKindNone = 0,
  WinxEventKindKeyPress,
  WinxEventKindKeyRelease,
  WinxEventKindKeyHold,
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
  WChar       _char;
} WinxEventKey;

typedef enum {
  WinxMouseButtonLeft = 0,
  WinxMouseButtonMiddle,
  WinxMouseButtonRight,
  WinxMouseButtonSide1,
  WinxMouseButtonSide2,
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
  WinxEventButton    button;
  WinxEventMouseMove mouse_move;
  WinxEventResize    resize;
} WinxEventAs;

typedef struct {
  WinxEventKind kind;
  WinxEventAs   as;
} WinxEvent;

WinxEvent winx_get_event(WinxWindow *window, bool wait);

#endif // EVENT_H
