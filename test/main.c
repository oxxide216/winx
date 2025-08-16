#include "winx.h"
#include "event.h"
#include "shl_log.h"

int main(void) {
  Winx winx = winx_init();
  WinxWindow window = winx_create_window(&winx, STR_LIT("Hello, world!"), 640, 480);

  while (true) {
    WinxEvent event = winx_get_event(&winx, false);
    if (event.kind != WinxEventKindNone) {
      if (event.kind == WinxEventKindQuit) {
        break;
      } else if (event.kind == WinxEventKindKeyPress &&
                 event.as.key_press._char != 0) {
        INFO("Pressed\n");
        wputc(event.as.key_press._char, stdout);
        printf(":%u\n", event.as.key_press._char);
        //putc('\n', stdout);
      } else if (event.kind == WinxEventKindKeyRelease &&
                 event.as.key_release._char != 0) {
        INFO("Released\n");
      }
    }
  }

  winx_destroy_window(&winx, &window);
  winx_cleanup(&winx);
  return 0;
}
