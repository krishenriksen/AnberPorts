#!/bin/bash

# strange fix
if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo fallocate -l 128M /swapfile
  sudo chmod 600 /swapfile
  sudo mkswap /swapfile
  sudo swapon /swapfile
else
  dd if=/dev/zero of=/storage/swapfile bs=1024 count=256k
  chmod 600 /storage/swapfile
  mkswap /storage/swapfile
  swapon /storage/swapfile
fi

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/eduke32"
export SDL_GAMECONTROLLERCONFIG="$(cat gamecontrollerdb.txt)"
./eduke32 2>&1 | tee -a ./log.txt
unset SDL_GAMECONTROLLERCONFIG