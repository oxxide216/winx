#include <stdlib.h>

#include "winx/winx.h"
#include "platform/winx.h"

Winx *winx_init(void) {
  WinxNative *native = winx_native_init();

  if (!native)
    return NULL;

  Winx *winx = malloc(sizeof(Winx));
  winx->native = native;

  return winx;
}

WinxWindow *winx_init_window(Winx *winx, Str name,
                             u32 width, u32 height,
                             WinxGraphicsMode graphics_mode,
                             WinxWindow *parent) {
  WinxNativeWindow *native_parent = NULL;
  if (parent)
    native_parent = parent->native;

  WinxNativeWindow *native = winx_native_init_window(winx->native, name,
                                                     width, height,
                                                     graphics_mode,
                                                     native_parent);

  if (!native)
    return NULL;

  WinxWindow *window = malloc(sizeof(WinxWindow));
  window->name = name;
  window->width = width;
  window->height = height;
  window->time = 0.0;
  window->delta_time = 0.0;
  window->fps = 0.0;
  window->native = native;

  return window;
}

u32 *winx_get_framebuffer(WinxWindow *window) {
  return winx_native_get_framebuffer(window->native);
}

void winx_make_context_current(WinxWindow *window) {
  winx_native_make_context_current(window->native);
}

f32 winx_get_refresh_rate(WinxWindow *window) {
  return winx_native_get_refresh_rate(window->native);
}

void winx_draw(WinxWindow *window) {
  winx_native_draw(window->native, window->width, window->height);

  f32 time = winx_native_get_time(window->native);
  if (window->target_fps > 0.0) {
    f32 delta = 1.0 / window->target_fps + window->time - time;
    if (delta > 0.0) {
      winx_native_sleep((u32) (delta * 1000.0));
      time += delta;
    }
  }

  window->delta_time = time - window->time;
  window->fps = 1.0 / window->delta_time;
  window->time = time;
}

void winx_destroy_window(WinxWindow *window) {
  winx_native_destroy_window(window->native);
  free(window->native);
  free(window);
}

void winx_cleanup(Winx *winx) {
  winx_native_cleanup(winx->native);
  free(winx->native);
  free(winx);
}

WinxApiProc winx_load_proc_address(const char *name) {
  return winx_native_load_proc_address(name);
}
