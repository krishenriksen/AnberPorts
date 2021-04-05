#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/Flashback"
LIBDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/lib64"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

# system
export LD_LIBRARY_PATH=$GAMEDIR/lib:$LIBDIR:/usr/lib

cd $GAMEDIR
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./rs 2>&1 | tee -a ./log.txt
unset SDL_GAMECONTROLLERCONFIG