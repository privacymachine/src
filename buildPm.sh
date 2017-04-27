#!/bin/bash
## This scripts clean all old build information and starts a fresh build
## Arguments: release : Do a release-build with packaging

build_type=debug
if [ "$1" = "release" ]; then
  build_type=release
fi

source common.sh

# set environment variables needed for windows build tools
function run_vs14
{
  # The environment variable VS140COMNTOOLS exists when Visual Studio 2015 is installed
  eval vssetup="\$VS140COMNTOOLS\\..\\\..\\\VC\\\vcvarsall.bat"
  cmd /Q /C call "$vssetup" x64 "&&" "${@}"
}

./cleanPm.sh
if [ ! -d build_pm ]; then
  mkdir build_pm
fi

if [ "$OS" == "Windows" ]; then
  if [ ! -f working_dir/build_libs/RemoteDisplay.lib ]; then
    echo "ERROR!: External libraries like working_dir/build_libs/RemoteDisplay.lib are missing"
    echo "ERROR: Please build them with the command './buildExtLibs.sh'"
    exit 1;
  fi
else
  if [ ! -f working_dir/lib/libRemoteDisplay.so ]; then
    echo "ERROR: External libraries like working_dir/lib/libRemoteDisplay.so are missing"
    echo "ERROR: Please build them with the command './buildExtLibs.sh'"
    exit 1;
  fi
fi

pushd build_pm

startTime=`date`

if [ "$OS" == "Windows" ]; then
  echo "windows: start cmake" || exit $?
  if [ "$build_type" = "release" ]; then
    run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles JOM" $@ .. || exit $?
    #run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles" $@ .. || exit $?
  else
    run_vs14 cmake -D CMAKE_BUILD_TYPE=Debug -G"NMake Makefiles" $@ .. || exit $?
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
    cmake -D CMAKE_BUILD_TYPE=Debug .. || exit $?
  fi

  # use 8 threads for compilation
  make -j -l 8 all || exit $?
  #make all || exit $?

#  if [ "$build_type" = "release" ]; then
#    cpack -G DEB .. || exit $?
#  fi
fi
popd

echo "Pm-Build started at: $startTime" 
echo "Pm-Build finished at: `date`"
