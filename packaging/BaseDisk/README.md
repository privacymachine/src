# Documentation of the BaseDisk-Build-Process
Because of the higher stability we use libvirt for our build process.

## To build your own BaseDisk follow this steps (tested on debian jessie)

1. Install the required packages and cofigure libvirt
    1. Packages:
        * libvirt0
        * virt-manager
        * qemu-kvm
        * python3
        * zerofree
        * rdiff
        * p7zip-full
    2. Setup libvirt and a network bridge (https://wiki.libvirt.org/page/Networking#Debian.2FUbuntu_Bridging)


2. Create a target directory for the BaseDisk builder (i.e. /opt/BaseDisk) and cd into it
    1. Make sure there are at least 20GB of free space
    2. Download "generateInitialConfig.py", "libBaseDiskBuild.py", "libVM.py", "cleanup.py" and "buildNewBaseDisk.py"
    3. Download the latest pm_files from http://unstable.privacymachine.eu/BaseDisk_build/pm_files/ and extract them
    4. Download latest guestadditions.iso from http://download.virtualbox.org/virtualbox/ and create a symlink:  
         ln -s VBoxGuestAdditions_5.1.26.iso VBoxGuestAdditionsUsed.iso
    5. Create a subfolder "grmlWithSshKey" and extract http://unstable.privacymachine.eu/BaseDisk_build/grmlWithSshKey/grmlWithSshKey.tar there

3. Edit "generateInitialConfig.py" and run it afterwards to create the initial configuration as well as the folder structure

4. Make sure you have a configured network bridge for libvirt 

5. Start "virt-manager"  
    1. File -> New Virtual Machine
    2. Select: Import existing disk image
    3. Hit: Forward
    4. As storage path select the empty-flat.vmdk in the target directory (Browse Local)
    5. OS type = Linux
    6. Version = Debian Wheezy (or later)
    7. Hit: Forward
    8. (optional:) select 2 CPUs
    9. Hit: Forward
    10. Name = vm_BaseDisk_build
    11. Select: Customize before install
    12. Open: Advanced options
    13. Select: Specify shared device name 
    14. Fill in the bridge name
    15. Hit: Finish
    16. Hit: Add Hardware
    17. Select: Storage
    18. Device Type = CDROM device
    19. As storage select grmlWithSshKey.iso (Browse Local)
    20. Hit: Finish
    16. Hit: Add Hardware
    17. Select: Storage
    18. Device Type = CDROM device
    19. As storage select VBoxGuestAdditionsUsed.iso (Browse Local)
    20. Hit: Finish   
    21. Select: Disk 1
    22. Open: Advanced options
    23. Disk bus = SATA
    24. Hit: Apply
    25. Select: Boot options
    26. Change to:  
        bootable: SATA Disk 1  
        bootable: IDE CDROM 1  
        not-bootable: IDE CDROM 2  
        not-bootable: NIC  
    27. Hit: Apply
    28. Hit: Begin Installation, but "Force off the VM" at boot prompt.

6. run "buildNewBaseDisk.py"
**In case of a failure nothing gets removed so you can debug manually. Run "cleanup.py" to remove all old files.**
   

## File documentation

### generateInitialConfig.py
This script generates the initial config, creates the folder structure and is **should be edited by the user before first run**. Because of the limitations of JSON files and the fact that buildNewBaseDisk.py updates its own config this script provides a easy way to generate an initial config. The configuration parameters are documented in this python script. Make sure you have no additional witespaces in lines ending with a backslash.

### buildNewBaseDisk.py
This script reads the configuration file "buildBaseDiskConfig.json" and triggers the build process for a BaseDisk.
**"buildBaseDiskConfig.json" will be updated after each successful run to store the required metadata!**
It also manages the generation of BaseDisk-deltas (incremental BaseDisk updates with rdiff) as well as the optional signing and uploading process.

### cleanup.py
This scripts delete the meta-data from the config file **buildBaseDiskConfig.json**
Also the following directories are cleared:
* BaseDisk_delta
* BaseDisk_image
* BaseDisk_log
* BaseDisk_signature
  
### libVM.py
This is a python class that allows communication with the VM over the libvirt command line tools and via SSH.

### libBaseDiskBuild.py
This is a python library that controls the actual build process of the BaseDisks. It manages the bootstrap (basic debian install) as well as the installation of additional packages and the VirtualBox guest additions.

### pm_files
This folder contains files which are copied to the BaseDisks. I.e. systemd scripts, openbox configurations....  
**Note: Neider "pm_files/pm/fonts" nor "pm_files/home/liveuser/.mozilla" are part of the git repository!**  
This is because of the size of this folders, for a complete download check out http://unstable.privacymachine.eu/BaseDisk_build/pm_files/


## Tips ans Tricks

### HowTo mount (loopback) the BaseDisk

1. mkdir mounted_BaseDisk  
2. losetup -o 1048576 /dev/loop0 empty-flat.vmdk  
3. mount /dev/loop0 mounted_BaseDisk  

