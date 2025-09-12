#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "xwinx.h"
#include "shl_defs.h"
#include "shl_str.h"
#include "shl_log.h"

#define EVENT_MASK (KeyPressMask |       \
                    KeyReleaseMask |     \
                    ButtonPressMask |    \
                    ButtonReleaseMask |  \
                    PointerMotionMask |  \
                    EnterWindowMask |    \
                    LeaveWindowMask |    \
                    StructureNotifyMask)

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  winx->display = XOpenDisplay(NULL);
  winx->screen = DefaultScreen(winx->display);
  winx->wm_delete_window = XInternAtom(winx->display, "WM_DELETE_WINDOW", false);
  winx->im = XOpenIM(winx->display, NULL, NULL, NULL);

  XSync(winx->display, false);

  return winx;
}

static XVisualInfo *winx_get_visual_info(WinxNative *winx, WinxGraphicsMode graphics_mode) {
  if (graphics_mode == WinxGraphicsModeFramebuffer) {
    i32 num_screens = 0;
    return XGetVisualInfo(winx->display, 0, NULL, &num_screens);
  } else if (graphics_mode == WinxGraphicsModeOpenGL) {
    GLint gl_visual_attributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    return glXChooseVisual(winx->display, 0, gl_visual_attributes);
  }

    ERROR("Wrong graphics mode\n");
    exit(1);
}

WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height,
                                          WinxGraphicsMode graphics_mode,
                                          WinxNativeWindow *parent) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));
  window->winx = winx;
  window->framebuffer = NULL;
  window->graphics_mode = graphics_mode;
  window->visual_info = winx_get_visual_info(winx, graphics_mode);

  XMatchVisualInfo(winx->display, winx->screen, 32, TrueColor, window->visual_info);

  Window parent_window;
  if (parent)
    parent_window = parent->window;
  else
    parent_window = RootWindow(winx->display, winx->screen);

  XSetWindowAttributes attributes = {
    .colormap = XCreateColormap(winx->display, parent_window,
                                window->visual_info->visual, AllocNone),
    .background_pixel = 0,
    .border_pixel = 0,
    .event_mask = EVENT_MASK,
  };

  window->window = XCreateWindow(winx->display, parent_window,
                                 0, 0, width, height, 0, window->visual_info->depth,
                                 InputOutput, window->visual_info->visual,
                                 CWBackPixel | CWColormap |
                                 CWBorderPixel | CWEventMask,
                                 &attributes);

  char *cstr_name = name.ptr;
  if (strlen(cstr_name) != name.len) {
    u8 *cstr_name = malloc(name.len + 1);
    memcpy(cstr_name, name.ptr, name.len);
    cstr_name[name.len] = '\0';
  }

  XStoreName(winx->display, window->window, cstr_name);
  XMapWindow(winx->display, window->window);
  XSetWMProtocols(winx->display, window->window, &winx->wm_delete_window, 1);

  u64 gcm = GCGraphicsExposures;
  XGCValues gcv;
  gcv.graphics_exposures = 0;
  window->graphic_context = XCreateGC(winx->display, window->window, gcm, &gcv);

  switch (graphics_mode) {
  case WinxGraphicsModeFramebuffer: {
    winx_native_init_framebuffer(window, width, height);
  } break;

  case WinxGraphicsModeOpenGL: {
    winx_native_init_gl_context(window);
  } break;
  }

  window->ic = XCreateIC(winx->im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, window->window, XNFocusWindow, window->window, NULL);
  XSetICFocus(window->ic);

  setlocale(LC_ALL, "");

  return window;
}

void winx_native_init_framebuffer(WinxNativeWindow *window, u32 width, u32 height) {
  if (window->framebuffer)
    free(window->framebuffer);

  u32 len = width * height;
  window->framebuffer = malloc(len * sizeof(u32));
  window->image = XCreateImage(window->winx->display, window->visual_info->visual,
                               window->visual_info->depth, ZPixmap,
                               0, (char *) window->framebuffer,
                               width, height, 8, width * sizeof(u32));
}

u32 *winx_native_get_framebuffer(WinxNativeWindow *window) {
  return window->framebuffer;
}

void winx_native_init_gl_context(WinxNativeWindow *window) {
  window->gl_context = glXCreateContext(window->winx->display,
                                        window->visual_info,
                                        NULL, GL_TRUE);
  glXMakeCurrent(window->winx->display, window->window, window->gl_context);
}

void winx_native_draw(WinxNativeWindow *window, u32 width, u32 height) {
  if (window->graphics_mode == WinxGraphicsModeFramebuffer)
    XPutImage(window->winx->display, window->window, window->graphic_context,
              window->image, 0, 0, 0, 0, width, height);
  else if (window->graphics_mode == WinxGraphicsModeOpenGL)
    glXSwapBuffers(window->winx->display, window->window);
}

void winx_native_destroy_window(WinxNativeWindow *window) {
  if (window->graphics_mode == WinxGraphicsModeFramebuffer) {
    XDestroyImage(window->image);
  } else if (window->graphics_mode == WinxGraphicsModeOpenGL) {
    glXMakeCurrent(window->winx->display, None, NULL);
    glXDestroyContext(window->winx->display, window->gl_context);
  }

  XFreeGC(window->winx->display, window->graphic_context);
  XUnmapWindow(window->winx->display, window->window);
  XDestroyWindow(window->winx->display, window->window);
}

void winx_native_cleanup(WinxNative *winx) {
  XCloseDisplay(winx->display);
}
