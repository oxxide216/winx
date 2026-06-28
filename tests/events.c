#include <wchar.h>
#include <errno.h>

#include "winx/winx.h"
#include "winx/event.h"
#include "shl/shl-log.h"
#include "shl/shl-defs.h"

static char *event_names[] = {
  "WinxEventKindNone",
  "WinxEventKindKeyPress",
  "WinxEventKindKeyRelease",
  "WinxEventKindKeyHold",
  "WinxEventKindChar",
  "WinxEventKindButtonPress",
  "WinxEventKindButtonRelease",
  "WinxEventKindMouseMove",
  "WinxEventKindFocus",
  "WinxEventKindUnfocus",
  "WinxEventKindResize",
  "WinxEventKindQuit",
};

i32 main(void) {
  Winx *winx = winx_init();
  WinxWindow *window = winx_init_window(winx, STR_LIT("Hello, world!"),
                                        640, 480, WinxGraphicsModeFramebuffer,
                                        NULL);

  bool is_running = true;

  while (is_running) {
    WinxEvent event;
    while ((event = winx_get_event(window, false)).kind != WinxEventKindNone) {
      puts(event_names[event.kind]);
      is_running = event.kind != WinxEventKindQuit;
      if (!is_running)
        break;
    }

    winx_draw(window);
  }

  winx_destroy_window(window);
  winx_cleanup(winx);
  return 0;
}
