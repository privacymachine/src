# -*- coding: utf-8 -*-
"""
Created on Wed Mar 16 19:00:26 2016

@author: olaf
"""

from __future__ import print_function
from libVM import VM
import subprocess as sp
import time
import os
import json




#===================== Exexute a shell command ====================================
def cmd_exec(cmd,args,logger,output=False):
    cmd_str =cmd
    for arg in args:
        cmd_str += ' ' + arg
    logger.info('Try executing Command "'+cmd_str+'"')
    try:
        str1 = sp.check_output([cmd]+args, stderr=sp.STDOUT).decode("utf-8")
    except sp.CalledProcessError as err:
        logger.fatal('Execution failed\n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
        if output:        
            return err.returncode, err.output
        else:
            return err.returncode
    logger.info('SUCCESS: executed '+cmd_str)
    if output:
        return 0, str1
    else:
        return 0



#*********************************************************************************#
#**********************  MAIN ROUTINE  *******************************************#
#*********************************************************************************#

def build_BaseDisk(build_config,BaseDisk,logger):

    # get build time and add it to capabilities
    now=time.localtime()
            
    capabilities={ \
                    'build_date':{ \
                                    'day':now.tm_mday,
                                    'month':now.tm_mon,
                                    'year':now.tm_year,
                                    'hour':now.tm_hour,
                                    'minute':now.tm_min,
                                    'secound':now.tm_sec}}
    

    #  variable for error checking
    returns=[]    
    
    #define VM handler
    buildVM = VM('root','root',build_config['VM']['VM_name'],build_config['paths']['grml_ssh_priv_key'],'22','grml'+build_config['local_tld'],logger,build_config['VM']['boot_delay'])

    # force stop a running build VM

    try:
        str1 = sp.check_output(['/usr/bin/virsh', 'list'], stderr=sp.STDOUT).decode("utf-8")
    except sp.CalledProcessError as err:
        logger.fatal('command: "/usr/bin/virsh list" failed \n cmd: '+ \
                     str(err.cmd)+'\n output: '+str(err.output) + \
                     '\n returncode: ' +str(err.returncode))
        exit(1)
    except OSError as err:
        logger.fatal('command: "/usr/bin/virsh list" failed: '+err.strerror)
        exit(1)    
    if not (str1.find(buildVM.vmname) < 0):
        try:
            sp.check_call(['/usr/bin/virsh', 'destroy', buildVM.vmname], stderr=sp.STDOUT)
        except sp.CalledProcessError as err:
            logger.fatal('command: "/usr/bin/virsh destroy" failed \n cmd: '+ \
                         str(err.cmd)+'\n output: '+str(err.output) + \
                         '\n returncode: ' +str(err.returncode))
            exit(1)
        except OSError as err:
            logger.fatal('command: "/usr/bin/virsh destroy" failed: '+err.strerror)
            exit(1)    

    # clean build-VM
    returns.append(cmd_exec('dd',['if=/dev/zero','bs=1M','count='+str(build_config['VM']['disk_size']),'of='+build_config['VM']['disk_path']],logger))

    # define VM with grml ISO for debian install
    #returns.append(cmd_exec('virsh',['undefine',build_config['VM']['VM_name']],logger))
    #returns.append(cmd_exec('virsh',['define',build_config['paths']['grml_xml']],logger))
    
    # start the build-VM
    buildVM.start()
    time.sleep(build_config['VM']['boot_delay'])
    
    if buildVM.isonline():
                
        
        # Partitionate the VDI-File
        returns.append(buildVM.sshcmd('parted /dev/sda -s mklabel msdos'))
        returns.append(buildVM.sshcmd('parted /dev/sda -s mkpart primary 2048s 100%'))
        returns.append(buildVM.sshcmd('parted /dev/sda -s set 1 boot on'))
        returns.append(buildVM.sshcmd('mkfs.ext4 /dev/sda1'))
        returns.append(buildVM.sshcmd('mkdir /target'))
        returns.append(buildVM.sshcmd('mount /dev/sda1 /target'))
        # Bootstrap debian jessie
        returns.append(buildVM.sshcmd('grml-debootstrap --force --grub /dev/sda --target /target --password 123 --hostname PM --release jessie --mirror '+build_config['apt']['deb_proxie']+build_config['apt']['deb_mirror']+' 2>&1 | tee /grml-debootstrap.log ; sleep 3 ; service ssh start'))
        # setup /etc/apt/sources.list     
        returns.append(buildVM.sshcmd('echo "deb ' + build_config['apt']['deb_proxie']+build_config['apt']['deb_mirror'] + ' jessie main non-free contrib" > /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb '+ build_config['apt']['deb_proxie']+ 'security.debian.org/ jessie/updates main contrib non-free" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb ' + build_config['apt']['deb_proxie']+build_config['apt']['deb_mirror'] + ' jessie-backports main non-free contrib" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt update 2>&1 | tee /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install pkg-mozilla-archive-keyring 2>&1 | tee /apt.log'))
        returns.append(buildVM.sshcmd('echo "deb '+ build_config['apt']['deb_proxie']+ 'mozilla.debian.net/ jessie-backports firefox-release" >> /target/etc/apt/sources.list'))
        
        # update jessie       
        returns.append(buildVM.sshcmd('grml-chroot /target apt update 2>&1 | tee /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt full-upgrade -y 2>&1 | tee -a /apt.log'))
        
        # install openbox
        returns.append(buildVM.sshcmd('echo "#!/bin/bash" > /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('echo "export DEBIAN_FRONTEND=noninteractive" >> /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('echo "apt install -y openbox xinit xterm" >> /target/tmp/openbox_install.sh'))
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('echo "apt install -y menu" >> /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/bash /tmp/openbox_install.sh 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('rm /target/tmp/openbox_install.sh'))
        
        # install x11-xserver-utils (containing xrandr which is needed by the vbox-guest-additions to resize the window)
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y x11-xserver-utils  2>&1 | tee -a /apt.log'))

#        # install apt-listchanges
#        #TODO: configurate
#        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y apt-listchanges 2>&1 | tee -a /apt.log'))
        
        # install firefox
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y firefox 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y firefox-l10n-all 2>&1 | tee -a /apt.log'))
        capabilities['browser']={'firefox':{'start':'firefox.service'}}

        # install firefox-esr
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y firefox-esr 2>&1 | tee -a /apt.log'))        
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y firefox-esr-l10n-all 2>&1 | tee -a /apt.log'))                
        capabilities['browser']['firefox-esr']={'start':'firefox-esr.service'}
        
        # install htop
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y htop  2>&1 | tee -a /apt.log'))
        
        # install zsh
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y zsh  2>&1 | tee -a /apt.log'))
        
        # install vim
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y vim  2>&1 | tee -a /apt.log'))
        
        # install xdotool
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y xdotool  2>&1 | tee -a /apt.log'))        
        
        # install xz-utils
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y xz-utils 2>&1 | tee -a /apt.log'))        
 
        # install dkms
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y dkms 2>&1 | tee -a /apt.log'))  

        # install curl
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y curl 2>&1 | tee -a /apt.log'))  
        
        # install ntp
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y ntp  2>&1 | tee -a /apt.log'))
        
        # install openvpn
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y openvpn  2>&1 | tee -a /apt.log'))
        capabilities['vpn']=['openvpn']
        
        # install audio: install needed packages
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y alsa-base  2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y gstreamer1.0-alsa  2>&1 | tee -a /apt.log'))
        if build_config['debug_build']:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y alsa-utils  2>&1 | tee -a /apt.log'))
        
        # purge unnecessary packages
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get purge -y lvm2 cryptsetup mdadm 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get autoremove --purge -y 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get autoclean 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get clean 2>&1 | tee -a /apt.log'))

        # update package cache
        returns.append(buildVM.sshcmd('grml-chroot /target apt update 2>&1 | tee -a /apt.log'))

        # remove resolveconf (use without openVPN) and download the package for installing in case of openvpn
        returns.append(buildVM.sshcmd('grml-chroot /target apt remove -y resolveconf 2>&1 | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -d -y resolveconf 2>&1 | tee -a /apt.log'))
    
        # install all locales
        returns.append(buildVM.sshcmd('grml-chroot /target apt install locales-all 2>&1 | tee -a /apt.log'))
        
        # setup liveuser
        returns.append(buildVM.sshcmd('echo "#!/bin/bash" > /target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('echo "/usr/sbin/useradd -m liveuser" >>/target/tmp/add_liveuser.sh'))
        # install audio: add user to the audio group
        returns.append(buildVM.sshcmd('echo "/usr/sbin/addgroup liveuser audio" >>/target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('echo "echo liveuser:123 | chpasswd"  >>/target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/bash /tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('rm /target/tmp/add_liveuser.sh'))

        
        # copy ssh-key to new BaseDisk
        returns.append(buildVM.sshcmd('cp -r /root/.ssh /target/root/'))
        
        #######################################################################
        ## copy PM required scripts and files to new BaseDisk
        #######################################################################
            # cpoy pm recursive
        returns.append(buildVM.copyFiles2VM(build_config['paths']['pm_dir']+'/pm','/target'))                
            # containing:            
            #   script to download tor_browser
            #   script to set the keyboardlayout to <de>            
            #   fonts to obuscate fingerprint 
        capabilities['paths']={'fonts_dir':'/pm/fonts'}
        
            # copy var recursive
        returns.append(buildVM.copyFiles2VM(build_config['paths']['pm_dir']+'/var','/target'))
            # containing: 
            #   enable sound volume: /var/lib/alsa/asound.state
            
            # copy home recursive
        returns.append(buildVM.copyFiles2VM(build_config['paths']['pm_dir']+'/home','/target'))
            # containing: 
            #   Start vbox-service needed for clipboard-sharing: /home/liveuser/.config/openbox/autostart.sh
            #   preconfigured .mozilla directory 
            
            # copy etc recursive
        returns.append(buildVM.copyFiles2VM(build_config['paths']['pm_dir']+'/etc','/target'))
            # containing: 
            #   SSH config: /etc/ssh/sshd_config
            #   allow anybody to start  X-server:  /etc/X11/Xwrapper.config
            #   remove icons: iconify, close in title bar: /etc/X11/openbox/rc.xml
            #   install fixed DNS Server to /etc/resolv.conf
            #   systemd service to start openbox at systemstart: /etc/systemd/system/openbox.service
            #   systemd service to start firefox
            #   systemd service to start torbrowser
            #   systemd service to start openvpn 
            #   systemd service to display the openvpn log in xterm
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/systemctl enable openbox.service'))
            # if you want to use apt-cacher-ng:
            #    o) create the file pm_files/etc/apt/apt.conf.d/01proxy with a content like this
            #         Acquire::http { Proxy "http://192.168.0.10:3142"; }; # your local LAN-IP, your configured port of the apt-cacher-ng
            #         Acquire::https { Proxy "https://"; };
            #    o) the file will be copied automatically by the above 'copy etc recursive' 
            #    o) Don't add this to the git repo ;)

        # set file rights for liveuser (needed for autostart and clipboardsharing)        
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/chown -R liveuser:liveuser /home/liveuser'))
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/chmod +x /home/liveuser/.config/openbox/autostart.sh'))
        
        # set ownership of /pm to liveuser
        returns.append(buildVM.sshcmd('grml-chroot /target chown -R liveuser:liveuser /pm'))
        
        #######################################################################
        ## Speedup start
        #######################################################################
            # Setup fstab
        returns.append(buildVM.sshcmd('echo "/dev/sda1 / auto defaults,errors=remount-ro 0 0" > /target/etc/fstab'))
        returns.append(buildVM.sshcmd('echo "proc /proc proc defaults 0 0" >> /target/etc/fstab'))
        returns.append(buildVM.sshcmd('echo "tmpfs /tmp tmpfs defaults 0 0" >> /target/etc/fstab'))
        returns.append(buildVM.sshcmd('echo "tmpfs /var/log tmpfs defaults 0 0" >> /target/etc/fstab'))
        returns.append(buildVM.sshcmd('echo "tmpfs /var/cache tmpfs defaults 0 0" >> /target/etc/fstab'))
        returns.append(buildVM.sshcmd('echo "tmpfs /var/tmp tmpfs defaults 0 0" >> /target/etc/fstab'))                
        returns.append(buildVM.sshcmd('grml-chroot /target update-initramfs -u -k all'))
            # disable grub boot menue
        returns.append(buildVM.copyFiles2VM(build_config['paths']['pm_dir']+'/etc/default/grub', '/target/etc/default/grub'))
        returns.append(buildVM.sshcmd('grml-chroot /target update-grub'))
        
        # remove local proxie from /etc/apt/sources.list  
        returns.append(buildVM.sshcmd('echo "deb http://' + build_config['apt']['deb_mirror'] + ' jessie main non-free contrib" > /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb http://' + 'security.debian.org/ jessie/updates main contrib non-free" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb http://' + build_config['apt']['deb_mirror'] + ' jessie-backports main non-free contrib" >> /target/etc/apt/sources.list'))

        # copy the log files and stop the VM    
        buildVM.copyFiles2Host('/apt.log',BaseDisk['log_dir']+'/apt.log')
        buildVM.copyFiles2Host('/grml-debootstrap.log',BaseDisk['log_dir']+'/grml-debootstrap.log')
        buildVM.stop()
        
        
        #######################################################################
        ## do post bootstrap work
        #######################################################################        
                               
        # set actual hostname
        buildVM.url='PM'+build_config['local_tld']
        buildVM.start()
        if buildVM.isonline():
            returns.append(buildVM.sshcmd('touch /post_build.log'))
        
            # mount the right CDROM (guestadditions.iso)
            returns.append(buildVM.sshcmd('mount /dev/sr1 /mnt'))
            if not buildVM.sshcmd('ls /mnt/VBoxLinuxAdditions.run') == 0:
                returns.append(buildVM.sshcmd('umount /mnt'))
                returns.append(buildVM.sshcmd('mount /dev/sr0 /mnt'))
            # install virtualbox-guest additions
            returns.append(buildVM.sshcmd('/mnt/VBoxLinuxAdditions.run 2>&1 | tee -a /post_build.log'))

            # install torbrowser
            returns.append(buildVM.sshcmd('/bin/chown -R liveuser:liveuser /pm'))
            returns.append(buildVM.sshcmd('chmod a+x /pm/download_tor_browser.sh'))
            returns.append(buildVM.sshcmd('su liveuser -c /pm/download_tor_browser.sh '+build_config['apt']['deb_proxie']))
            capabilities['browser']['torbrowser']={'start':'torbrowser.service'}
            
            # create timezones.txt which includes all chooseable timezones
            returns.append(buildVM.sshcmd('timedatectl list-timezones > /timezones.txt'))
        
            # create locales.txt which includes all chooseable locales
            returns.append(buildVM.sshcmd('localectl list-locales > /locales.txt'))
            
            # copy timezones and locales  
            buildVM.copyFiles2Host('/timezones.txt',BaseDisk['image_dir']+'/timezones.txt')
            buildVM.copyFiles2Host('/locales.txt',BaseDisk['image_dir']+'/locales.txt')

             # copy the log file
            buildVM.copyFiles2Host('/post_build.log',BaseDisk['log_dir']+'/post_build.log')
            
            # copy BaseDisks ssh-host-pub-key
            buildVM.copyFiles2Host('/etc/ssh/ssh_host_rsa_key.pub',BaseDisk['image_dir']+'/ssh_host_rsa_key.pub')

            # cleanup            
            returns.append(buildVM.sshcmd('rm /post_build.log'))
            returns.append(buildVM.sshcmd('rm /timezones.txt'))
            returns.append(buildVM.sshcmd('rm /locales.txt'))
            
            # shutdown VM
            buildVM.stop()
            
            # write fonts to capabilities
            installed_fonts = os.listdir(build_config['paths']['pm_dir']+'/pm/fonts')
            capabilities['fonts']=installed_fonts            

    
    # check if anything went wrong
    if all(map(lambda x:x==0,returns)):
        logger.info('>>>>>>>>>>>>>   SUCCESS: installing debian   >>>>>>>>>>>>>>')
        logger.info('copy finished BaseDisk')
        image_name = BaseDisk['image_identifier']+'_flat.vmdk'
        BaseDisk['flat_vmdk_path'] = BaseDisk['image_dir'] +'/'+ image_name
        cmd_exec('cp',[build_config['VM']['disk_path'], BaseDisk['flat_vmdk_path']] ,logger)
                        
        capabilities['version'] = BaseDisk['version']
                                   
        logger.info('collect data for capabilities.json')   
        with open(BaseDisk['image_dir']+'/ssh_host_rsa_key.pub','r') as f:
            capabilities['ssh_keys']={'BaseDisk_pub':f.read()}
        cmd_exec('rm',[BaseDisk['image_dir']+'/ssh_host_rsa_key.pub'],logger)
        with open(buildVM.sshcmdkey,'r') as f:
            capabilities['ssh_keys']['PM_priv']=f.read()
        timezones=[]
        with open(BaseDisk['image_dir']+'/timezones.txt','r') as f:
            for line in f:
                timezones.append(line.strip('\n'))
        capabilities['timezones']=timezones
        cmd_exec('rm',[BaseDisk['image_dir']+'/timezones.txt'],logger)
        locales=[]
        with open(BaseDisk['image_dir']+'/locales.txt','r') as f:
            for line in f:
                locales.append(line.strip('\n'))
        capabilities['locales']=locales
        cmd_exec('rm',[BaseDisk['image_dir']+'/locales.txt'],logger)

        logger.info('write capabilities.json')  
        BaseDisk['capabilities_path'] = BaseDisk['image_dir'] + '/' + BaseDisk['image_identifier']+'_capabilities.json'
        with open(BaseDisk['capabilities_path'],'w') as f:
            json.dump(capabilities,f)
            
        logger.info('zerofree'+ BaseDisk['flat_vmdk_path'])
        cmd_exec('losetup',['-D'],logger)
        cmd_exec('losetup',['-o',str(2048*512),'/dev/loop0',BaseDisk['flat_vmdk_path']],logger)
        cmd_exec('zerofree',['/dev/loop0'],logger)
        cmd_exec('losetup',['-D'],logger)
        
        logger.info('>>>>>>>>>>>>>>>>>>   SUCCESS: BaseDisk builded    >>>>>>>>>>>>>>>>>>')
        
        return BaseDisk
        
    else:
        logger.fatal('A error occured building the BaseDisk')
        logger.fatal('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
        logger.fatal('   please debug the VM '+build_config['VM']['VM_name']+' on your own')
        logger.fatal('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
        exit(1)
        
 
#*********************************************************************************#
#**********************  END OF MAIN ROUTINE  ************************************#
#*********************************************************************************#

       
