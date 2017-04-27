#!/bin/bash


Candle="${PROGRAMFILES} (x86)/WiX Toolset v3.10/bin/candle.exe"
Light="${PROGRAMFILES} (x86)/WiX Toolset v3.10/bin/light.exe"
BuildDir=../../build_pm/
OutDir=../../working_dir/

echo "clean old build files"
rm -f ${BuildDir}/*.wixobj
rm -f ${OutDir}/*.msi
rm -f ${OutDir}/*.wixpdb

echo "start candle..."
"$Candle" -nologo -arch x64 -out "${BuildDir}/directories.wixobj" "./directories.wxs" || exit $?

"$Candle" -nologo -arch x64 -out "${BuildDir}/files.wixobj" "./files.wxs" || exit $?

"$Candle" -nologo -arch x64 -out "${BuildDir}/features.wixobj" "./features.wxs" || exit $?

"$Candle" -nologo -arch x64 -out "${BuildDir}/main.wixobj" "./main.wxs" || exit $?

echo "start light..."
"$Light" -nologo -out "${OutDir}/PrivacyMachine_Windows_0_10_0.msi" -ext "WixUIExtension" "-loc" "../CustomStrings.wxl"  "${BuildDir}/directories.wixobj" "${BuildDir}/files.wixobj" "${BuildDir}/features.wixobj" "${BuildDir}/main.wixobj" || exit $?

echo "build finished"

