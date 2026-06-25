.RECIPEPREFIX = >

PLATFORM = X11

ifeq ($(PLATFORM), X11)
  CC = cc
else ifeq ($(PLATFORM), WIN32)
  CC = x86_64-w64-mingw32-gcc
endif
override CFLAGS += -Wall -Wextra -Iinclude
ifeq ($(PLATFORM), X11)
  override LDFLAGS += -lX11 -lGL
else ifeq ($(PLATFORM), WIN32)
  override LDFLAGS += -lopengl32 -lgdi32
endif
BUILD_DIR = build

SRC = $(wildcard src/*.c)
ifeq ($(PLATFORM), X11)
  PLATFORM_SRC = $(wildcard src/platform/x11/*.c)
else ifeq ($(PLATFORM), WIN32)
  PLATFORM_SRC = $(wildcard src/platform/win32/*.c)
endif
TESTS_SRC = $(wildcard tests/*.c)

ifeq ($(PLATFORM), X11)
  OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))
  PLATFORM_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(PLATFORM_SRC))

  TESTS_EXECS = $(patsubst tests/%.c,$(BUILD_DIR)/tests/%,$(TESTS_SRC))
else ifeq ($(PLATFORM), WIN32)
  OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.obj,$(SRC))
  PLATFORM_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.obj,$(PLATFORM_SRC))

  TESTS_EXECS = $(patsubst tests/%.c,$(BUILD_DIR)/tests/%.exe,$(TESTS_SRC))
endif

ifeq ($(PLATFORM), X11)
libwinx.a: $(OBJ) $(PLATFORM_OBJ)
> ar rcs libwinx.a $(OBJ) $(PLATFORM_OBJ)
else ifeq ($(PLATFORM), WIN32)
libwinx.lib: $(OBJ) $(PLATFORM_OBJ)
> x86_64-w64-mingw32-ar rcs libwinx.lib $(OBJ) $(PLATFORM_OBJ)
endif

tests: $(TESTS_EXECS)

$(BUILD_DIR)/%.o: src/%.c
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%: tests/%.c libwinx.a
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -o $@ $< libwinx.a $(LDFLAGS)

$(BUILD_DIR)/%.obj: src/%.c
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%.exe: tests/%.c libwinx.lib
> mkdir -p $(dir $@)
> $(CC) $(CFLAGS) -o $@ $< libwinx.lib $(LDFLAGS)

clean:
> rm -rf $(BUILD_DIR) libwinx.a libwinx.lib
