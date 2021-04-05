#/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/doom1"
/usr/local/bin/retroarch -L /home/ark/.config/retroarch/cores/prboom_libretro.so $DIR/Doom.wad 2>&1 | tee -a $DIR/log.txt