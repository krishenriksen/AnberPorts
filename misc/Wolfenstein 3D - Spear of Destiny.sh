#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/ecwolf"
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./ecwolf --config ./ecwolf.cfg --data SOD
unset SDL_GAMECONTROLLERCONFIG
