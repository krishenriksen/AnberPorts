#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/TheyNeedToBeFed"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib32"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# gl4es
export LIBGL_FB=4

# system
export LD_LIBRARY_PATH=$LIBDIR:/usr/lib32:/usr/local/lib/arm-linux-gnueabihf/
export LD_PRELOAD=$LIBDIR/libbcm_host.so

cd $GAMEDIR

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo $BINDIR/rg351p-js2xbox &
  sudo $BINDIR/oga_controls TheyNeedToBeFed &
else
  patchelf --set-interpreter /usr/lib32/ld-linux-armhf.so.3 ./TheyNeedToBeFed
  patchelf --set-interpreter /usr/lib32/ld-linux-armhf.so.3 $BINDIR/rg351p-js2xbox
  $BINDIR/rg351p-js2xbox &
  $BINDIR/oga_controls TheyNeedToBeFed &
fi

./TheyNeedToBeFed 2>&1 | tee -a ./log.txt

# make sure AnberPorts-Joystick is killed if quit via game menu
if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  pgrep -f rg351p-js2xbox | sudo xargs kill -9
  pgrep -f oga_controls | sudo xargs kill -9
else
  pgrep -f rg351p-js2xbox | xargs kill -9
  pgrep -f oga_controls | xargs kill -9
fi