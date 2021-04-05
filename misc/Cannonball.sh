#/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/cannonball"
/usr/local/bin/retroarch -L /home/ark/.config/retroarch/cores/cannonball_libretro.so $DIR/epr-10187.88 2>&1 | tee -a $DIR/log.txt