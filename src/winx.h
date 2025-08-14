#ifndef WINX_H
#define WINX_H

#include "shl_defs.h"
#include "shl_str.h"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

typedef struct {
  WinxNative *native;
} Winx;

typedef struct {
  Str               name;
  u32               width;
  u32               height;
  WinxNativeWindow *native;
} WinxWindow;

Winx       winx_init(void);
WinxWindow winx_create_window(Winx *winx, Str name, u32 width, u32 height);
void       winx_cleanup(Winx *winx);
void       winx_destroy_window(Winx *winx, WinxWindow *window);

#endif // WINX_H
