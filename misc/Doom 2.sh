#/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/doom2"
/usr/local/bin/retroarch -L /home/ark/.config/retroarch/cores/prboom_libretro.so $DIR/Doom2.wad 2>&1 | tee -a $DIR/log.txt