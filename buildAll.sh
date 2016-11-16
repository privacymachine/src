#!/bin/bash
## This scripts clean all old build information and starts a fresh build
## Arguments: release : Do a release-build with packaging

build_type=debug
if [ "$1" = "release" ]; then
  build_type=release
fi

# set environment variables needed for windows build tools
function run_vs14
{
  # The environment variable VS140COMNTOOLS exists when Visual Studio 2015 is installed
  eval vssetup="\$VS140COMNTOOLS\\..\\\..\\\VC\\\vcvarsall.bat"
  #eval vssetup="\$VS140COMNTOOLS\\VsDevCmd.bat"

  cmd /Q /C call "$vssetup" x64 "&&" "${@}"
}

./cleanAll.sh
if [ ! -d build ]; then
  mkdir build
fi

if [ ! -d ext_libs/FreeRDP ]; then
  pushd ext_libs
  ./checkoutExternal.sh
  popd
fi

pushd build

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
  echo "windows: start cmake" || exit $?
  if [ "$build_type" = "release" ]; then
    run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles JOM" $@ .. || exit $?
    #run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles" $@ .. || exit $?
  else
    run_vs14 cmake -G"NMake Makefiles" $@ .. || exit $?
  fi  

  run_vs14 jom -j 8 all || exit $?
  #run_vs14 nmake all || exit $?

  if [ "$build_type" = "release" ]; then
    pushd ../install_windows/manual_packaging
    ./buildMsiSetup.sh || exit $?
    popd
  fi

else

  # Linux
  if [ "$build_type" = "release" ]; then
    cmake -D CMAKE_BUILD_TYPE=Release .. || exit $?
  else
    cmake .. || exit $?
  fi

  # use 8 threads for compilation
  make -j -l 8 all || exit $?
  #make all || exit $?

#  if [ "$build_type" = "release" ]; then
#    cpack -G DEB .. || exit $?
#  fi
fi
popd

echo "Build-Full started at: $startTime" 
echo "Build-Full finished at: `date`"
