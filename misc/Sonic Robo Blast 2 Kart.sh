#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/srb2kart"
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./srb2kart 2>&1 | tee -a ./log.txt
unset SDL_GAMECONTROLLERCONFIG