Lightweight On-Screen-Keyboard based on SDL2 for RG351P, RG351M and RG351V
---

This keyboard is used to take input from the user.

### Building:

This project uses meson, and can be built with:

```
$ meson --prefix=/roms/ports/AnberPorts/bin/ -Dbuildtype=release build
$ meson compile -j4 -C build
```