# Deployment of files

### some definitions
{INSTALL_DIR} ... PmManager::getPmInstallDir()  
{USER_CONFIG_DIR} ... PmManager::getPmConfigDir()  + also used in INI-Files!  

## Windows

The buildmachine builds one package which can be installed on Win7 and Win10: PrivacyMachine_win64_W_M_N.msi  
The default install folder is "C:\Program Files\PrivacyMachine" which can be changed to i.e. "E:\pm".  
In folders like "C:\Program Files\" programs have no write access without show an UAC-Prompt.  


#### Folder structure after the setup  

* C:\Program Files\PrivacyMachine or user-selected i.e. E:\pm -> {INSTALL_DIR}  
* {INSTALL_DIR}\conf\PrivacyMachine_Example_de.ini
* {INSTALL_DIR}\conf\PrivacyMachine_Example_en.ini
* {INSTALL_DIR}\PrivacyMachine.exe
* {INSTALL_DIR}\RemoteDisplay.dll
* {INSTALL_DIR}\platforms\qwindows.dll (needed by Qt)
* {INSTALL_DIR}\BaseDisk\ (Folder for files base_1.vmdk + base_1_flat.vmdk)  


#### On first start

**{USER_CONFIG_DIR} ... %UserProfile%\PrivacyMachine**

On first start the PrivacyMachine creates the folder %UserProfile%\PrivacyMachine and copy the file {INSTALL_DIR}\conf\PrivacyMachine_Example_{LANG}.ini to %UserProfile%\PrivacyMachine\PrivacyMachine.ini  
{LANG} is based on the system language   

{USER_CONFIG_DIR}\PrivacyMachineInternals.ini is also created and contains i.e. the Path to the (in future changeable) BaseDisk-folder


#### Folder structure after first run

%UserProfile%\PrivacyMachine\PrivacyMachine.ini (VPN-Paths: {USER_CONFIG_DIR}/vpn/TigerVPN, {USER_CONFIG_DIR}/vpn/CryptoFree)
%UserProfile%\PrivacyMachine\PrivacyMachineInternals.ini
%UserProfile%\PrivacyMachine\vpn\


## Linux

** Draft - not implemented **
The buildmachines build packages for each distro i.e. PrivacyMachine_debian-jessie_W_M_N.deb  
The packages installes the following files to /opt/privacymachine

#### Folder structure after the setup

* /opt/privacymachine/bin/PrivacyMachine (executable)  
* /opt/privacymachine/lib/libRemoteDisplay.so
* /opt/privacymachine/conf/PrivacyMachine_Example_de.ini  (template which gets copied to {USER_CONFIG_DIR})
* /opt/privacymachine/conf/PrivacyMachine_Example_en.ini  (template which gets copied to {USER_CONFIG_DIR})

#### On first start

**{USER_CONFIG_DIR} ... ~/.config/privacymachine**

On first start the PrivacyMachine creates the folder ~/.config/privacymachine and copy the file PrivacyMachine_Example_{LANG}.ini to ~/.config/privacymachine/PrivacyMachine.ini  
{LANG} is based on the system language   

{USER_CONFIG_DIR}\PrivacyMachineInternals.ini is also created and contains i.e. the Path to the (in future changeable) BaseDisk-folder


#### Folder structure after first run

* ~/.config/privacymachine/PrivacyMachine.ini (VPN-Paths: {USER_CONFIG_DIR}/vpn/TigerVPN, {USER_CONFIG_DIR}/vpn/CryptoFree)
* ~/.config/privacymachine/PrivacyMachineInternals.ini (internal configuration)  
* ~/.config/privacymachine/vpn/ (location for User-Configured VPNs like TigerVPN)


## Update of Configs

The folder {USER_CONFIG_DIR}/conf will be updated.  
The folder {INSTALL_DIR}/conf remains at the state at install time.  
If changes to PrivacyMachine.ini are needed the User has to do them manually or in future the PrivacyMachine is able to patch the file itself.

#### Update of Binaries

** Draft - not implemented **
At least at Windows the exe and dll's are updated via the internal update mechanism
