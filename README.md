# src

## Here is the published source-code of the project PrivacyMachine

Check out our homepage [https://www.privacymachine.eu](https://www.privacymachine.eu)!

We currently using this github-repository for publishing releases. The development occurs on a more private git-repository.

__Status__: **First Beta-Version for public use!**
  
For Windows-Users we have an Installer, for Linux-Users the compile manual below.

The automatic update is not ready but you will be notified when a new version is available.

### Requirements

* VirtualBox maintained by Oracle (currently 5.0.x or 5.1.*)

* VirtualBox Extension Pack is needed (please download and install it from [https://www.virtualbox.org/wiki/Downloads](https://www.virtualbox.org/wiki/Downloads)  
   This is needed because we currently use the VRDP-Server from VirtualBox  
   The long term plan is to replace it with the FreeRDP-Server  

* Hardware Virtualisation Support (Intel-VTx/AMD-V) is needed to be enabled in the UEFI/BIOS because the BaseDisk is running under 64bit.

### Build-HowTo Linux

* check that you have these packages installed:  
  
  __debian jessie:__ list can maybe smaller, but this works:  
  ```
  apt install build-essential git-core cmake qt5-default qtmultimedia5-dev libssl-dev libx11-dev libxext-dev libxinerama-dev libxcursor-dev libxdamage-dev libxv-dev libxkbfile-dev libasound2-dev libcups2-dev libxml2 libxml2-dev libxrandr-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libxi-dev libgstreamer-plugins-base1.0-dev xmlto libxtst-dev libavcodec-dev libavutil-dev libjpeg-dev genisoimage squashfs-tools sshpass qttools5-dev qttools5-dev-tools p7zip-full
  ```
  
  __ubuntu 16.04:__
  ```
  apt install build-essential git-core cmake zlib1g-dev qt5-default qtmultimedia5-dev qttools5-dev-tools libssl-dev sshpass p7zip-full libpopt-dev libbz2-dev
  ```
  
* Start the build process  
   ./buildAll.sh  

* Launch the PrivacyMachine!  
   cd working_dir  
   ./PrivacyMachine  

### Build-HowTo Windows

* Install cygwin

* Install VisualStudio Community Edition 2015 English
  * Deselect all
  * Select: Programming Language->VisualC++->Common Tools

* If running as jenkins-slave: Install Java JDK

* Install Qt via the Online Installer (OpenSource Edition)

  * https://www.qt.io/download-open-source/
  * Install to C:\\Qt
  * Deselect Qt 5.6
  * Select Qt 5.7->msvc2015 64bit
  * Deselect Qt 5.7->Qt Purchasing
  * Deselect Qt 5.7->Qt WebEngine
  * Deselect Qt 5.7->Qt SCXML
  * Deselect Qt 5.7->Qt SerialBus
  * Deselect Qt 5.7->Qt Gamepad
  * Deselect Qt 5.7->Qt Script

* Add to environment variable PATH of whole System C:\\Qt\\5.7\\msvc2015_64\\bin

* Install cmake

  * https://cmake.org/download/
  * Install via 64Bit Installer
    * Select option: "Add CMake to system PATH for all users

* Install OpenSSL

  * Use the Installer "Win64 OpenSSL v1.0.*" from https://slproweb.com/products/Win32OpenSSL.html to install Win64 OpenSSL v1.0.2j to C:\OpenSSL-Win64 directory
  * Use Option: "Copy OpenSSL-DLL's to /bin directory"

* Install Wix Toolset: Wix 3.10.3

* Install JOM

  * Download http://download.qt.io/official_releases/jom/jom.zip
  * extract to i.e. C:\\Qt\\jom
  * set environment variable PATH to the folder above

* Install Popt for Windows from http://downloads.sourceforge.net/project/gnuwin32/popt/1.8-1/popt-1.8-1-lib.exe

  * install to C:\\Program Files (x86)\\GnuWin32



### Code reviews and design suggestions are very welcome!

Please write us at contact@privacymachine.eu
PGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

If you want to join us you are very welcome!  
We are looking forward for a short design review before you develop bigger changes, so we can discuss how merge them together.
