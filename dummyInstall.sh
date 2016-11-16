#!/bin/bash
# this scripts links the files to places where a debian package would install them


if [ -f /usr/share/pixmaps/privacymachine.png ]; then
  sudo rm /usr/share/pixmaps/privacymachine.png
fi
sudo cp ${PWD}/artwork/PrivacyMachine_Logo_Icon32.png /usr/share/pixmaps/privacymachine.png


cat pm.desk_top > privacymachine.desktop
echo "Path=${PWD}/working_dir" >> privacymachine.desktop
echo "Exec=${PWD}/working_dir/PrivacyMachine" >> privacymachine.desktop
chmod +x privacymachine.desktop

if [ -f /usr/share/applications/privacymachine.desktop ]; then
  sudo rm /usr/share/applications/privacymachine.desktop
fi
sudo mv ${PWD}/privacymachine.desktop /usr/share/applications/privacymachine.desktop
