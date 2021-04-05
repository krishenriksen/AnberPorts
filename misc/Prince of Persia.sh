#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/sdlpop"
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
LD_LIBRARY_PATH=. ./prince 2>&1 | tee -a ./log.txt
unset SDL_GAMECONTROLLERCONFIG