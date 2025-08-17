#include "winx.h"
#include "event.h"
#include "shl_log.h"
#include "shl_defs.h"

static u32 colors[] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };

void fill_window_framebuffer(WinxWindow *window, u32 color_index) {
  for (u32 i = 0; i < window->width * window->height; ++i)
      window->framebuffer[i] = colors[color_index];
}

bool process_event(WinxEvent *event, u32 *color_index) {
  if (event->kind == WinxEventKindQuit) {
    return false;
  }  else if (event->kind == WinxEventKindKeyPress) {
    if (event->as.key_press.key_code == WinxKeyCodeRight) {
      if (++*color_index >= ARRAY_LEN(colors))
        *color_index = 0;
    } else if (event->as.key_press.key_code == WinxKeyCodeLeft) {
      if (*color_index == 0)
        *color_index = ARRAY_LEN(colors) - 1;
      else
        --*color_index;
    }
  }

  return true;
}

int main(void) {
  Winx winx = winx_init();
  WinxWindow window = winx_init_window(&winx, STR_LIT("Hello, world!"), 640, 480);
  winx_init_framebuffer(&window);

  u32 color_index = 0;
  bool is_running = true;

  while (is_running) {
    WinxEvent event;
    while ((event = winx_get_event(&window, false)).kind != WinxEventKindNone) {
      is_running = process_event(&event, &color_index);
      if (!is_running)
        break;
    }

    fill_window_framebuffer(&window, color_index);
    winx_draw(&window);
  }

  winx_destroy_window(&window);
  winx_cleanup(&winx);
  return 0;
}
