.RECIPEPREFIX = >

CC = cc
override CFLAGS += -Wall -Wextra -Iinclude
override LDFLAGS += -lX11 -lGL
BUILD_DIR = build

SRC = $(wildcard src/*.c)
X11_SRC = $(wildcard src/platform/x11/*.c)
TESTS_SRC = $(wildcard tests/*.c)

OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))
X11_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(X11_SRC))

TESTS_EXECS = $(patsubst tests/%.c,$(BUILD_DIR)/tests/%,$(TESTS_SRC))

libwinx.a: $(OBJ) $(X11_OBJ)
> ar rcs libwinx.a $(OBJ) $(X11_OBJ)

tests: $(TESTS_EXECS)

$(BUILD_DIR)/%.o: src/%.c
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%: tests/%.c libwinx.a
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -o $@ $< libwinx.a $(LDFLAGS)

clean:
> rm -rf $(BUILD_DIR) libwinx.a
