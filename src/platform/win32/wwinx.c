#include <locale.h>

#include "winx/winx.h"
#include "wwinx.h"
#include "../../wstr.h"
#include "../../shl_defs.h"
#include "../../shl_str.h"
#include "../../shl_log.h"

#define WINDOW_CLASS_NAME L"Winx Window Class"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

typedef HGLRC (WINAPI *WglCreateContextAttribsARBProc)(HDC device_ctx,
                                                       HGLRC gl_context,
                                                       const i32 *attribs);

typedef HRESULT (APIENTRY *WglChoosePixelFormatARBProc)(HDC device_ctx,
                                                        const i32 *i_attribs,
                                                        const FLOAT *f_attribs,
                                                        UINT max_formats,
                                                        i32 *formats,
                                                        UINT *num_formats);

typedef BOOL(APIENTRY *WglSwapIntervalEXTProc)(i32 interval);

static WglCreateContextAttribsARBProc wglCreateContextAttribsARB = NULL;
static WglChoosePixelFormatARBProc wglChoosePixelFormatARB = NULL;
static WglSwapIntervalEXTProc wglSwapIntervalEXT = NULL;

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

WinxNative *winx_native_init(void) {
  WinxNative *winx = malloc(sizeof(WinxNative));
  memset(winx, 0, sizeof(WinxNative));
  winx->instance = GetModuleHandle(NULL);

  WNDCLASSW window_class = {0};
  window_class.lpfnWndProc = window_proc;
  window_class.hInstance = winx->instance;
  window_class.lpszClassName = WINDOW_CLASS_NAME;
  RegisterClassW(&window_class);

  setlocale(LC_ALL, "");

  return winx;
}

WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height,
                                          WinxGraphicsMode graphics_mode,
                                          WinxNativeWindow *parent) {
  WinxNativeWindow *window = malloc(sizeof(WinxNativeWindow));
  memset(window, 0, sizeof(WinxNativeWindow));
  window->winx = winx;
  window->graphics_mode = graphics_mode;

  u32 name_wlen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
                                      name.ptr, name.len, NULL, 0);
  LPWSTR name_wstr = malloc((name_wlen + 1) * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, name.ptr,
                      name.len, name_wstr, name_wlen);
  name_wstr[name_wlen] = 0;

  HWND parent_window = NULL;
  if (parent)
    parent_window = parent->window;

  window->window = CreateWindowExW(0, WINDOW_CLASS_NAME,
                                   name_wstr, WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   width, height,
                                   parent_window, NULL,
                                   winx->instance, NULL);

  if (window->window == NULL) {
    free(window);
    free(name_wstr);
    return NULL;
  }

  ShowWindow(window->window, SW_SHOWNORMAL);
  UpdateWindow(window->window);

  free(name_wstr);

  switch (graphics_mode) {
  case WinxGraphicsModeFramebuffer: {
    winx_native_init_framebuffer(window, width, height);
  } break;

  case WinxGraphicsModeOpenGL: {
    winx_native_init_gl_context(window);
  } break;
  }

  return window;
}

void winx_native_init_framebuffer(WinxNativeWindow *window, u32 width, u32 height) {
  if (window->framebuffer)
    DeleteObject(window->bitmap);

  BITMAPINFO bitmap_info = {0};
  bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmap_info.bmiHeader.biWidth = width;
  bitmap_info.bmiHeader.biHeight = -height; // Top-down
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB;

  window->device_ctx = GetDC(window->window);
  window->framebuffer = malloc(width * height * sizeof(u32));
  window->bitmap = CreateDIBSection(window->device_ctx, &bitmap_info, DIB_RGB_COLORS,
                                    (void **) &window->framebuffer, NULL, 0);
  window->bitmap_device_ctx = CreateCompatibleDC(window->device_ctx);

  SelectObject(window->bitmap_device_ctx, window->bitmap);
}

u32 *winx_native_get_framebuffer(WinxNativeWindow *window) {
  return window->framebuffer;
}

