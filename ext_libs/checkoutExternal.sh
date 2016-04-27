#!/bin/bash
# remark: find HEAD-id: git rev-parse HEAD

if [ -d ./FreeRDP ]; then
  echo "The directory 'FreeRDP' does already exist, please delete it manually!"
  exit 1
fi

if [ -d ./RemoteDisplay ]; then
  echo "The directory 'RemoteDisplay' does already exist, please delete it manually!"
  exit 1
fi

# Get project FreeRDP from github
git clone https://github.com/pm-bernhard/FreeRDP.git

# Get project RemoteDisplay from github
git clone https://github.com/pm-bernhard/RemoteDisplay.git
pushd RemoteDisplay
git checkout updating

