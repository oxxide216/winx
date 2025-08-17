#include "event.h"
#include "platform/event.h"

WinxEvent winx_get_event(WinxWindow *window, bool wait) {
  return winx_native_get_event(window->native, wait);
}
