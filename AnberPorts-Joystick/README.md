# AnberPorts-Joystick
Emulated keyboard / mouse / joystick for Anbernic rg351p / rg351m / rg351v

# Howto
Launch with `sudo ./oga_controls` or add current user to uinput via udev rule to avoid using sudo.

If you append application as argument to launch command, you can quit by using Start + Select like so:

`sudo ./oga_controls Application &`

This will `pgrep -f Application  | xargs kill -9`

# /etc/udev/rules.d
```
SUBSYSTEM=="misc", KERNEL=="uinput", MODE="0660", GROUP="uinput"
```

## Support the project

[<img src="https://github.com/krishenriksen/AnberPorts/raw/master/patreon.png"/>](https://www.patreon.com/bePatron?u=54003740) [<img src="https://github.com/krishenriksen/AnberPorts/raw/master/sponsor.png"/>](https://github.com/sponsors/krishenriksen)
