#include "shl_defs.h"
#include "shl_str.h"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative       *winx_native_init(void);
WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height,
                                          WinxNativeWindow *parent);
void              winx_native_init_framebuffer(WinxNativeWindow *window,
                                               u32 width, u32 height);
void              winx_native_init_opengl_context(WinxNativeWindow *window);
void              winx_native_draw(WinxNativeWindow *window, u32 width, u32 height);
u32              *winx_native_get_framebuffer(WinxNativeWindow *window);
void              winx_native_destroy_window(WinxNativeWindow *window);
void              winx_native_cleanup(WinxNative *winx);
