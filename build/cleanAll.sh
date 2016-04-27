#!/bin/bash

if [ -f Makefile ]; then
  case "$(uname -s)" in
    Linux)
      echo "linux: make clean"
      make clean
      ;;
    CYGWIN*|MINGW*|MSYS*)
      echo "windows: nmake clean"
      "${VS120COMNTOOLS}/../../VC/bin/amd64/nmake.exe" clean
      ;;
    *)
      echo 'unknown OS' 
      ;;
  esac
fi

echo "remove all remaining files..."
rm -f Project.cbp
rm -rf CMakeFiles
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f CMakeCPackOptions.cmake
rm -f CPackConfig.cmake
rm -f CPackSourceConfig.cmake
rm -f Makefile
rm -rf ext_libs
rm -rf pm/CMakeFiles
rm -f pm/cmake_install.cmake
rm -rf pm/moc_*
rm -f pm/Makefile
rm -rf pm/rdp
rm -f pm/images.qrc.depends
rm -rf pm/logs/*
rm -f pm/PrivacyMachine.cbp
rm -f PrivacyMachine.cbp
rm -f pm/PrivacyMachine.ilk
rm -f pm/core
rm -rf pm/*.dll
