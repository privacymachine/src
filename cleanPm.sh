#!/bin/bash

echo "clean content of the build_pm dir"
if [ -d build_pm ]; then
  rm -rf build_pm/*
fi

echo "remove files from working_dir..."
rm -f working_dir/pmTests
rm -f working_dir/logs/*
rm -f working_dir/build_libs/*
rm -f working_dir/winpr-*
rm -f working_dir/UserConfigTest
rm -f working_dir/example.exe
rm -f working_dir/GuiTest
rm -f working_dir/GuiTest.exe
rm -f working_dir/isprefix_test
rm -f working_dir/isprefix_test.exe
rm -f working_dir/minigzip.exe
rm -f working_dir/PrivacyMachine
rm -f working_dir/PrivacyMachine.exe
rm -f working_dir/ProblemReporter
rm -f working_dir/ProblemReporter.exe
rm -f working_dir/RemoteDisplay-client
rm -f working_dir/RemoteDisplay-client.exe
rm -f working_dir/TestPatching
rm -f working_dir/TestPatching.exe
rm -f working_dir/TestGui
rm -f working_dir/TestGui.exe
rm -f working_dir/TestUserConfigOpenVPN
rm -f working_dir/TestUserConfigOpenVPN.exe
rm -f working_dir/TestXmlUpdateParser
rm -f working_dir/TestXmlUpdateParser.exe
rm -f working_dir/testresults.xml
rm -f working_dir/UserConfigTest.exe
rm -f working_dir/winpr*
rm -f working_dir/wfreerdp.exe
rm -f working_dir/wfreerdp-client.*
rm -f working_dir/quazip*
rm -f working_dir/zlib*
rm -f working_dir/*.dll
rm -f working_dir/*.exp
rm -f working_dir/*.lib
rm -f working_dir/*.pdb
rm -f working_dir/*.ilk
rm -f working_dir/*.manifest
rm -f working_dir/*.msi
rm -f working_dir/*.wixpdb

