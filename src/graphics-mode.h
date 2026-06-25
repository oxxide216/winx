#ifndef GRAPHICS_MODE_H
#define GRAPHICS_MODE_H

#ifndef WINX_GL_MAJOR_VERSION
#define WINX_GL_MAJOR_VERSION 3
#define WINX_GL_MINOR_VERSION 3
#endif

typedef enum {
  WinxGraphicsModeFramebuffer = 0,
  WinxGraphicsModeOpenGL,
} WinxGraphicsMode;

#endif // GRAPHICS_MODE_H
