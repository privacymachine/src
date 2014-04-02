#!/bin/bash
# This Scripts copies all scripts from the running VM to the local directory root_of_vm

echo Copy all scriptfiles from VM to local dir

VBOXMANAGE=vboxmanage
user=pm
passwd=123
machine_name=pm_base
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#recursive copy does not work, so we copy them separately

file=/pm/scripts/wait_till_desktop_running.sh
$VBOXMANAGE guestcontrol "$machine_name" copyfrom $file $DIR/root_of_vm/$file --username "$user" --password "$passwd"

file=/pm/scripts/autostart_after_x_login.sh
$VBOXMANAGE guestcontrol "$machine_name" copyfrom $file $DIR/root_of_vm/$file --username "$user" --password "$passwd"

file=/pm/scripts/change_display_resolution.sh
$VBOXMANAGE guestcontrol "$machine_name" copyfrom $file $DIR/root_of_vm/$file --username "$user" --password "$passwd"

file=/home/pm/.config/autostart/pm_startup.desktop
$VBOXMANAGE guestcontrol "$machine_name" copyfrom $file $DIR/root_of_vm/$file --username "$user" --password "$passwd"

file=/etc/init/x11vnc.conf
$VBOXMANAGE guestcontrol "$machine_name" copyfrom $file $DIR/root_of_vm/$file --username "$user" --password "$passwd"
