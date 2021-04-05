#/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/diablo1"
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./devilutionx 2>&1 | tee -a ./log.txt
unset SDL_GAMECONTROLLERCONFIG