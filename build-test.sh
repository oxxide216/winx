#!/usr/bin/sh

CFLAGS="-Wall -Wextra -Isrc"
WINX_SRC="$(find src -name "*.c" -not -path 'src/platform/*')"
WINX_X11_SRC="$(find src/platform/x11 -name '*.c')"
TEST_SRC="$(find test -name '*.c')"
LIBS="-lxcb -lxcb-keysyms -lxkbcommon -lxkbcommon-x11"

cc -o winx-test $CFLAGS $LIBS "${@:1}" $WINX_SRC $WINX_X11_SRC $TEST_SRC
