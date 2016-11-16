
## Here is the python3 script to build the template VM.
This script only works on Linux (tested on debian jessie)

1. Make shure you have installed kvm, libvirt, python3 and ssh 
2. Download a grml64 ISO from http://grml.org/download/
3. Use this guide to customize the ISO with your ssh-key: http://blog.grml.org/archives/367-Create-a-Grml-ISO-image-with-your-own-ssh-keys-for-password-less-login.html
4. Download Virtualbox-GuestAdditions ISO http://download.virtualbox.org/virtualbox/5.0.0_RC3/VBoxGuestAdditions_5.0.0_RC3.iso
5. Modify the xml file "base-disk-build.xml_grml" to be a kvm configuration witch boots your build vm with grml
6. Modify the xml file "base-disk-build.xml_guestadditions" to be a kvm configuration witch boots your build vm from disk
7. Modify the configuration file "prepare_BaseDisk_update.config.json"
8. copy fontfiles to pm_files/pm/fonts
9. copy a firefox ptofile to pm_files/home/liveuser/.mozilla
10. Execute "python3 prepare_BaseDisk_update.py prepare_BaseDisk_update.config.json"




In case of failure nothing gets cleand up so you can debug,
this means you have to cleanup by your own
