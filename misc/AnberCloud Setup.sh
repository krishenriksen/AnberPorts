#!/bin/bash

version=0.0.1

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
# AnberCloud
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DIR=~/AnberCloud
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"
GITSRC=https://github.com/krishenriksen/AnberCloud.git
DEVICE=`cat /sys/class/net/$(ip route show default | awk '/default/ {print $5}')/address | sed 's/:/-/g'`
SYNC=`cat $SCRIPTDIR/sync-id`
KEY=`cat $SCRIPTDIR/key`
LOG=/tmp/AnberCloud.txt

export TERM=linux

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo chmod 666 /dev/tty1
fi

#
# Check connectivity
#
GW=`ip route | awk '/default/ { print $3 }'`
if [ -z "$GW" ]; then
  printf "\e[32mYour network connection does not seem to be working.\nDid you make sure to configure your wifi connection\nusing the Wifi selection in the Options menu?" > /dev/tty1
  sleep 10

  exit 1
fi

printf "\033c" > /dev/tty1

# clear log
rm -rf $LOG

ExitMenu() {
  if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
    pgrep -f oga_controls | sudo xargs kill -9
    pgrep -f AnberCloud | sudo xargs kill -9
  else
    pgrep -f oga_controls | xargs kill -9
    pgrep -f AnberCloud | xargs kill -9
  fi
}

gitit() {
  git -c core.sshCommand="ssh -i $DIR/.github/save" "$@"
}

Syncing() {
  dialog --backtitle "AnberCloud - DEVICE ID: $DEVICE" --infobox "\nNow syncing with device:\n\n$1" 7 45 > /dev/tty1

  gitit pull git@github.com:krishenriksen/AnberCloud $1 2>&1 | tee -a $LOG

  # populate saves with protected from zip file
  unzip -P $KEY -qq -o ./saves/saves.zip -d .
  unzip -P $KEY -qq -o ./states/states.zip -d .
  unzip -P $KEY -qq -o ./savestates/savestates.zip -d .
  unzip -P $KEY -qq -o ./roms/roms.zip -d .

  # load
  rsync -r -u ./saves/* ~/.config/retroarch/saves/ 2>&1 | tee -a $LOG
  rsync -r -u ./states/* ~/.config/retroarch/states/ 2>&1 | tee -a $LOG
  if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
    rsync -r -u ./savestates/* /opt/amiberry/savestates/ 2>&1 | tee -a $LOG
    sudo rsync -a -u --include '*/' --include '*.state' --exclude '*' ./roms/ /roms/
  else
  	rsync -a -u --include '*/' --include '*.state' --exclude '*' ./roms/ /roms/
  fi

  # save
  rsync -r -u ~/.config/retroarch/saves/* ./saves/ 2>&1 | tee -a $LOG
  rsync -r -u ~/.config/retroarch/states/* ./states/ 2>&1 | tee -a $LOG
  if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
    rsync -r -u /opt/amiberry/savestates/* ./savestates/ 2>&1 | tee -a $LOG
    sudo rsync -a -u --include '*/' --include '*.state' --exclude '*' /roms/ ./roms/
  else
    rsync -a -u --include '*/' --include '*.state' --exclude '*' /roms/ ./roms/
  fi

  # password protect saves with key
  zip -r ./saves/saves.zip ./saves/ -P $KEY
  zip -r ./states/states.zip ./states/ -P $KEY
  zip -r ./savestates/savestates.zip ./savestates/ -P $KEY
  zip -r ./roms/roms.zip ./roms/ -P $KEY

  gitit add saves savestates states roms 2>&1 | tee -a $LOG

  # stamp it
  gitit commit -m `date +%s` 2>&1 | tee -a $LOG

  gitit push --set-upstream origin $1 2>&1 | tee -a $LOG
}

SelectSync() {
  dialog --backtitle "AnberCloud - DEVICE ID: $1" --infobox "\nNow syncing with device:\n\n$1" 7 45 > /dev/tty1

  gitit checkout -b $1 2>&1 | tee -a $LOG

  echo $1 > $SCRIPTDIR/sync-id

  Syncing $1
}

Generate() {
  # unique key
  echo `uuidgen` > $SCRIPTDIR/key
  KEY=`cat $SCRIPTDIR/key`
}

Setup() {
  dialog --backtitle "AnberCloud - DEVICE ID: $DEVICE" --infobox "\nPlease wait ..." 5 25 > /dev/tty1

  mkdir ~/.ssh
  ssh-keyscan -H github.com >> ~/.ssh/known_hosts 2>&1 | tee -a $LOG

  # setup git
  gitit config --global user.email "device@anbernic" 2>&1 | tee -a $LOG
  gitit config --global user.name $DEVICE 2>&1 | tee -a $LOG

  gitit clone $GITSRC $DIR 2>&1 | tee -a $LOG

  cd $DIR

  # fix it
  git remote set-url origin git@github.com:krishenriksen/AnberCloud.git

  # unique key
  Generate

  unzip .github/cloud.zip -d .github/
  chmod 600 .github/save

  gitit checkout -b $DEVICE 2>&1 | tee -a $LOG

  # save states
  SelectSync $DEVICE
}

if [[ $1 == "sync" ]]; then
  if [ -d "$DIR/.git" ]; then
    cd $DIR
    Syncing $SYNC
  fi
else
  if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
    sudo $BINDIR/oga_controls AnberCloud &
  else
    $BINDIR/oga_controls AnberCloud &
  fi

  if [[ ! -d "$DIR/.git" ]]; then
    Setup
  fi

  if [[ -d "$DIR/.git" ]]; then
    cd $DIR

    # update list
    dialog --backtitle "AnberCloud - DEVICE ID: $DEVICE" --infobox "\nPlease wait, refreshing list ..." 5 40 > /dev/tty1
    gitit pull

    options=(
      Sync "Synchronize"
      Key "Generate new"
    )

    for BRANCH in `gitit branch --remotes --format='%(refname:short)' | cut -c 8-`; do
  	  if [[ $BRANCH != "HEAD" ]] && [[ $BRANCH != "master" ]]; then
        options+=($BRANCH "Device")
      fi
    done

    while true; do
      selection=(dialog \
   	  --backtitle "AnberCloud - DEVICE ID: $DEVICE" \
   	  --title "[ Syncing with device: $SYNC ]" \
   	  --no-collapse \
   	  --clear \
	  --cancel-label "Select + Start to Exit" \
      --menu "Key: $KEY" 15 55 15)

      choices=$("${selection[@]}" "${options[@]}" 2>&1 > /dev/tty1)

      for choice in $choices; do
        case $choice in
          Sync) Syncing $SYNC ;;
          Key) Generate ;;
          *) SelectSync $choice ;;
        esac
      done
    done  
  fi
fi