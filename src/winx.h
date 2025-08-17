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
  u32              *framebuffer;
  WinxNativeWindow *native;
} WinxWindow;

Winx       winx_init(void);
WinxWindow winx_init_window(Winx *winx, Str name, u32 width, u32 height);
void       winx_init_framebuffer(WinxWindow *window);
void       winx_redraw(WinxWindow *window);
void       winx_destroy_window(WinxWindow *window);
void       winx_cleanup(Winx *winx);

#endif // WINX_H
