@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
REM This Scripts copies all scripts from the running VM to the local directory root_of_vm

echo Copy all scriptfiles from VM to local dir

set VBOXMANAGE="%VBOX_INSTALL_PATH%\VBoxManage.exe"
set user=pm
set passwd=123
set machine_name=pm_base

rem recursive copy does not work, so we copy them separately

set file=/pm/scripts/wait_till_desktop_running.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyfrom %file% %~dp0root_of_vm\%file% --username "%user%" --password "%passwd%"

set file=/pm/scripts/autostart_after_x_login.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyfrom %file% %~dp0root_of_vm\%file% --username "%user%" --password "%passwd%"

set file=/pm/scripts/change_display_resolution.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyfrom %file% %~dp0root_of_vm\%file% --username "%user%" --password "%passwd%"

set file=/home/pm/.config/autostart/pm_startup.desktop
%VBOXMANAGE% guestcontrol "%machine_name%" copyfrom %file% %~dp0root_of_vm\%file% --username "%user%" --password "%passwd%"

set file=/etc/init/x11vnc.conf
%VBOXMANAGE% guestcontrol "%machine_name%" copyfrom %file% %~dp0root_of_vm\%file% --username "%user%" --password "%passwd%"

pause