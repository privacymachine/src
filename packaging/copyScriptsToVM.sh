#!/bin/bash
# This Scripts copies all scripts from the local directory root_of_vm to the running VM

echo "Copy all scriptfiles from local dir to the VM"

VBOXMANAGE=vboxmanage
user=pm
passwd=123
machine_name=pm_base
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#recursive copy does not work, so we copy them separately (+ add the executable flag)

$VBOXMANAGE guestcontrol "$machine_name" mkdir /pm/scripts --parents --username "$user" --password "$passwd"

file=/pm/scripts/wait_till_desktop_running.sh
$VBOXMANAGE guestcontrol "$machine_name" copyto $DIR/root_of_vm/$file $file --username "$user" --password "$passwd"
$VBOXMANAGE guestcontrol "$machine_name" execute --image /bin/chmod --username "$user" --password "$passwd" -- +x $file

file=/pm/scripts/autostart_after_x_login.sh
$VBOXMANAGE guestcontrol "$machine_name" copyto $DIR/root_of_vm/$file $file --username "$user" --password "$passwd"
$VBOXMANAGE guestcontrol "$machine_name" execute --image /bin/chmod --username "$user" --password "$passwd" -- +x $file

file=/pm/scripts/change_display_resolution.sh
$VBOXMANAGE guestcontrol "$machine_name" copyto $DIR/root_of_vm/$file $file --username "$user" --password "$passwd"
$VBOXMANAGE guestcontrol "$machine_name" execute --image /bin/chmod --username "$user" --password "$passwd" -- +x $file

file=/home/pm/.config/autostart/pm_startup.desktop
$VBOXMANAGE guestcontrol "$machine_name" copyto $DIR/root_of_vm/$file $file --username "$user" --password "$passwd"
$VBOXMANAGE guestcontrol "$machine_name" execute --image /bin/chmod --username "$user" --password "$passwd" -- +x $file

file=/etc/init/x11vnc.conf
$VBOXMANAGE guestcontrol "$machine_name" copyto $DIR/root_of_vm/$file $file --username "$user" --password "$passwd"
$VBOXMANAGE guestcontrol "$machine_name" execute --image /bin/chmod --username "$user" --password "$passwd" -- +x $file
