#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/undertale"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib32"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# gl4es
export LIBGL_FB=4

# system
export LD_LIBRARY_PATH=$LIBDIR:/usr/lib32:/usr/local/lib/arm-linux-gnueabihf/

# box86
export BOX86_ALLOWMISSINGLIBS=1
export BOX86_LD_LIBRARY_PATH=$LIBDIR
export BOX86_LIBGL=$LIBDIR/libGL.so.1
export BOX86_PATH=$BINDIR

cd $GAMEDIR

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo $BINDIR/oga_controls box86 &
else
  patchelf --set-interpreter /usr/lib32/ld-linux-armhf.so.3 $BINDIR/box86
  $BINDIR/oga_controls box86 &
fi

$BINDIR/box86 $GAMEDIR/runner 2>&1 | tee -a $GAMEDIR/log.txt

# make sure AnberPorts-Joystick is killed if quit via game menu
if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  pgrep -f oga_controls | sudo xargs kill -9
else
  pgrep -f oga_controls | xargs kill -9
fi