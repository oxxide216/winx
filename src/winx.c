#include <stdlib.h>

#include "winx.h"
#include "platform/winx.h"

Winx winx_init(void) {
  Winx winx;
  winx.native = winx_native_init();
  return winx;
}

WinxWindow winx_create_window(Winx *winx, Str name, u32 width, u32 height) {
  WinxWindow window;
  window.name = name;
  window.width = width;
  window.height = height;
  window.native = winx_create_native_window(winx->native, name, width, height);
  return window;
}

void winx_cleanup(Winx *winx) {
  winx_native_cleanup(winx->native);
  free(winx->native);
}

void winx_destroy_window(Winx *winx, WinxWindow *window) {
  winx_destroy_native_window(winx->native, window->native);
  free(window->native);
}
