#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "xwinx.h"
#include "../../shl_defs.h"
#include "../../shl_str.h"
#include "../../shl_log.h"

#define EVENT_MASK (KeyPressMask |       \
                    KeyReleaseMask |     \
                    ButtonPressMask |    \
                    ButtonReleaseMask |  \
                    PointerMotionMask |  \
                    EnterWindowMask |    \
                    LeaveWindowMask |    \
                    StructureNotifyMask)

#define WINX_GL_MAJOR_VERSION 3
#define WINX_GL_MINOR_VERSION 3

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig,
                                                     GLXContext, bool, const i32 *);

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  memset(winx, 0, sizeof(WinxNative));
  winx->display = XOpenDisplay(NULL);
  winx->screen = DefaultScreen(winx->display);
  winx->wm_delete_window = XInternAtom(winx->display, "WM_DELETE_WINDOW", false);
  winx->im = XOpenIM(winx->display, NULL, NULL, NULL);

  XkbSetDetectableAutoRepeat(winx->display, true, NULL);

  return winx;
}

static XVisualInfo *winx_get_visual_info(WinxNative *winx, WinxGraphicsMode graphics_mode) {
  if (graphics_mode == WinxGraphicsModeFramebuffer) {
    i32 num_screens = 0;
    return XGetVisualInfo(winx->display, 0, NULL, &num_screens);
  } else if (graphics_mode == WinxGraphicsModeOpenGL) {
    if (winx->fbc)
      return glXGetVisualFromFBConfig(winx->display, winx->best_fbc);

    static i32 gl_visual_attributes[] = {
      GLX_X_RENDERABLE,  True,
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_RENDER_TYPE,   GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
      GLX_RED_SIZE,      8,
      GLX_GREEN_SIZE,    8,
      GLX_BLUE_SIZE,     8,
      GLX_ALPHA_SIZE,    8,
      GLX_DEPTH_SIZE,    24,
      GLX_STENCIL_SIZE,  8,
      GLX_DOUBLEBUFFER,  True,
      None
    };

    i32 fb_count;
    GLXFBConfig* fbc = glXChooseFBConfig(winx->display, winx->screen,
                                         gl_visual_attributes, &fb_count);

    i32 best_fbc = -1, worst_fbc = -1, best_num_sampler = -1, worst_num_sampler = 999;

    for (u32 i = 0; i < (u32) fb_count; ++i) {
      XVisualInfo *vi = glXGetVisualFromFBConfig(winx->display, fbc[i]);

      i32 sample_buffer, samples;
      glXGetFBConfigAttrib(winx->display, fbc[i], GLX_SAMPLE_BUFFERS, &sample_buffer);
      glXGetFBConfigAttrib(winx->display, fbc[i], GLX_SAMPLES,        &samples);

      if (best_fbc < 0 || (sample_buffer && samples > best_num_sampler))
        best_fbc = i, best_num_sampler = samples;
      if (worst_fbc < 0 || !sample_buffer || samples < worst_num_sampler)
        worst_fbc = i, worst_num_sampler = samples;

      XFree(vi);
    }

    winx->fbc = fbc;
    winx->best_fbc = fbc[best_fbc];

    return glXGetVisualFromFBConfig(winx->display, winx->best_fbc);
  }

  ERROR("Wrong graphics mode\n");
  exit(1);
}

WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height,
                                          WinxGraphicsMode graphics_mode,
                                          WinxNativeWindow *parent) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));
  memset(window, 0, sizeof(WinxNativeWindow));
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
    .border_pixel = 0,
    .event_mask = EVENT_MASK,
  };

  window->window = XCreateWindow(winx->display, parent_window,
                                 0, 0, width, height, 0, window->visual_info->depth,
                                 InputOutput, window->visual_info->visual,
                                 CWColormap | CWBorderPixel | CWEventMask,
                                 &attributes);

  window->ic = XCreateIC(winx->im,
                         XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, window->window,
                         XNFocusWindow, window->window,
                         NULL);

  char *cstr_name = malloc(name.len + 1);
  memcpy(cstr_name, name.ptr, name.len);
  cstr_name[name.len] = '\0';

  XStoreName(winx->display, window->window, cstr_name);
  XMapWindow(winx->display, window->window);
  XSetWMProtocols(winx->display, window->window, &winx->wm_delete_window, 1);

  free(cstr_name);

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
  if (window->framebuffer) {
    free(window->framebuffer);
    XDestroyImage(window->image);
  }

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
  static i32 context_attributes[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, WINX_GL_MAJOR_VERSION,
    GLX_CONTEXT_MINOR_VERSION_ARB, WINX_GL_MINOR_VERSION,
    None,
  };

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB" );

  window->gl_context = glXCreateContextAttribsARB(window->winx->display, window->winx->best_fbc,
                                                  0, True, context_attributes);

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
    free(window->framebuffer);
    if (window->image)
      XDestroyImage(window->image);
  } else if (window->graphics_mode == WinxGraphicsModeOpenGL) {
    glXMakeCurrent(window->winx->display, None, NULL);
    glXDestroyContext(window->winx->display, window->gl_context);
    glXDestroyWindow(window->winx->display, window->window);
  }

  XFree(window->visual_info);
  XFreeGC(window->winx->display, window->graphic_context);
  XUnmapWindow(window->winx->display, window->window);
  XDestroyWindow(window->winx->display, window->window);
}

void winx_native_cleanup(WinxNative *winx) {
  XFree(winx->fbc);
  XCloseDisplay(winx->display);
}
