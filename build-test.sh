#!/usr/bin/sh

CFLAGS="-Wall -Wextra -Isrc"
WINX_SRC="$(find src -name "*.c" -not -path 'src/platform/*')"
WINX_PLATFORM_SRC="$(find src/platform -name '*.c')"
TEST_SRC="$(find test -name '*.c')"
LIBS="-lX11"

cc -o winx-test $CFLAGS $LIBS "${@:1}" $WINX_SRC $WINX_PLATFORM_SRC $TEST_SRC
