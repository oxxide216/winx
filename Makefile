.RECIPEPREFIX = >

PLATFORM = LINUX

ifeq ($(PLATFORM), LINUX)
  CC = cc
else ifeq ($(PLATFORM), WINDOWS)
  CC = x86_64-w64-mingw32-gcc
endif
override CFLAGS += -Wall -Wextra -Iinclude -Ilibs
ifeq ($(PLATFORM), LINUX)
  override LDFLAGS += -lm -lX11 -lXext -lGL
else ifeq ($(PLATFORM), WINDOWS)
  override LDFLAGS += -lm -lopengl32 -lgdi32 -lwinmm -static
endif
BUILD_DIR = build

SRC = $(wildcard src/*.c)
ifeq ($(PLATFORM), LINUX)
  PLATFORM_SRC = $(wildcard src/platform/x11/*.c)
else ifeq ($(PLATFORM), WINDOWS)
  PLATFORM_SRC = $(wildcard src/platform/win32/*.c)
endif
TESTS_SRC = $(wildcard tests/*.c)

ifeq ($(PLATFORM), LINUX)
  OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))
  PLATFORM_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(PLATFORM_SRC))

  TESTS_EXECS = $(patsubst tests/%.c,$(BUILD_DIR)/tests/%,$(TESTS_SRC))
else ifeq ($(PLATFORM), WINDOWS)
  OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.obj,$(SRC))
  PLATFORM_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.obj,$(PLATFORM_SRC))

  TESTS_EXECS = $(patsubst tests/%.c,$(BUILD_DIR)/tests/%.exe,$(TESTS_SRC))
endif

ifeq ($(PLATFORM), LINUX)
libwinx.a: $(OBJ) $(PLATFORM_OBJ)
> ar rcs libwinx.a $(OBJ) $(PLATFORM_OBJ)
else ifeq ($(PLATFORM), WINDOWS)
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
