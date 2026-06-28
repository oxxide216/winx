#include "winx/winx.h"
#include "../graphics-mode.h"
#include "shl/shl-defs.h"
#include "shl/shl-str.h"

typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative       *winx_native_init(void);
WinxNativeWindow *winx_native_init_window(WinxNative *winx, Str name,
                                          u32 width, u32 height,
                                          WinxGraphicsMode graphics_mode,
                                          WinxNativeWindow *parent);
void              winx_native_init_framebuffer(WinxNativeWindow *window,
                                               u32 width, u32 height);
u32              *winx_native_get_framebuffer(WinxNativeWindow *window);
void              winx_native_init_gl_context(WinxNativeWindow *window);
void              winx_native_make_context_current(WinxNativeWindow *window);
f32               winx_native_get_refresh_rate(WinxNativeWindow *window);
f32               winx_native_get_time(WinxNativeWindow *window);
void              winx_native_draw(WinxNativeWindow *window, u32 width, u32 height);
void              winx_native_destroy_window(WinxNativeWindow *window);
void              winx_native_cleanup(WinxNative *winx);

void winx_native_sleep(u32 ms);

WinxApiProc winx_native_load_proc_address(const char *name);
