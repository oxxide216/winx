#ifndef X11_KEY_CODE_H
#define X11_KEY_CODE_H

#include <X11/Xlib.h>

#include "../../key_code.h"

WinxKeyCode keysym_to_key_code(KeySym keysym);

#endif // X11_KEY_CODE_H
