#/bin/bash
/usr/local/bin/retroarch32 -L /home/ark/.config/retroarch32/cores/tyrquake_libretro.so /roms/ports/quake1/ 2>&1 | tee -a /roms/ports/quake1/log.txt