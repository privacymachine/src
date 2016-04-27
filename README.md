# src

## Here is the published source-code of the project PrivacyMachine

Check out our homepage [https://www.privacymachine.eu](https://www.privacymachine.eu)!

We currently using this github-repository for publishing releases. The development occurs on a more private git-repository.

__Status__: It's a working Demo with some limitations.  

__Just a demo-version at the Moment! The fingerprint obfuscation is incomplete and applying of browser security updates is missing currently!!__
  
__So use it carefully!!__

__Currently only linux is supported.__


### Build-HowTo 

1. check that you have these packages installed (tested on ubuntu 14.04 and debian jessie, list can maybe smaller, but this works):
build-essential git-core cmake qt5-default qtmultimedia5-dev libssl-dev libx11-dev libxext-dev libxinerama-dev libxcursor-dev libxdamage-dev libxv-dev libxkbfile-dev libasound2-dev libcups2-dev libxml2 libxml2-dev libxrandr-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libxi-dev libgstreamer-plugins-base1.0-dev xmlto libxtst-dev libavcodec-dev libavutil-dev libjpeg-dev genisoimage squashfs-tools sshpass qttools5-dev qttools5-dev-tools

2. "base-disk.vdi": This is used as immutable disk in all new created virtual machines
   The build-process of this disk is located in packaging/build_base-disk, but it has many external dependencies which are currently only partial documented.
   Please use the ready to use disk from here:     
   cd pm/build/pm  
   wget https://github.com/privacymachine/src/releases/download/v0.1.2/base-disk.vdi.7z.sfx  
   extract with:  
   ./base-disk.vdi.7z.sfx      or      7za e base-disk.vdi.7z.sfx
   cd ..

3. We are using two external github repositories which you have to checkout: FreeRDP and RemoteDisplay  
   cd ext_libs  
   ./checkoutExternal.sh  

4. Start the build process  
   cd build  
   ./buildAll.sh  

5. Launch the PrivacyMachine!  
   cd build/pm  
   ./PrivacyMachine  


__Code reviews and design suggestions are very welcome!__

Please write us at contact@privacymachine.eu  
PGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242
