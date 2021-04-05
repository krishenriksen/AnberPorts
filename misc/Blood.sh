#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/Blood"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib64"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# system
export LD_LIBRARY_PATH=$GAMEDIR/lib:$LIBDIR:/usr/lib

cd $GAMEDIR

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo $BINDIR/oga_controls nblood &
else
  $BINDIR/oga_controls nblood &
fi

./nblood 2>&1 | tee -a ./log.txt

# make sure AnberPorts-Joystick is killed if quit via game menu
if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  pgrep -f oga_controls | sudo xargs kill -9
else
  pgrep -f oga_controls | xargs kill -9
fi