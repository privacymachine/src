rem  This file starts a clean build from windows

rem  The cleanup script also works from windows (inside a bash)
call "%LOCALAPPDATA%\Programs\Git\git-bash.exe" cleanAll.sh

rem  Set some environment variables needed for compilation 
call "%VS120COMNTOOLS%../../VC/vcvarsall.bat" amd64

cmake -G"NMake Makefiles" ..
nmake all
pause
