#!/bin/bash

echo "clean content of the build_pm dir"
if [ -d build_pm ]; then
  rm -rf build_pm/*
fi

echo "remove files from working_dir..."
rm -f working_dir/pmTests
rm -f working_dir/logs/*
rm -f working_dir/UserConfigTest
rm -f working_dir/GuiTest
rm -f working_dir/GuiTest.exe
rm -f working_dir/PrivacyMachine
rm -f working_dir/PrivacyMachine.exe
rm -f working_dir/PrivacyMachine.exe.manifest
rm -f working_dir/ProblemReporter
rm -f working_dir/ProblemReporter.exe
rm -f working_dir/ProblemReporter.exe.manifest
rm -f working_dir/TestPatching
rm -f working_dir/TestPatching.exe
rm -f working_dir/TestPatching.exe.manifest
rm -f working_dir/TestGui
rm -f working_dir/TestGui.exe
rm -f working_dir/TestGui.exe.manifest
rm -f working_dir/TestUserConfigOpenVPN
rm -f working_dir/TestUserConfigOpenVPN.exe
rm -f working_dir/TestUserConfigOpenVPN.exe.manifest
rm -f working_dir/TestXmlUpdateParser
rm -f working_dir/TestXmlUpdateParser.exe
rm -f working_dir/TestXmlUpdateParser.exe.manifest
rm -f working_dir/testresults.xml
rm -f working_dir/UserConfigTest.exe
rm -f working_dir/UserConfigTest.exe.manifest
rm -f working_dir/*.msi
rm -f working_dir/*.wixpdb

