#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov  8 13:46:40 2016

@author: olaf
"""

import json
import os
import subprocess as sp


if __name__ == '__main__':

    # the base directory for the build of BaseDisks, there at ablolute minimum 10GB of space should be avaiable (recommended min 50GB)
    base_dir='/opt/pm/BaseDisk'
    base_dir='/data/storage/BaseDisk_build'

    # here we check if the base direktory exists
    if not os.path.isdir(base_dir):
        print('BaseDisk build location "'+base_dir+'" is not existing or not a directory!')
        exit(1)



    config={ \
                   'COMMENT_01':'This file is generated once with the script "generateInitialConfig.py"', \
                   'COMMENT_02':'All fields are also documented in this script', \
                   # BaseDisk version numbers
                   'actual_version': \
                   { \
                        # Mainly for marketing purpose (Beta: start with 0)
                       'Major':0, \
                        # changes on incompatible new Features (API to comunicate with PrivacyMachine changes)
                       'Minor':10, \
                        # changes after build basedisk was necessary (will be incremented at the start of each build)
                       'ComponentMajor':0, \
                        # not used (always 0)
                       'ComponentMinor':0
                   }, \
                    # Filesystem path's where build related stuff will be stored
                   'base_paths': \
                   { \
                      # In 'delta_dir' the incremental BaseDisk updates are stored
                       'delta_dir':base_dir+'/BaseDisk_delta', \
                      # In 'signature_dir' the rdiff signatures are stored. They are mandatory to build a incremental update = delta file
                       'signature_dir':base_dir+'/BaseDisk_signature', \
                      # In 'image_dir' the builded BaseDisks are stored
                       'image_dir':base_dir+'/BaseDisk_image', \
                      # In 'log_dir' the log files for each build are stored in a seperate folder
                       'log_dir':base_dir+'/BaseDisk_log', \
                      # Location and name of a lockfile to prevent simultaneous builds
                       'lock_file':'/tmp/buildBaseDisk.lock' \
                   }, \
                   # to get a working BaseDisk we also need a VMDK-description which is based on a template
                   'VMDK_template': \
                   { \
                       # Path of the VMDK-description-template
                       'path':base_dir+'/template-description.vmdk', \
                       # A magic word which will be replaced with the BaseDisk name
                       'magic_word':'__magic_word__'
                   }, \
                    # Configuration which is used by build_BaseDisk.py
                   'build_config': \
                   { \
                        # Virtual machine related parameters (a working libvirt environment is required)
                       'VM': \
                       { \
                            # Path to the virtual harddisk (VMDK) which is configured in the VM definition XML's
                           'disk_path':base_dir+'/empty-flat.vmdk', \
                            # Size of the virtual harddisk in MB, is used to overwrite the harddisk with zeros
                           'disk_size':4096, \
                            # Name of the VM configured in the VM definition XML's
                           'VM_name':'vm_BaseDisk_build', \
                            # Startup time to prevent sending commands to the VM while starting or stopping
                           'boot_delay':90
                       }, \
                       # Paths to the neccesary files for a BaseDisk build
                       'paths': \
                       { \
                           ## The unencrypted SSH private key are needed to communicate with the VM and the live GRML
                           'grml_ssh_priv_key':base_dir+'/grmlWithSshKey/grml_iso__id_rsa', \
                           # Folder containing additional files stored in the BaseDisk image, i.e. fonts and systemd scripts to start the browsers
                           'pm_dir':base_dir+'/pm_files'
                       }, \
                       # apt-cacher-ng is recommended but not required
                       'apt': \
                       { \
                            # All examples are using the debian mirror 'http://ftp.de.debian.org/debian'
                            # * Example without cache:
                           'deb_proxie':'http://', \
                           'deb_mirror':'ftp.de.debian.org/debian'
                            # * Example for a local machine named 'cacher' with installed apt-cacher-ng: 'http://cacher:3142/ftp.de.debian.org/debian
                            #   Take care of the tailing '/' at the value of 'deb_proxie'
                            # 'deb_proxie':'http://cacher:1234/', \
                            # 'deb_mirror':'ftp.de.debian.org/debian'
                       }, \
                       # Enable Debug-Build to install additional software on BaseDisk like htop (see libBaseDiskBuild.py)
                       'debug_build':True, \
                       # the local top level domain (i.e. ".lan") for finding the build machine on the local network
                       'local_tld':''
                   }, \
                    # settings to upload builded BaseDisks to a webserver
                   'delivery_server': \
                   { \
                        # the ssh password to login to the webserver
                       'ssh_pw':'xxxxxx', \
                        # SSH identifier i.e. user@server.tld
                       'ssh_host':'user@webserver.com', \
                        # SSH options
                       'ssh_options':['-q','-o', 'PreferredAuthentications=password', '-o', 'PubkeyAuthentication=no'], \
                        # The webservers base directory for delivering the BaseDisks
                       'ssh_home':'/var/www/BaseDisk'
                   }, \
                    # enable the GPG signing process of BaseDisks
                   'gpg_sign':False, \
                    # GPG-ID
                   'signing_key':'XXXXXXXX', \
                    # Number of old BaseDisks for which incremental updates will be created -> User can update from any of the last i.e. 5 BaseDisk-Releases
                   'max_BaseDisks':5, \
                    # enable upload to delivery_server configured above
                    # also older base disks and logs will be deleted
                   'upload_zips':False, \
                    # Do not touch: Inital empty Array for the metadata of already builded BaseDisks
                   'BaseDisks':[]}



    answer = input('Check configuration and create inital paths? [Y|n]  ')
    if not answer.lower().startswith('n'):
        def checkDir(disp,path):
            if os.path.isdir(path):
                print('GOOD','         ','{:40}'.format(disp),path)
                return True
            else:
                print('NOT EXISTIG','  ','{:40}'.format(disp),path)
                answer = input('Create '+path+'? [Y|n]  ')
                if not answer.lower().startswith('n'):
                    os.makedirs(path)
                print()
                return False
        def checkFile(disp,path):
            if os.path.isfile(path):
                print('GOOD','         ','{:40}'.format(disp),path)
                return True
            else:
                print('NOT EXISTIG','  ','{:40}'.format(disp),path)
                return False
        def genVmdkTemplate(magic_word, size):
            res = '# Disk DescriptorFile'
            res += '\n' + 'version=1'
            res += '\n' + 'CID=a31364e4'
            res += '\n' + 'parentCID=ffffffff'
            res += '\n' + 'createType="monolithicFlat"'
            res += '\n'
            res += '\n' + '# Extent description'
            # calculate disk size in sectors (512B); MB*1024*1024/512
            res += '\n' + 'RW '+str(size*2048)+' FLAT "'+magic_word+'" 0'
            res += '\n' 
            res += '\n' + '# The disk Data Base '
            res += '\n' + '#DDB'
            res += '\n' 
            res += '\n' + 'ddb.virtualHWVersion = "4"'
            res += '\n' + 'ddb.adapterType="ide"'
            res += '\n' + 'ddb.uuid.image="0ac66e6e-2a04-4d75-9357-c5c3bc87dcf2"'
            res += '\n' + 'ddb.uuid.parent="00000000-0000-0000-0000-000000000000"'
            res += '\n' + 'ddb.uuid.modification="00000000-0000-0000-0000-000000000000"'
            res += '\n' + 'ddb.uuid.parentmodification="00000000-0000-0000-0000-000000000000"'         
            return res
            
            
        print('\nChecking if configured paths are present\n')

        checkDir('base_paths->delta_dir',config['base_paths']['delta_dir'])
        checkDir('base_paths->signature_dir',config['base_paths']['signature_dir'])
        checkDir('base_paths->image_dir',config['base_paths']['image_dir'])
        checkDir('base_paths->log_dir',config['base_paths']['log_dir'])
        checkDir('build_config->paths->pm_dir',config['build_config']['paths']['pm_dir'])
        
        print()
        if not checkFile('build_config->paths->grml_ssh_priv_key',config['build_config']['paths']['grml_ssh_priv_key']):
            print('It seems no grml with ssh-key is avaiable. Please download http://unstable.privacymachine.eu/BaseDisk_build/grmlWithSshKey/grmlWithSshKey.tar and extract it to '+ os.path.split(config['build_config']['paths']['grml_ssh_priv_key'])[0])
            

        print()
        if not checkFile('build_config->VM->disk_path',config['build_config']['VM']['disk_path']):
            answer = input('Create empty VM disk? [Y|n]  ')
            if not answer.lower().startswith('n'):
                try:
                    sp.check_call(['dd','if=/dev/zero','bs=1M','count='+str(config['build_config']['VM']['disk_size']),'of='+config['build_config']['VM']['disk_path']])
                except sp.CalledProcessError as err:
                    print('Execution of dd failed, make shure there is enought space.\n cmd: '+str(err.cmd)+'\n output: '+str(err.output)+'\n retunrcode: '+ str(err.returncode))
        print()            
        if not checkFile('VMDK_template->path',config['VMDK_template']['path']):
            answer = input('Create VMDK_template? [Y|n]  ')
            if not answer.lower().startswith('n'):
                with open(config['VMDK_template']['path'],'w') as f:
                    f.write(genVmdkTemplate(config['VMDK_template']['magic_word'],config['build_config']['VM']['disk_size']))


    # Do not touch: Name of generated JSON-Config file, used in buildNewBaseDisk.py
    filename = 'buildBaseDiskConfig.json'

    # write json-config in base directory
    with open(base_dir+'/'+filename,'w') as f:
        json.dump(config, f)

    # and create a backup with tailing '.orig_' + increasing number
    i = 1
    while True:
        if not os.path.exists(base_dir+'/'+filename+'.orig_'+str(i)):
            with open(base_dir+'/'+filename+'.orig_'+str(i),'w') as f:
                json.dump(config, f)
            break
        i += 1


    print()
    print('successfully created configuration file ' + base_dir+'/'+filename + ' and backup configuration file ' + base_dir+'/'+filename+'.orig_'+str(i))
    print()
