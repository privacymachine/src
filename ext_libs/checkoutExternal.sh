#!/bin/bash

if [ -d ./FreeRDP ]; then
  echo "The directory 'FreeRDP' does already exist, please delete it manually!"
  exit 1
fi

if [ -d ./RemoteDisplay ]; then
  echo "The directory 'RemoteDisplay' does already exist, please delete it manually!"
  exit 1
fi

if [ -d ./CLI11 ]; then
  echo "The directory 'RemoteDisplay' does already exist, please delete it manually!"
  exit 1
fi

# Get project FreeRDP from github
git clone -b PrivacyMachine_fRDP_2.0.0 https://github.com/pm-bernhard/FreeRDP.git

# Get project RemoteDisplay from github
git clone -b PrivacyMachine_fRDP_2.0.0 https://github.com/pm-bernhard/RemoteDisplay.git

# Get project CLI11 from github
git clone https://github.com/CLIUtils/CLI11.git
cd CLI11
git checkout v1.3.0
cd ..

# LibSodium
#   under debian based distros the package libsodium-dev is used
#   under Windows it's needed to be installed separately

