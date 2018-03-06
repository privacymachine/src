#!/bin/bash

echo "clean content of the build_ext_libs dir"
if [ -d build_ext_libs ]; then
  rm -rf build_ext_libs/*
fi

echo "remove files from working_dir..."
rm -f working_dir/lib/*
rm -f working_dir/build_libs/*
rm -f working_dir/example.exe
rm -f working_dir/example.exe.manifest
rm -f working_dir/freerdp.dll
rm -f working_dir/freerdp.dll.manifest
rm -f working_dir/freerdp-client.dll
rm -f working_dir/freerdp-client.dll.manifest
rm -f working_dir/isprefix_test.exe
rm -f working_dir/isprefix_test.exe.manifest
rm -f working_dir/isprefix_test
rm -f working_dir/minigzip.exe
rm -f working_dir/minigzip.exe.manifest
rm -f working_dir/quazip5.dll
rm -f working_dir/quazip5.dll.manifest
rm -f working_dir/quazip5d.dll
rm -f working_dir/quazip5d.dll.manifest
rm -f working_dir/rdtk.dll
rm -f working_dir/rdtk.dll.manifest
rm -f working_dir/RemoteDisplay.dll
rm -f working_dir/RemoteDisplay.dll.manifest
rm -f working_dir/RemoteDisplay-client.exe
rm -f working_dir/RemoteDisplay-client.exe.manifest
rm -f working_dir/RemoteDisplay-client
rm -f working_dir/wfreerdp.exe
rm -f working_dir/wfreerdp.exe.manifest
rm -f working_dir/wfreerdp-client.dll
rm -f working_dir/wfreerdp-client.dll.manifest
rm -f working_dir/winpr.dll
rm -f working_dir/winpr.dll.manifest
rm -f working_dir/zlib1.dll
rm -f working_dir/zlib1.dll.manifest
rm -f working_dir/zlib1d.dll
rm -f working_dir/zlib1d.dll.manifest

