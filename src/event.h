#ifndef EVENT_H
#define EVENT_H

#include "winx.h"
#include "key_code.h"
#include "wstr.h"

typedef enum {
  WinxEventKindNone = 0,
  WinxEventKindKeyPress,
  WinxEventKindKeyRelease,
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
} WinxEventKeyPress;

typedef struct {
  WinxKeyCode key_code;
  WChar       _char;
} WinxEventKeyRelease;

typedef enum {
  MouseButtonLeft = 0,
  MouseButtonMiddle,
  MouseButtonRight,
  MouseButtonSide1,
  MouseButtonSide2,
} MouseButton;

typedef struct {
  MouseButton button;
} WinxEventButtonPress;

typedef struct {
  MouseButton button;
} WinxEventButtonRelease;

typedef struct {
  u32 x, y;
} WinxEventMouseMove;

typedef struct {
  u32 width, height;
} WinxEventResize;

typedef union {
  WinxEventKeyPress      key_press;
  WinxEventKeyRelease    key_release;
  WinxEventButtonPress   button_press;
  WinxEventButtonRelease button_release;
  WinxEventMouseMove     mouse_move;
  WinxEventResize        resize;
} WinxEventAs;

typedef struct {
  WinxEventKind kind;
  WinxEventAs   as;
} WinxEvent;

WinxEvent winx_get_event(WinxWindow *window, bool wait);

#endif // EVENT_H
