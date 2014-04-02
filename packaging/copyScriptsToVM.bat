@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
REM This Scripts copies all scripts from the local directory root_of_vm to the running VM

echo Copy all scriptfiles from local dir to the VM

set VBOXMANAGE="%VBOX_INSTALL_PATH%\VBoxManage.exe"
set user=pm
set passwd=123
set machine_name=pm_base

rem recursive copy does not work, so we copy them separately (+ add the executable flag)

%VBOXMANAGE% guestcontrol "%machine_name%" mkdir /pm/scripts --parents --username "%user%" --password "%passwd%"

set file=/pm/scripts/wait_till_desktop_running.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyto %~dp0root_of_vm\%file% %file% --username "%user%" --password "%passwd%"
%VBOXMANAGE% guestcontrol "%machine_name%" execute --image /bin/chmod --username "%user%" --password "%passwd%" -- +x %file%

set file=/pm/scripts/autostart_after_x_login.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyto %~dp0root_of_vm\%file% %file% --username "%user%" --password "%passwd%"
%VBOXMANAGE% guestcontrol "%machine_name%" execute --image /bin/chmod --username "%user%" --password "%passwd%" -- +x %file%

set file=/pm/scripts/change_display_resolution.sh
%VBOXMANAGE% guestcontrol "%machine_name%" copyto %~dp0root_of_vm\%file% %file% --username "%user%" --password "%passwd%"
%VBOXMANAGE% guestcontrol "%machine_name%" execute --image /bin/chmod --username "%user%" --password "%passwd%" -- +x %file%

set file=/home/pm/.config/autostart/pm_startup.desktop
%VBOXMANAGE% guestcontrol "%machine_name%" copyto %~dp0root_of_vm\%file% %file% --username "%user%" --password "%passwd%"
%VBOXMANAGE% guestcontrol "%machine_name%" execute --image /bin/chmod --username "%user%" --password "%passwd%" -- +x %file%

set file=/etc/init/x11vnc.conf
%VBOXMANAGE% guestcontrol "%machine_name%" copyto %~dp0root_of_vm\%file% %file% --username "root" --password "%passwd%"
%VBOXMANAGE% guestcontrol "%machine_name%" execute --image /bin/chmod --username "%user%" --password "%passwd%" -- +x %file%

pause