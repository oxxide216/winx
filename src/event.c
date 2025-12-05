#include "winx/event.h"
#include "platform/event.h"

WinxEvent winx_get_event(WinxWindow *window, bool wait) {
  WinxEvent event = winx_native_get_event(window->native, wait);

  if (event.kind == WinxEventKindResize) {
    window->width = event.as.resize.width;
    window->height = event.as.resize.height;
    if (window->framebuffer)
      winx_init_framebuffer(window);
  }

  return event;
}
