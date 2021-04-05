#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/Half-Life"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib64"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# system
export LD_LIBRARY_PATH=$GAMEDIR/lib:$LIBDIR:/usr/lib

cd $GAMEDIR

./xash3d -fullscreen -console -sdl_joy_old_api -game cstrike 2>&1 | tee -a ./log.txt