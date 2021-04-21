#!/bin/bash
GAMEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/moonlight-embedded"
BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/AnberPorts/bin"

export TERM=linux
export XDG_RUNTIME_DIR=/run/user/$UID/

if id "ark" &>/dev/null || id "odroid" &>/dev/null; then
  sudo chmod 666 /dev/tty1
  # fix permissions
  sudo chown root:video /dev/vpu_service
  sudo chmod 660 /dev/vpu_service
  sudo chown root:video /dev/hevc_service
  sudo chmod 660 /dev/hevc_service
fi

mapfile settings < $GAMEDIR/settings.txt

SaveSettings() {
  for j in "${settings[@]}"
  do
    echo $j 
  done > $GAMEDIR/settings.txt
}

Steam() {
	IP="$(cat $GAMEDIR/ip.txt)"

	if [ -z "$IP" ]; then
		Pair
	else
		ip=$(echo ${settings[0]}|tr -d '\n')
		fps=$(echo ${settings[1]}|tr -d '\n')
		res=$(echo ${settings[2]}|tr -d '\n')
		app=$(echo ${settings[3]}|tr -d '\n')
		platform=$(echo ${settings[4]}|tr -d '\n')

		pgrep -f oga_controls | sudo xargs kill -9

		printf "\033c" > /dev/tty1

		export LD_LIBRARY_PATH=$GAMEDIR/lib:/usr/lib
		$GAMEDIR/moonlight stream -fps $fps $res --unsupported --nosops -mapping $GAMEDIR/gamecontrollerdb.txt -app $app -platform $platform $ip > /dev/tty1

		exit
	fi
}

Platform() {
	cmd=(dialog --clear --backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" --title "[ Platform ]" --menu "${settings[4]}" "15" "55" "15")

	options=(
		Back "Back to main menu"
		"1)" "Rockchip"
		"2)" "SDL"
	)

    choices=$("${cmd[@]}" "${options[@]}" 2>&1 > /dev/tty1)

    case $choices in
    	"1)")
			settings[4]="rk"
    	;;

    	"2)")
			settings[4]="sdl"
    	;;
    esac

    SaveSettings
}

FPS() {
	cmd=(dialog --clear --backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" --title "[ FPS ]" --menu "${settings[1]}" "15" "55" "15")

	options=(
		Back "Back to main menu"
		"1)" "24"
		"2)" "30"
		"3)" "60"
	)

    choices=$("${cmd[@]}" "${options[@]}" 2>&1 > /dev/tty1)

    case $choices in
    	"1)")
			settings[1]="24"
    	;;

    	"2)")
			settings[1]="30"
    	;;

    	"3)")
			settings[1]="60"
    	;;
    esac

    SaveSettings
}

Resolution() {
	cmd=(dialog --clear --backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" --title "[ Resolution ]" --menu "${settings[2]}" "15" "55" "15")

	options=(
		Back "Back to main menu"
		"1)" "480p"
		"2)" "720p"
		"3)" "1080p"
	)

    choices=$("${cmd[@]}" "${options[@]}" 2>&1 > /dev/tty1)

    case $choices in
    	"1)")
			settings[2]="-width 480 -height 320"
    	;;

    	"2)")
			settings[2]="-720"
    	;;

    	"3)")
			settings[2]="-1080"
    	;;
    esac

    SaveSettings
}

App() {
	cmd=(dialog --clear --backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" --title "[ App ]" --menu "Add the app in GeForce Experience under SHIELD:" "15" "55" "15")

	options=(
		Back "Back to main menu"
		"Steam" "Steam"
		"Desktop" "Desktop"
		"PPSSPP" "PPSSPP"
		"Dolphin" "Dolphin"
		"pcsx2" "pcsx2"
		"rpcs3" "rpcs3"
		"redream" "redream"
	)

    choices=$("${cmd[@]}" "${options[@]}" 2>&1 > /dev/tty1)
    settings[3]="${choices[0]}"

    SaveSettings
}

Pair() {
  IP=`$BINDIR/osk-sdl -t "Enter IP address" | tail -n 1`
  settings[0]="$IP"
  SaveSettings

  echo "$IP" > $GAMEDIR/ip.txt

  printf "\033c" > /dev/tty1

  export LD_LIBRARY_PATH=$GAMEDIR/lib:/usr/lib
  $GAMEDIR/moonlight pair "$IP" > /dev/tty1
}

Settings() {
	cmd=(dialog --clear --backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" --title "[ Settings ]" --menu "Select option from the list:" "15" "55" "15")

	options=(
		Back "Back to main menu"
		"1)" "Pair PC"
		"2)" "App - ${settings[3]}"
		"3)" "Resolution - ${settings[2]}"
		"4)" "Frames per second - ${settings[1]}"
		"5)" "Platform - ${settings[4]}"
	)

    choices=$("${cmd[@]}" "${options[@]}" 2>&1 > /dev/tty1)

    case $choices in
    	"1)")
			Pair
    	;;

    	"2)")
			App
    	;;

    	"3)")
			Resolution
    	;;

    	"4)")
			FPS
    	;;

    	"5)")
			Platform
    	;;
    esac
}

#
# Joystick controls
#
# only one instance
if ! pgrep -x "oga_controls" > /dev/null; then
  sudo $BINDIR/oga_controls Moonlight Embedded.sh &
fi

#
# Main menu
#
while true; do
	pair=`cat $GAMEDIR/log.txt | tail -n 1`

    selection=(dialog \
   	--backtitle "Moonlight Game Streaming: Play Your PC Games Remotely" \
   	--title "Paired with ${settings[0]}" \
   	--no-collapse \
   	--clear \
	--cancel-label "Select + Start to Exit" \
    --menu "$pair" 9 55 9)

	options=(
		"1)" "Start Streaming"
		"2)" "Settings"
	)

	choices=$("${selection[@]}" "${options[@]}" 2>&1 > /dev/tty1)

	for choice in $choices; do
		case $choice in
			"1)") Steam ;;
			"2)") Settings ;;
		esac
	done
done