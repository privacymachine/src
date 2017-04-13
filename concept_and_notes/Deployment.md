# Deployment of files

## Linux

The buildmachines build packages for each distro i.e. PrivacyMachine_debian-jessie_W_M_N.deb  
The packages installes the following files to /opt/privacymachine

#### Folder structure after the setup

* /opt/privacymachine/bin/PrivacyMachine (executable)  
* /opt/privacymachine/lib/libRemoteDisplay.so
* /opt/privacymachine/conf/vpn/CryptoFree/ (maintained by the PM-Team)  
* /opt/privacymachine/conf/PrivacyMachine_Example_de.ini  
* /opt/privacymachine/conf/PrivacyMachine_Example_en.ini  
* /opt/privacymachine/BaseDisk/ (Folder for files base_1.vmdk + base_1_flat.vmdk)  

#### On first start

On first start the PrivacyMachine creates the folder ~/.config/privacymachine (={USER_CONFIG_DIR}) and copy based on the system language the file PrivacyMachine_Example_{LANG}.ini to ~/.config/privacymachine/PrivacyMachine.ini  

~/.config/privacymachine/PrivacyMachineInternals.ini contains the Path to the (in future changeable) BaseDisk-folder and the {INSTALL_DIR}, which is always /opt/privacymachine on linux  

#### Folder structure after first run

* ~/.config/privacymachine/vpn/ (location for User-Configured VPNs like TigerVPN)
* ~/.config/privacymachine/PrivacyMachine.ini (VPN-Paths: {USER_CONFIG_DIR}/vpn/TigerVPN, {INSTALL_DIR}/vpn/CryptoFree)
* ~/.config/privacymachine/PrivacyMachineInternals.ini (internal configuration)  


## Windows

The buildmachine builds one package which can be installed on Win7 and Win10: PrivacyMachine_win64_W_M_N.msi  
The default install folder is C:\Program Files\PrivacyMachine which can be changed to i.e. E:\pm.


#### Folder structure after the setup  

* C:\Program Files\PrivacyMachine or user-selected i.e. E:\pm -> {INSTALL_DIR}  
* {INSTALL_DIR}\conf\PrivacyMachine_Example_de.ini
* {INSTALL_DIR}\conf\PrivacyMachine_Example_en.ini
* {INSTALL_DIR}\PrivacyMachine.exe
* {INSTALL_DIR}\RemoteDisplay.dll
* {INSTALL_DIR}\platforms\qwindows.dll (needed by Qt)
* {INSTALL_DIR}\BaseDisk\ (Folder for files base_1.vmdk + base_1_flat.vmdk)  


#### On first start

On first start the PrivacyMachine creates the folder %AppData%\PrivacyMachine (={USER_CONFIG_DIR}) and copy based on the system language the file {INSTALL_DIR}\conf\PrivacyMachine_Example_{LANG}.ini to %AppData%\PrivacyMachine\PrivacyMachine.ini  

%AppData%\PrivacyMachine\PrivacyMachineInternals.ini contains the Path to the (in future changeable) BaseDisk-folder and the {INSTALL_DIR} (i.e. E:\pm)  


#### Folder structure after first run

%AppData%\PrivacyMachine\PrivacyMachine.ini (VPN-Paths: {USER_CONFIG_DIR}/vpn/TigerVPN, {INSTALL_DIR}/vpn/CryptoFree)
%AppData%\PrivacyMachine\vpn\TigerVPN\
%AppData%\PrivacyMachine\PrivacyMachineInternals.ini


## Update of Configs

The folder {INSTALL_DIR}/conf will be updated.  
If changes to PrivacyMachine.ini are needed the User has to do them manually or the PrivacyMachine is able to patch the file

#### Update of Binaries

At least at Windows the exe and dll's are updated via the internal update mechanism
