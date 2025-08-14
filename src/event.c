#include "event.h"
#include "platform/event.h"

WinxEvent winx_get_event(Winx *winx, bool wait) {
  return winx_native_get_event(winx->native, wait);
}
