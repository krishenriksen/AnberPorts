Lightweight On-Screen-Keyboard based on SDL2
---

Tested on RG351P, RG351M and RG351V

### Building:

This project uses meson, and can be built with:

```
$ meson --prefix=/roms/ports/AnberPorts/bin/ -Dbuildtype=release build
$ meson compile -j4 -C build
```

### Install
Place `DejaVuSans.ttf` in home directory of current user.

### Usage

For text input

`osk-sdl -t "Enter Text"`

For password input

`osk-sdl -p "Enter Text"`