void winx_native_init_gl_context(WinxNativeWindow *window) {
  window->device_ctx = GetDC(window->window);

  PIXELFORMATDESCRIPTOR desc = {0};
  desc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  desc.nVersion = 1;
  desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                 PFD_SUPPORT_COMPOSITION | PFD_DOUBLEBUFFER;
  desc.iPixelType = PFD_TYPE_RGBA;
  desc.cColorBits = 32;
  desc.cAlphaBits = 8;
  desc.cDepthBits = 32;
  desc.iLayerType = PFD_MAIN_PLANE;

  i32 pixel_format = ChoosePixelFormat(window->device_ctx, &desc);
  SetPixelFormat(window->device_ctx, pixel_format, &desc);

  HGLRC temp_gl_context = wglCreateContext(window->device_ctx);
  wglMakeCurrent(window->device_ctx, temp_gl_context);

  wglCreateContextAttribsARB =
    (WglCreateContextAttribsARBProc) (void *)
      wglGetProcAddress("wglCreateContextAttribsARB");
  wglChoosePixelFormatARB =
    (WglChoosePixelFormatARBProc) (void *)
      wglGetProcAddress("wglChoosePixelFormatARB");
  wglSwapIntervalEXT =
    (WglSwapIntervalEXTProc) (void *)
      wglGetProcAddress("wglSwapIntervalEXT");

  wglMakeCurrent(window->device_ctx, 0);
  wglDeleteContext(temp_gl_context);

  static i32 attribs[] = {
    0x2003, // WGL_ACCELERATION_ARB
    0x2027, // WGL_FULL_ACCELERATION_ARB
    0x201b, 8, // WGL_ALPHA_BITS_ARB
    0x2022, 32, // WGL_DEPTH_BITS_ARB
    0x2001, 1, // WGL_DRAW_TO_WINDOW_ARB
    0x2015, 8, // WGL_RED_BITS_ARB
    0x2017, 8, // WGL_GREEN_BITS_ARB
    0x2019, 8, // WGL_BLUE_BITS_ARB
    0x2013, 0x202B, // WGL_PIXEL_TYPE_ARB,  WGL_TYPE_RGBA_ARB
    0x2010, 1, // WGL_SUPPORT_OPENGL_ARB
    0x2014, 32, // WGL_COLOR_BITS_ARB
    0x2011, 1, // Double buffer
    0,
  };

  UINT num_formats;
  wglChoosePixelFormatARB(window->device_ctx, attribs, 0, 1, &pixel_format, &num_formats);
  DescribePixelFormat(window->device_ctx, pixel_format, sizeof(desc), &desc);
  SetPixelFormat(window->device_ctx, pixel_format, &desc);

  i32 context_attribs[] = {
    0x9126, 1, // WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB
    0x2091, WINX_GL_MAJOR_VERSION, // WGL_CONTEXT_MAJOR_VERSION_ARB
    0x2092, WINX_GL_MINOR_VERSION, // WGL_CONTEXT_MINOR_VERSION_ARB
    0,
  };

  window->gl_context = wglCreateContextAttribsARB(window->device_ctx, NULL, context_attribs);
  wglMakeCurrent(window->device_ctx, window->gl_context);
}

void winx_native_make_context_current(WinxNativeWindow *window) {
  wglMakeCurrent(window->device_ctx, window->gl_context);
}

void winx_native_draw(WinxNativeWindow *window, u32 width, u32 height) {
  if (window->graphics_mode == WinxGraphicsModeFramebuffer)
    BitBlt(window->device_ctx, 0, 0, width, height,
           window->bitmap_device_ctx, 0, 0, SRCCOPY);
  else if (window->graphics_mode == WinxGraphicsModeOpenGL)
    wglSwapLayerBuffers(window->device_ctx, WGL_SWAP_MAIN_PLANE);
}

void winx_native_destroy_window(WinxNativeWindow *window) {
  if (window->graphics_mode == WinxGraphicsModeFramebuffer) {
    DeleteObject(window->bitmap);
    DeleteDC(window->bitmap_device_ctx);
  } else if (window->graphics_mode == WinxGraphicsModeOpenGL) {
    wglMakeCurrent(window->device_ctx, NULL);
    wglDeleteContext(window->gl_context);
  }

  ReleaseDC(window->window, window->device_ctx);
  DestroyWindow(window->window);
}

void winx_native_cleanup(WinxNative *winx) {
  UnregisterClassW(WINDOW_CLASS_NAME, winx->instance);
}

WinxApiProc winx_native_load_proc_address(const char *name) {
  return (WinxApiProc) wglGetProcAddress(name);
}
