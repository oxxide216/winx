#ifndef WINX_H
#define WINX_H

#include "../../src/graphics-mode.h"
#include "shl/shl-defs.h"
#include "shl/shl-str.h"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

typedef struct {
  WinxNative *native;
} Winx;

typedef struct {
  Str               name;
  u32               width;
  u32               height;
  f32               time;
  f32               delta_time;
  f32               fps;
  WinxNativeWindow *native;
} WinxWindow;

typedef void (*WinxApiProc)(void);

Winx       *winx_init(void);
WinxWindow *winx_init_window(Winx *winx, Str name,
                             u32 width, u32 height,
                             WinxGraphicsMode graphics_mode,
                             WinxWindow *parent);
u32        *winx_get_framebuffer(WinxWindow *window);
void        winx_make_context_current(WinxWindow *window);
void        winx_draw(WinxWindow *window);
void        winx_destroy_window(WinxWindow *window);
void        winx_cleanup(Winx *winx);

WinxApiProc winx_load_proc_address(const char *name);

#endif // WINX_H
