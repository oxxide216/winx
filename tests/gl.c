#include <GL/gl.h>

#include "winx.h"
#include "event.h"
#include "shl_log.h"
#include "shl_defs.h"

bool process_event(WinxEvent *event) {
  if (event->kind == WinxEventKindQuit)
    return false;
  else if (event->kind == WinxEventKindResize)
    glViewport(0, 0, event->as.resize.width, event->as.resize.height);

  return true;
}

int main(void) {
  Winx winx = winx_init();
  WinxWindow window = winx_init_window(&winx, STR_LIT("Hello, world!"),
                                       640, 480, WinxGraphicsModeOpenGL,
                                       NULL);

  bool is_running = true;

  while (is_running) {
    WinxEvent event;
    while ((event = winx_get_event(&window, false)).kind != WinxEventKindNone) {
      is_running = process_event(&event);
      if (!is_running)
        break;
    }

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);
      glColor3f(1.0, 0.0, 0.0); glVertex3f(-0.5, -0.5, 0.0);
      glColor3f(0.0, 1.0, 0.0); glVertex3f( 0.5, -0.5, 0.0);
      glColor3f(0.0, 0.0, 1.0); glVertex3f( 0.5,  0.5, 0.0);
      glColor3f(1.0, 1.0, 0.0); glVertex3f(-0.5,  0.5, 0.0);
    glEnd();

    winx_draw(&window);
  }

  winx_destroy_window(&window);
  winx_cleanup(&winx);
  return 0;
}
