# AnberPorts-Joystick
Emulated keyboard / mouse / joystick for Anbernic rg351p / rg351m / rg351v

# Howto
Launch with `sudo ./oga_controls` or add current user to uinput via udev rule.

# /etc/udev/rules.d
```
SUBSYSTEM=="misc", KERNEL=="uinput", MODE="0660", GROUP="uinput"
```

## Support the project

[<img src="https://raw.githubusercontent.com/krishenriksen/AnberPorts/master/sponsor.png" width="200"/>](https://github.com/sponsors/krishenriksen)

[Become a sponsor to Kris Henriksen](https://github.com/sponsors/krishenriksen)