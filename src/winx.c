#include <stdlib.h>

#include "winx.h"
#include "platform/winx.h"

Winx winx_init(void) {
  Winx winx;
  winx.native = winx_native_init();
  return winx;
}

WinxWindow winx_init_window(Winx *winx, Str name,
                            u32 width, u32 height,
                            WinxGraphicsMode graphics_mode,
                            WinxWindow *parent) {
  WinxWindow window;
  window.name = name;
  window.width = width;
  window.height = height;

  WinxNativeWindow *native_parent = NULL;
  if (parent)
    native_parent = parent->native;

  window.native = winx_native_init_window(winx->native, name,
                                          width, height,
                                          graphics_mode,
                                          native_parent);

  return window;
}

void winx_init_framebuffer(WinxWindow *window) {
  winx_native_init_framebuffer(window->native, window->width, window->height);
}

u32 *winx_get_framebuffer(WinxWindow *window) {
  return winx_native_get_framebuffer(window->native);
}

void winx_draw(WinxWindow *window) {
  winx_native_draw(window->native, window->width, window->height);
}

void winx_destroy_window(WinxWindow *window) {
  winx_native_destroy_window(window->native);
  free(window->native);
}

void winx_cleanup(Winx *winx) {
  winx_native_cleanup(winx->native);
  free(winx->native);
}
