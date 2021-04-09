#!/bin/bash

version=1.3.5

# Copyright (c) 2021
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#
# Authored by: Kris Henriksen <krishenriksen.work@gmail.com>
#
# AnberPorts
#

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo chmod 666 /dev/tty1
fi

export TERM=linux
export XDG_RUNTIME_DIR=/run/user/$UID/

printf "\033c" > /dev/tty1

#
# Check connectivity
#
GW=`ip route | awk '/default/ { print $3 }'`
if [ -z "$GW" ]; then
  printf "\e[32mYour network connection does not seem to be working.\nDid you make sure to configure your wifi connection\nusing the Wifi selection in the Options menu?" > /dev/tty1
  sleep 10

  exit 1
fi

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  if ! dpkg -s "zip" &> /dev/null ; then
    install_dependencies="$install_dependencies zip"
  fi

  if ! dpkg -s "unzip" &> /dev/null ; then
    install_dependencies="$install_dependencies unzip"
  fi

  if ! dpkg -s "curl" &> /dev/null ; then
    install_dependencies="$install_dependencies curl"
  fi  

  if ! dpkg -s "wget" &> /dev/null ; then
    install_dependencies="$install_dependencies wget"
  fi

  if ! dpkg -s "libevdev2" &> /dev/null ; then
    install_dependencies="$install_dependencies libevdev2"
  fi

  if ! dpkg -s "rsync" &> /dev/null ; then
    install_dependencies="$install_dependencies rsync"
  fi

  if [ ! -z "$install_dependencies" ]
  then
  	printf "\e[32mnstalling dependencies ...\n" > /dev/tty1
    sudo apt update && sudo apt install -y $install_dependencies --no-install-recommends
  fi
fi

#
# Version management
#
latest=`wget -qO- https://api.github.com/repos/krishenriksen/AnberPorts/releases/latest | grep tag_name | cut -d '"' -f 4`
update="Using latest version"

if [ $latest \> $version ]; then
  update="New version available!"
fi

GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts"

cd $GAMEDIR

#
# Menu type ( sdl or dialog )
#
if [[ `cat ./type.txt` == "sdl" ]]; then
  ./AnberPortsSDL "$update" 2>&1 | tee -a ./log.txt
else
  bash ./AnberPorts "$version" "$update" 2>&1 | tee -a ./log.txt
fi
