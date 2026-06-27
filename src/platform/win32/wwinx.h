#ifndef WIN32_WINX_H
#define WIN32_WINX_H

#include <windows.h>

#include "winx/key_code.h"
#include "../winx.h"
#include "shl/shl-defs.h"

struct WinxNative {
  HMODULE  instance;
};

struct WinxNativeWindow {
  WinxNative       *winx;
  HWND              window;
  WinxGraphicsMode  graphics_mode;
  HDC               device_ctx;
  u32              *framebuffer;
  HBITMAP           bitmap;
  HDC               bitmap_device_ctx;
  HGLRC             gl_context;
  u64               is_key_pressed[WinxKeyCodeCount];
  // u64               start_nanos;
  u64               start_millis;
};

#endif // WIN32_WINX_H
