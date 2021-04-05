#/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/cavestory"
/usr/local/bin/retroarch -L /home/ark/.config/retroarch/cores/nxengine_libretro.so $DIR/Doukutsu.exe 2>&1 | tee -a $DIR/log.txt