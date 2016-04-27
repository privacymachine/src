# -*- coding: utf-8 -*-
"""
Created on Wed Mar 16 19:00:26 2016

@author: olaf
"""


import logging
import subprocess as sp
import time


class VM:

    #============   Constructor of class VM, set initial values   =============
    def __init__(self,vmuser,vmname,sshkey,port,url,bootDelay = 90,logger=None):      
        self.vmuser = str(vmuser)
        self.vmname = str(vmname)
        self.sshcmdkey = str(sshkey)
        self.port = str(port)
        self.url = str(url)
        self.bootDelay = int(bootDelay)
        if type(logger)==logging.Logger:
            self.logger=logger
        else:
            self.logger=logging.getLogger('VM '+self.vmname)
            

    #===================== check if vm is online ==============================
        
    def isonline(self):   
        self.logger.info('test if VM '+self.vmname+' is online')
        for n in range(3):
            try:
                str1 = sp.check_output(['vboxmanage', 'list', 'runningvms'],
                                        stderr=sp.STDOUT)        
            except sp.CalledProcessError as err:
                self.logger.fatal('command: "vboxmanage list runningvms" failed \n cmd: '+str(err.cmd)+'\n output: '+str(err.output) + '\n returncode: ' +str(err.returncode))                          
            
            if str1.decode("utf-8").find(self.vmname) < 0:
                
                self.logger.info('SUCCESS: "vboxmanage list runningvms": '+self.vmname+' is offline')
                return False
                
            self.logger.info('SUCCESS: "vboxmanage list runningvms": '+self.vmname+' seems to be online, now test ssh')

            tmp = self.sshcmd('echo',True)
            if not tmp[0]==0 and n==0:     
                self.logger.info('SSH test 1 of 3 failed, will try again after '+ \
                '3 secounds')
                time.sleep(3)
                continue
            elif not tmp[0]==0 and n==1:
                self.logger.info('SSH test 2 of 3 failed, will try again after '+ \
                str(self.bootDelay)+' secounds')
                time.sleep(self.bootDelay)
                continue
            elif not tmp[0]==0:
                self.logger.warn(' SSH test failed three times!\n VM '+ \
                self.vmname+' seems online but cold not be reached by ssh.\n'+ \
                '\t check server functionallity and ssh parameter:\n\t port='+ \
                self.port+'\n\t keyfile='+self.sshcmdkey+ \
                '\n\t user='+self.vmuser+'\n\t server='+self.url  )
                
                self.online=True                          
                return self.online
                
        self.online=True
        self.logger.info('SUCCESS: VM '+self.vmname+' is online')                            
        return self.online
        
        
        

    #============================ copy files 2 host ===============================        
    def copyFiles2Host(self,source,target):
        ssh_identify_str = self.vmuser+'@'+self.url+':'+source        
        self.logger.info(self.vmname+': copy '+ssh_identify_str+' to '+target)        
        try:
            str1 = sp.check_output(['scp','-r',
                                    '-P',self.port,
                                    '-i',self.sshcmdkey,
                                    '-o','StrictHostKeyChecking=no',
                                    '-o','UserKnownHostsFile=/dev/null',
                                    '-o','IdentitiesOnly=yes',
                                    ssh_identify_str, target],
                                    stderr=sp.STDOUT)

        except sp.CalledProcessError as err:
            self.logger.fatal('cold not copy '+ ssh_identify_str +' to '+target+'\n cmd: '+str(err.cmd)+'\n output: '+str(err.output) + '\n returncode: ' +str(err.returncode))
            return err.returncode
        self.logger.info('SUCCESS: '+self.vmname+': copied files\n output:'+str1.decode("utf-8"))
        return 0
        
        
    #============================ copy files 2 VM =================================        
    def copyFiles2VM(self,source,target):
        ssh_identify_str = self.vmuser+'@'+self.url+':'+target
        self.logger.info(self.vmname+': copy '+source +' to '+ ssh_identify_str )
        try:
            str1 = sp.check_output(['scp','-r',
                                    '-P',self.port,
                                    '-i',self.sshcmdkey,
                                    '-o','StrictHostKeyChecking=no',
                                    '-o','UserKnownHostsFile=/dev/null',
                                    '-o','IdentitiesOnly=yes',
                                    source, ssh_identify_str],
                                    stderr=sp.STDOUT)

        except sp.CalledProcessError as err:
            self.logger.fatal('cold not copy '+source+' to '+ssh_identify_str+'\n cmd: '+str(err.cmd)+'\n output: '+str(err.output) + '\n returncode: ' +str(err.returncode))
            return err.returncode
        self.logger.info('SUCCESS: '+self.vmname+': copied files\n output:'+str1.decode("utf-8"))
        return 0
        
        
    #============================ execute command over ssh ========================        
    def sshcmd(self,cmd,returnOutput=False):
        self.logger.info('VM '+self.vmname+': sshcmd: '+cmd)
        params = ['ssh',
                  '-p',self.port,
                  '-i',self.sshcmdkey,
                  '-o','StrictHostKeyChecking=no',
                  '-o','UserKnownHostsFile=/dev/null',
                  '-o','IdentitiesOnly=yes',
                  self.vmuser+'@'+self.url, 
                  cmd]
        try:
            str1 = sp.check_output(params, stderr=sp.STDOUT)
        except sp.CalledProcessError as err:
            self.logger.error('sshcmd failed \n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
            if returnOutput:         
                return err.returncode,err.output
            else:
                return err.returncode
        self.logger.info('SUCCESS: VM '+self.vmname+': sshcmd: '+cmd+'\n output:'+str1.decode("utf-8"))
        if returnOutput:
            return (0,str1.decode("utf-8"))
        else:
            return 0  
    #============================ start VM ========================================   
    def start(self):    
        self.logger.info('start the VM '+self.vmname)
        try:
            str1 = sp.check_output(['vboxmanage','startvm',
                                    self.vmname,'--type','headless'],
                                    stderr=sp.STDOUT)
        except sp.CalledProcessError as err:
            self.logger.fatal('could not start VM '+self.vmname+'\n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
            return err.returncode
        self.logger.info('SUCESS: started the VM '+self.vmname+'\n output: '+str1.decode("utf-8"))
        return 0
        
    #============================ stop VM =========================================   
    def stop(self,shutdown_cmd='shutdown -hP 0'):    
        self.logger.info('try to stop the VM '+self.vmname)
        if self.isonline():
            self.sshcmd(shutdown_cmd)
            time.sleep(10)
            if self.isonline():           
                try:
                    str1 = sp.check_output(['vboxmanage','controlvm',
                                            self.vmname,'poweroff'],
                                            stderr=sp.STDOUT)
                except sp.CalledProcessError as err:
                    self.logger.fatal('could not stop VM '+self.vmname+'\n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
                    return err.returncode
        self.logger.info('SUCESS: stopped the VM '+self.vmname)
        return 0
        




#===================== Exexute a shell command ====================================
def cmd_exec(cmd,args,logger,output=False):
    cmd_str =cmd
    for arg in args:
        cmd_str += ' ' + arg
    logger.info('Try executing Command "'+cmd_str+'"')
    try:
        str1 = sp.check_output([cmd]+args, stderr=sp.STDOUT)
    except sp.CalledProcessError as err:
        logger.fatal('Execution failed\n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
        return err.returncode
    logger.info('SUCCESS: executed '+cmd_str)



#*********************************************************************************#
#*********************  MAIN PROGRAMM  *******************************************#
#*********************************************************************************#


if __name__=='__main__':


###############################################################################
###               general configuration parameter                           ###
###############################################################################   

    # the target directory for base_disk.vdi and the log files
    vdi_target_dir = 'PATH/to/empty/dir'

    # path to the customized grml-iso
    grml_iso_path = 'PATH/to/customized/grml64.iso'
    
    # the location of the SSH key to communicate with the VM (grml.iso)
    ssh_key_location = 'PATH/to/ssh/key/to/access/grml64'
    
    # the path to the folder "pm_files" in which scripts and files, required for starting PM, are stored
    pm_files_dir = 'pm_files'
        
    # size of the new VDI-file im MB
    hdd_size = 5000
    
    # the time in seconds to wait till the VM is online
    bootDelay = 90
    
    # a debian mirror
    deb_mirror = 'http://ftp.de.debian.org/debian'
    
    # the name the temporarly used VM will get in Virtualbox
    buildVM_name = 'removeMe_PM_build_VM' # no need to change
    
    # Port to cummunicate with the VM (choosen randomly)
    import random
    port = random.randint(10000,60000)
    
    # in developer_mode install services like htop, contextmenue, zsh, ... 
    developer_mode = True
    
###############################################################################    
    
    
    # convert current time to a timestamp        
    import re
    time_str=re.sub(r"\s+", "_",time.ctime())
    
    
    #====================== LOGGING ==========================================#
    # create logger with 'spam_application'
    logger = logging.getLogger('backup_logger')
    logger.setLevel(logging.DEBUG)

    # create file handler which logs even debug messages
    fh = logging.FileHandler(vdi_target_dir+'/'+time_str+'__base-disk.vdi.python_log')
    fh.setLevel(logging.DEBUG)

    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)

    # create formatter and add it to the handlers
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)

    # add the handlers to the logger
    logger.addHandler(fh)
    logger.addHandler(ch)
    #=========================================================================#
    

    # setup a build-VM
    cmd_exec('VBoxManage', ['createvm', '--name', buildVM_name, '--register'],logger)
    cmd_exec('VBoxManage', ['modifyvm', buildVM_name, '--memory','1024',
                            '--boot1','dvd','--ostype','Debian_64'],logger)    
    cmd_exec('VBoxManage', ['modifyvm', buildVM_name, '--nic1', 'nat'], logger)
    cmd_exec('VBoxManage', ['createhd','--filename', 
                            vdi_target_dir+'/BAD__'+time_str+'__base-disk.vdi',
                            '--size', str(hdd_size),'--format','VDI'],logger)
    cmd_exec('VBoxManage',['storagectl', buildVM_name, '--name', 'SATA1',
                           '--add', 'sata'],logger)
    cmd_exec('VBoxManage',['storageattach',buildVM_name,'--storagectl','SATA1',
                           '--port','0', '--device','0', '--type','hdd',
                           '--medium', vdi_target_dir+'/BAD__'+time_str+'__base-disk.vdi'], logger)
    cmd_exec('VBoxManage',['storageattach',buildVM_name,'--storagectl','SATA1',
                           '--port','1', '--device','0', '--type','dvddrive',
                           '--medium',grml_iso_path],logger)
                           
    cmd_exec('VBoxManage',['modifyvm',buildVM_name, 
                           '--natpf1', 'guestssh,tcp,,'+str(port)+',,22'],logger)
  
    buildVM = VM('root',buildVM_name,ssh_key_location,port,'localhost',bootDelay = bootDelay,logger=logger)
    
    # start the build-VM
    buildVM.start()
    time.sleep(bootDelay)
    
    if buildVM.isonline():
        returns=[]        
        
        # Partitionate the VDI-File
        returns.append(buildVM.sshcmd('parted /dev/sda -s mklabel msdos'))
        returns.append(buildVM.sshcmd('parted /dev/sda -s mkpart primary 2048s 100%'))
        returns.append(buildVM.sshcmd('parted /dev/sda -s set 1 boot on'))
        returns.append(buildVM.sshcmd('mkfs.ext4 /dev/sda1'))
        returns.append(buildVM.sshcmd('mkdir /target'))
        returns.append(buildVM.sshcmd('mount /dev/sda1 /target'))
        # Bootstrap debian jessie
        returns.append(buildVM.sshcmd('grml-debootstrap --force --grub /dev/sda --target /target --password 123 --hostname PM --release jessie --mirror '+deb_mirror+' | tee /grml-debootstrap.log && sleep 3 && service ssh start'))
        # setup /etc/apt/sources.list        
        returns.append(buildVM.sshcmd('echo "deb http://ftp.de.debian.org/debian/ jessie main non-free contrib" > /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb http://security.debian.org/ jessie/updates main contrib non-free" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb http://ftp.de.debian.org/debian/ jessie-backports main non-free contrib" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt update | tee /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install pkg-mozilla-archive-keyring | tee /apt.log'))
        returns.append(buildVM.sshcmd('echo "deb http://mozilla.debian.net/ jessie-backports firefox-release" >> /target/etc/apt/sources.list'))
        returns.append(buildVM.sshcmd('echo "deb http://mozilla.debian.net/ jessie-backports firefox-esr" >> /target/etc/apt/sources.list'))
        # Use VirtualBox from oracle instead of debian because of incompatibility of guest-additions clipboard sharing
        #returns.append(buildVM.sshcmd('echo "deb http://download.virtualbox.org/virtualbox/debian jessie contrib" >> /target/etc/apt/sources.list'))
        #returns.append(buildVM.sshcmd('wget -q https://www.virtualbox.org/download/oracle_vbox.asc -O- | sudo apt-key add -'))
        
        # update jessie       
        returns.append(buildVM.sshcmd('grml-chroot /target apt update | tee /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt full-upgrade -y | tee -a /apt.log'))
        
        # install openbox
        returns.append(buildVM.sshcmd('echo "#!/bin/bash" > /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('echo "export DEBIAN_FRONTEND=noninteractive" >> /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('echo "apt install -y openbox xinit xterm" >> /target/tmp/openbox_install.sh'))
        if developer_mode:
            returns.append(buildVM.sshcmd('echo "apt install -y menu" >> /target/tmp/openbox_install.sh'))
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/bash /tmp/openbox_install.sh | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('rm /target/tmp/openbox_install.sh'))
        
        # install firefox
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y firefox | tee -a /apt.log'))
        
        # install htop
        if developer_mode:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y htop  | tee -a /apt.log'))
        
        # install zsh
        if developer_mode:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y zsh  | tee -a /apt.log'))
        
        #install vim
        if developer_mode:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y vim  | tee -a /apt.log'))

        # install audio: install needed packages
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y alsa-base  | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y gstreamer1.0-alsa  | tee -a /apt.log'))
        if developer_mode:
            returns.append(buildVM.sshcmd('grml-chroot /target apt install -y alsa-utils  | tee -a /apt.log'))
            
        # install virtualbox-guest additions
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -y dkms  | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -t jessie-backports -y virtualbox-guest-dkms virtualbox-guest-utils | tee -a /apt.log'))
        
        # purge unnecessary packages
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get purge -y lvm2 cryptsetup mdadm | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get autoremove --purge -y | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get autoclean | tee -a /apt.log'))
        returns.append(buildVM.sshcmd('grml-chroot /target apt-get clean | tee -a /apt.log'))
        
        # Download font packages
        returns.append(buildVM.sshcmd('grml-chroot /target apt install -d -y aglfn culmus culmus-fancy fonts-aoyagi-kouzan-t fonts-aoyagi-soseki fonts-arabeyes fonts-arphic-bkai00mp fonts-arphic-bsmi00lp fonts-arphic-gbsn00lp fonts-arphic-gkai00mp fonts-arphic-ukai fonts-arphic-uming fonts-averia-gwf fonts-averia-sans-gwf fonts-averia-serif-gwf fonts-baekmuk fonts-beteckna fonts-bpg-georgian fonts-breip fonts-cabin fonts-cabinsketch fonts-cantarell fonts-century-catalogue fonts-circos-symbols fonts-cmu fonts-cns11643-kai fonts-cns11643-sung fonts-comfortaa fonts-croscore fonts-crosextra-caladea fonts-crosextra-carlito fonts-dancingscript fonts-dejavu fonts-dejavu-core fonts-dejavu-extra fonts-dejima-mincho fonts-dkg-handwriting fonts-dosis fonts-droid fonts-dustin fonts-dzongkha fonts-ebgaramond fonts-ebgaramond-extra fonts-ecolier-court fonts-ecolier-lignes-court fonts-eeyek fonts-evertype-conakry fonts-f500 fonts-fanwood fonts-farsiweb fonts-font-awesome fonts-freefarsi fonts-freefont-otf fonts-freefont-ttf fonts-gargi fonts-gfs-artemisia fonts-gfs-baskerville fonts-gfs-bodoni-classic fonts-gfs-complutum fonts-gfs-didot fonts-gfs-didot-classic fonts-gfs-gazis fonts-gfs-neohellenic fonts-gfs-olga fonts-gfs-porson fonts-gfs-solomos fonts-gfs-theokritos fonts-gubbi fonts-hanazono fonts-horai-umefont fonts-hosny-amiri fonts-hosny-thabit fonts-inconsolata fonts-ipaexfont fonts-ipaexfont-gothic fonts-ipaexfont-mincho fonts-ipafont fonts-ipafont-gothic fonts-ipafont-mincho fonts-ipafont-nonfree-jisx0208 fonts-ipafont-nonfree-uigothic fonts-ipamj-mincho fonts-isabella fonts-johnsmith-induni fonts-jsmath fonts-junction fonts-junicode fonts-jura fonts-kacst fonts-kacst-one fonts-kalapi fonts-kanjistrokeorders fonts-kaushanscript fonts-khmeros fonts-kiloji fonts-klaudia-berenika fonts-komatuna fonts-konatu fonts-kouzan-mouhitsu fonts-lao fonts-larabie-deco fonts-larabie-straight fonts-larabie-uncommon fonts-lato fonts-levien-museum fonts-levien-typoscript fonts-lg-aboriginal fonts-liberation fonts-lindenhill fonts-linex fonts-linuxlibertine fonts-lklug-sinhala fonts-lobster fonts-lobstertwo fonts-lohit-beng-assamese fonts-lohit-beng-bengali fonts-lohit-deva fonts-lohit-gujr fonts-lohit-guru fonts-lohit-knda fonts-lohit-mlym fonts-lohit-orya fonts-lohit-taml fonts-lohit-taml-classical fonts-lohit-telu fonts-lyx fonts-manchufont fonts-mathjax fonts-mathjax-extras fonts-meera-taml fonts-mgopen fonts-migmix fonts-mikachan fonts-misaki fonts-mmcedar fonts-moe-standard-kai fonts-moe-standard-song fonts-mona fonts-monapo fonts-motoya-l-cedar fonts-motoya-l-maruberi fonts-mph-2b-damase fonts-mplus fonts-nafees fonts-nakula fonts-nanum fonts-nanum-coding fonts-nanum-eco fonts-nanum-extra fonts-navilu fonts-noto fonts-ocr-a fonts-oflb-asana-math fonts-oflb-euterpe fonts-okolaks fonts-oldstandard fonts-opendin fonts-opendyslexic fonts-opensymbol fonts-pagul fonts-paktype fonts-pecita fonts-play fonts-prociono fonts-quattrocento fonts-roboto fonts-rufscript fonts-sahadeva fonts-samyak fonts-samyak-deva fonts-samyak-gujr fonts-samyak-mlym fonts-samyak-orya fonts-samyak-taml fonts-sarai fonts-sawarabi-gothic fonts-sawarabi-mincho fonts-senamirmir-washra fonts-sil-abyssinica fonts-sil-andika fonts-sil-charis fonts-sil-dai-banna fonts-sil-doulos fonts-sil-ezra fonts-sil-galatia fonts-sil-gentium fonts-sil-gentium-basic fonts-sil-nuosusil fonts-sil-padauk fonts-sil-scheherazade fonts-sil-sophia-nubian fonts-sil-zaghawa-beria fonts-sipa-arundina fonts-smc fonts-stix fonts-takao fonts-takao-gothic fonts-takao-mincho fonts-thai-tlwg fonts-tibetan-machine fonts-tlwg-garuda fonts-tlwg-kinnari fonts-tlwg-loma fonts-tlwg-mono fonts-tlwg-norasi fonts-tlwg-purisa fonts-tlwg-sawasdee fonts-tlwg-typewriter fonts-tlwg-typist fonts-tlwg-typo fonts-tlwg-umpush fonts-tlwg-waree fonts-tomsontalks fonts-tuffy fonts-ubuntu-title fonts-ukij-uyghur fonts-umeplus fonts-unfonts-core fonts-unfonts-extra fonts-unikurdweb fonts-uralic fonts-vlgothic fonts-vollkorn fonts-woowa-hanna fonts-wqy-microhei fonts-wqy-zenhei fonts-yanone-kaffeesatz fonts-yozvox-yozfont fonts-yozvox-yozfont-antique fonts-yozvox-yozfont-cute fonts-yozvox-yozfont-edu fonts-yozvox-yozfont-new-kana fonts-yozvox-yozfont-standard-kana hershey-fonts-data t1-cyrillic t1-oldslavic t1-teams toilet-fonts ttf-adf-accanthis ttf-adf-baskervald ttf-adf-berenis ttf-adf-gillius ttf-adf-ikarius ttf-adf-irianis ttf-adf-libris ttf-adf-mekanus ttf-adf-oldania ttf-adf-romande ttf-adf-switzera ttf-adf-tribun ttf-adf-universalis ttf-adf-verana ttf-aenigma ttf-alee ttf-ancient-fonts ttf-anonymous-pro ttf-atarismall ttf-bitstream-vera ttf-dejavu ttf-dejavu-core ttf-dejavu-extra ttf-denemo ttf-engadget ttf-essays1743 ttf-femkeklaver ttf-fifthhorseman-dkg-handwriting ttf-freefarsi ttf-georgewilliams ttf-goudybookletter ttf-isabella ttf-jsmath ttf-kochi-gothic ttf-kochi-mincho ttf-marvosym ttf-mona ttf-radisnoir ttf-sjfonts ttf-staypuft ttf-summersby ttf-tagbanwa ttf-tiresias ttf-unifont ttf-wqy-zenhei ttf-xfree86-nonfree ttf-xfree86-nonfree-syriac tv-fonts xfonts-100dpi xfonts-100dpi-transcoded xfonts-75dpi xfonts-75dpi-transcoded xfonts-a12k12 xfonts-ayu xfonts-baekmuk xfonts-base xfonts-bitmap-mule xfonts-biznet-100dpi xfonts-biznet-75dpi xfonts-biznet-base xfonts-bolkhov-75dpi xfonts-bolkhov-cp1251-75dpi xfonts-bolkhov-cp1251-misc xfonts-bolkhov-isocyr-75dpi xfonts-bolkhov-isocyr-misc xfonts-bolkhov-koi8r-75dpi xfonts-bolkhov-koi8r-misc xfonts-bolkhov-koi8u-75dpi xfonts-bolkhov-koi8u-misc xfonts-bolkhov-misc xfonts-cronyx-100dpi xfonts-cronyx-75dpi xfonts-cronyx-cp1251-100dpi xfonts-cronyx-cp1251-75dpi xfonts-cronyx-cp1251-misc xfonts-cronyx-isocyr-100dpi xfonts-cronyx-isocyr-75dpi xfonts-cronyx-isocyr-misc xfonts-cronyx-koi8r-100dpi xfonts-cronyx-koi8r-75dpi xfonts-cronyx-koi8r-misc xfonts-cronyx-koi8u-100dpi xfonts-cronyx-koi8u-75dpi xfonts-cronyx-koi8u-misc xfonts-cronyx-misc xfonts-cyrillic xfonts-efont-unicode xfonts-efont-unicode-ib xfonts-encodings xfonts-intl-arabic xfonts-intl-asian xfonts-intl-chinese xfonts-intl-chinese-big xfonts-intl-european xfonts-intl-japanese xfonts-intl-japanese-big xfonts-intl-phonetic xfonts-jisx0213 xfonts-jmk xfonts-kaname xfonts-kapl xfonts-kappa20 xfonts-marumoji xfonts-mona xfonts-mplus xfonts-naga10 xfonts-nexus xfonts-scalable xfonts-shinonome xfonts-terminus xfonts-terminus-dos xfonts-terminus-oblique xfonts-thai xfonts-thai-etl xfonts-thai-manop xfonts-thai-nectec xfonts-thai-poonlap xfonts-thai-vor xfonts-unifont xfonts-wqy xfonts-x3270-misc | tee -a /apt.log'))
        
        # install all locales
        returns.append(buildVM.sshcmd('grml-chroot /target apt install locales-all | tee -a /apt.log'))
        
        # create timezones.txt which includes all chooseable timezones
        returns.append(buildVM.sshcmd('grml-chroot /target timedatectl list-timezones > /timezones.txt'))
        
        # create locales.txt which includes all chooseable locales
        returns.append(buildVM.sshcmd('grml-chroot /target localectl list-locales > /locales.txt'))
        
        # setup liveuser
        returns.append(buildVM.sshcmd('echo "#!/bin/bash" > /target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('echo "/usr/sbin/useradd -m liveuser" >>/target/tmp/add_liveuser.sh'))
        # install audio: add user to the audio group
        returns.append(buildVM.sshcmd('echo "/usr/sbin/addgroup liveuser audio" >>/target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('echo "echo liveuser:123 | chpasswd"  >>/target/tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('grml-chroot /target /bin/bash /tmp/add_liveuser.sh'))
        returns.append(buildVM.sshcmd('rm /target/tmp/add_liveuser.sh'))



        
        #######################################################################
        ## copy PM required scripts and files to new base-disk
        #######################################################################
            # scripts to interact with PM
        returns.append(buildVM.copyFiles2VM(pm_files_dir+'/pm','/target'))
            # copy var recursive
        returns.append(buildVM.copyFiles2VM(pm_files_dir+'/var','/target'))
            # containing: 
            #   enable sound volume: /var/lib/alsa/asound.state
            
            # copy home recursive
        returns.append(buildVM.copyFiles2VM(pm_files_dir+'/home','/target'))
            # containing: 
            #   Start vbox-service needed for clipboard-sharing: /home/liveuser/.config/openbox/autostart.sh
            
            # copy etc recursive
        returns.append(buildVM.copyFiles2VM(pm_files_dir+'/etc','/target'))
            # containing: 
            #   SSH config: /etc/ssh/sshd_config
            #   allow anybody to start  X-server:  /etc/X11/Xwrapper.config
            #   remove icons: iconify, close in title bar: /etc/X11/openbox/rc.xml
            #   sytemd service to start firefox: /etc/systemd/system/firefox.service
            #   sytemd service to start openbox at systemstart: /etc/systemd/system/openbox.service
            #   automatically start openbox at systemstart
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
        returns.append(buildVM.sshcmd('grml-chroot /target update-initramfs -u -k all'))
            # disable grub boot menue
        returns.append(buildVM.copyFiles2VM(pm_files_dir+'/etc/default/grub', '/target/etc/default/grub'))
        returns.append(buildVM.sshcmd('grml-chroot /target update-grub'))
        

        # copy the log files and stop the VM    
        buildVM.copyFiles2Host('/apt.log',vdi_target_dir+'/'+time_str+'__base-disk.vdi.apt_log')
        buildVM.copyFiles2Host('/timezones.txt',vdi_target_dir+'/'+time_str+'__timezones.txt')
        buildVM.copyFiles2Host('/locales.txt',vdi_target_dir+'/'+time_str+'__locales.txt')
        buildVM.copyFiles2Host('/grml-debootstrap.log',vdi_target_dir+'/'+time_str+'__base-disk.vdi.debootstrap_log')
        buildVM.stop()
        
    # check if anything went wrong
    if all(map(lambda x:x==0,returns)):
        logger.info('>>>>>>>>>>>>>   SUCCESS: installing debian   >>>>>>>>>>>>>>')
        logger.info('cleanup:')
        cmd_exec('VBoxManage',['closemedium', vdi_target_dir+'/'+time_str+'__base-disk.vdi'],logger)
        cmd_exec('mv',[vdi_target_dir+'/BAD__'+time_str+'__base-disk.vdi',vdi_target_dir+'/'+time_str+'__base-disk.vdi'],logger)
        cmd_exec('VBoxManage',['unregistervm', buildVM_name,'--delete'],logger)

        
    else:
        logger.fatal('A error occured at firering up commands in the VM')
        logger.fatal('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
        logger.fatal('!!!   please debug and remove the VM '+buildVM_name+' on your own   !!!')
        logger.fatal('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!')
