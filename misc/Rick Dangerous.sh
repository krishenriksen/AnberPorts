#/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/xrick"
/usr/local/bin/retroarch -L /home/ark/.config/retroarch/cores/xrick_libretro.so $DIR/data.zip 2>&1 | tee -a $DIR/log.txt