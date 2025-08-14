typedef struct WinxNative WinxNative;
typedef struct WinxNativeWindow WinxNativeWindow;

WinxNative       *winx_native_init(void);
WinxNativeWindow *winx_create_native_window(WinxNative *winx, Str name, u32 width, u32 height);
void              winx_native_cleanup(WinxNative *winx);
void              winx_destroy_native_window(WinxNative *winx, WinxNativeWindow *window);
