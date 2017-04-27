#!/bin/bash

if [ -d ./FreeRDP ]; then
  echo "The directory 'FreeRDP' does already exist, please delete it manually!"
  exit 1
fi

if [ -d ./RemoteDisplay ]; then
  echo "The directory 'RemoteDisplay' does already exist, please delete it manually!"
  exit 1
fi

if [ -d ./libsodium ]; then
  echo "The directory 'libsodium' does already exist, please delete it manually!"
  exit 1
fi

# Get project FreeRDP from github
git clone https://github.com/pm-bernhard/FreeRDP.git

# Get project RemoteDisplay from github
git clone -b updating https://github.com/pm-bernhard/RemoteDisplay.git

# LibSodium
#   under debian based distros the package libsodium-dev is used
#   under Windows it's needed to be installed separately

