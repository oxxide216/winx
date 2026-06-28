#include "winx/winx.h"
#include "winx/event.h"
#include "shl/shl-log.h"
#include "shl/shl-defs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
  u32 *data;
  u32  width;
  u32  height;
} ViewedImage;

u32 min(u32 a, u32 b) {
  return a <= b ? a : b;
}

void render(WinxWindow *window, ViewedImage *image) {
  u32 *framebuffer = winx_get_framebuffer(window);
  for (u32 y = 0; y < min(window->height, image->height); ++y) {
    u32 sy = y * image->width;
    u32 dy = y * window->width;
    for (u32 x = 0; x < min(window->width, image->width); ++x)
      framebuffer[dy + x] = image->data[sy + x];
  }
}

i32 main(i32 argc, char **argv) {
  if (argc < 2) {
    ERROR("No input image was provided!\n");
    return 1;
  }

  ViewedImage image;
  image.data = (u32 *) stbi_load(argv[1], (i32 *) &image.width,
                                 (i32 *) &image.height, NULL, 4);

  Winx *winx = winx_init();
  WinxWindow *window = winx_init_window(winx, STR_LIT("Image Viewer"),
                                        image.width, image.height,
                                        WinxGraphicsModeFramebuffer,
                                        NULL);

  bool is_running = true;

  while (is_running) {
    WinxEvent event;
    while ((event = winx_get_event(window, false)).kind != WinxEventKindNone) {
      is_running = event.kind != WinxEventKindQuit;
      if (!is_running)
        break;
    }

    render(window, &image);
    winx_draw(window);
  }

  winx_destroy_window(window);
  winx_cleanup(winx);
  free(image.data);
  return 0;
}
