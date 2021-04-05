#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/quake2"

export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./quake2
unset SDL_GAMECONTROLLERCONFIG