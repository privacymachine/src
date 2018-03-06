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

./cleanAll.sh
if [ ! -d build_ext_libs ]; then
  mkdir build_ext_libs
fi

# TODO: Commented out for "schiachabalaft"
#if [ ! -d ext_libs/FreeRDP ]; then
#  pushd ext_libs
#  ./checkoutExternal.sh
#  popd
#fi

pushd build_ext_libs

startTime=`date`

if [ "$OS" == "Windows" ]; then
  echo "windows: start cmake" || exit $?
  if [ "$build_type" = "release" ]; then
    run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles JOM" $@ ../ext_libs/ || exit $?
    #run_vs14 cmake -D CMAKE_BUILD_TYPE=Release -G"NMake Makefiles" $@ ../ext_libs/ || exit $?
  else
    run_vs14 cmake -D CMAKE_BUILD_TYPE=Debug -G"NMake Makefiles JOM" $@ ../ext_libs/ || exit $?
    #run_vs14 cmake -D CMAKE_BUILD_TYPE=Debug -G"NMake Makefiles" $@ ../ext_libs/ || exit $?
  fi  

  run_vs14 jom -j 8 all || exit $?
  #run_vs14 nmake all || exit $?

else

  # Linux
  if [ "$build_type" = "release" ]; then
    cmake -D CMAKE_BUILD_TYPE=Release ../ext_libs/ || exit $?
  else
    cmake -D CMAKE_BUILD_TYPE=Debug ../ext_libs/ || exit $?
  fi

  # use 8 threads for compilation
  make -j -l 8 all || exit $?
  #make all || exit $?

#  if [ "$build_type" = "release" ]; then
#    cpack -G DEB .. || exit $?
#  fi
fi
popd

echo "ExtLibs-Build started at: $startTime" 
echo "ExtLibs-Build finished at: `date`"
