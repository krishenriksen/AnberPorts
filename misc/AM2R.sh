#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/am2r"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib32"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# gl4es
export LIBGL_FB=4

# system
export LD_LIBRARY_PATH=$LIBDIR:/usr/lib32

# box86
export BOX86_ALLOWMISSINGLIBS=1
export BOX86_LD_LIBRARY_PATH=$LIBDIR
export BOX86_LIBGL=$LIBDIR/libGL.so.1
export BOX86_PATH=$BINDIR

cd $GAMEDIR

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  setarch linux32 -L $BINDIR/box86 ./am2r 2>&1 | tee -a ./log.txt &
  sudo $BINDIR/oga_controls
  pgrep -f oga_controls | sudo xargs kill -9
else
  patchelf --set-interpreter /usr/lib32/ld-linux-armhf.so.3 $BINDIR/box86
  $BINDIR/box86 ./am2r 2>&1 | tee -a ./log.txt &
  $BINDIR/oga_controls
  pgrep -f oga_controls | xargs kill -9
fi

pgrep -f box86 | xargs kill -9
