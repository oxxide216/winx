#!/usr/bin/sh

CFLAGS="-Wall -Wextra -Iinclude"
WINX_SRC="$(find src -name "*.c" -not -path 'src/platform/*')"
WINX_PLATFORM_SRC="$(find src/platform -name '*.c')"
LIBS="-lX11 -lGL"

cc -o winx-test $CFLAGS $LIBS "${@:1}" $WINX_SRC $WINX_PLATFORM_SRC
