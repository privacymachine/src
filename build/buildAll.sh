#!/bin/bash

./cleanAll.sh


# detect operating system
set OS=Linux
case "$(uname -s)" in
  Linux)
    OS=Linux
    ;;
  CYGWIN*|MINGW*|MSYS*)
    OS=Windows
    ;;
  *)
    echo 'unknown OS' 
    return 1
    ;;
esac

startTime=`date`

if [ "$OS" == "Windows" ]; then
  echo "windows: start cmake"
  cmake -G"NMake Makefiles" ..
  nmake all
else
  # Linux
  cmake ..
  make all
  # create some symlinks
  if [ ! -L pm/xfreerdp ]; then
    ln -s ../ext_libs/FreeRDP/client/X11/xfreerdp pm/
  fi
fi

if [ ! -f pm/base-disk.vdi ]; then
 echo "WARNING: The PrivacyMachine needs the file base-disk.vdi in it's working directory"
fi

echo "Build-Full started at: $startTime" 
echo "Build-Full finished at: `date`"
