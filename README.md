# Winx

A simple windowing library!

Currently supports *nix (via X11 protocol) and Windows.

## Dependencies

 - libxcb (when using x11 backend)
 - make

## Building

### Linux

```shell
make
```

#### Tests

```shell
make tests
```

#### Cross-compile to Windows

```shell
make PLATFORM=WINDOWS
```
