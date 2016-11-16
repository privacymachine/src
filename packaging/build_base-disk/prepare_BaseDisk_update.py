# -*- coding: utf-8 -*-
"""
Created on Mon Oct 10 16:48:49 2016

@author: olaf
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import generators
from __future__ import nested_scopes
from __future__ import print_function
from __future__ import unicode_literals
from __future__ import with_statement

from build_BaseDisk import *

import logging
import subprocess as sp
import shutil
import os
import json
import collections
import sys

def flatten(l):
    for el in l:
        if isinstance(el, collections.Iterable) and not isinstance(el, (str, bytes)):
            yield from flatten(el)
        else:
            yield el

def copyFile2DeliverServer(source, target, ssh_data,logger):
    file_name = os.path.split(source)[-1]
    logger.info('calculate local md5sum')
    try:
        md5sum_local = sp.check_output(['md5sum',source], stderr=sp.STDOUT).decode("utf-8").split(' ')[0]
    
    except sp.CalledProcessError as err:
        logger.fatal('cold not calculate md5sum from '+source+ \
                        '\n cmd: '+str(err.cmd)+'\n output: '+ \
                        str(err.output) + '\n returncode: ' +\
                        str(err.returncode))
        return err.returncode,err
                        
    ssh_identify_str = ssh_data['ssh_host']+':'+ssh_data['ssh_home']+'/'+target
    logger.info('copy '+source +' to '+ ssh_identify_str )
    cmd = ['sshpass','-p', ssh_data['ssh_pw'],
           'scp',ssh_data['ssh_options'], source, ssh_identify_str]
    cmd = list(flatten(cmd))
    try:
        sp.check_output(cmd, stderr=sp.STDOUT).decode("utf-8")

    except sp.CalledProcessError as err:
        logger.fatal('cold not copy '+source+' to '+ssh_identify_str+ \
                        '\n cmd: '+str(err.cmd)+'\n output: '+ \
                        str(err.output) + '\n returncode: ' +\
                        str(err.returncode))
        return err.returncode,err
        
    logger.info('calculate remote md5sum')
    cmd = ['sshpass','-p', ssh_data['ssh_pw'],
           'ssh',ssh_data['ssh_options'], ssh_data['ssh_host'], 
           'md5sum', ssh_data['ssh_home']+'/'+target+'/'+file_name]
    cmd = list(flatten(cmd))
    try:
        md5sum_remote = sp.check_output(cmd, stderr=sp.STDOUT).decode("utf-8").split(' ')[0]     
    except sp.CalledProcessError as err:
        logger.fatal('cold not calculate md5sum of '+ \
                        ssh_data['ssh_host']+':'+ssh_data['ssh_home']+'/'+target+'/'+file_name +\
                        '\n cmd: '+str(err.cmd)+'\n output: '+ \
                        str(err.output) + '\n returncode: ' +\
                        str(err.returncode))
        return err.returncode,err
    
    logger.info('verify integrity')
    if md5sum_local != md5sum_remote:
        error_str= 'local and remote file differ\n'+ \
            md5sum_local+'     '+'md5sum_local\n'+md5sum_remote+'     '+'md5sum_remote'
        logger.error(error_str)
        return 1, error_str
        
    # set right permissions        
    logger.info('set remote permissions')
    cmd = ['sshpass','-p', ssh_data['ssh_pw'],
           'ssh',ssh_data['ssh_options'], ssh_data['ssh_host'], 
           'chmod', '644', ssh_data['ssh_home']+'/'+target+'/'+file_name]
    cmd = list(flatten(cmd))
    try:
        sp.check_output(cmd, stderr=sp.STDOUT)    
    except sp.CalledProcessError as err:
        logger.fatal('cold not set permissions to 444 for '+ \
                        ssh_data['ssh_host']+':'+ssh_data['ssh_home']+'/'+target+'/'+file_name +\
                        '\n cmd: '+str(err.cmd)+'\n output: '+ \
                        str(err.output) + '\n returncode: ' + \
                        str(err.returncode))
        return err.returncode,err
        
    logger.info('successfully copied file')    
    return 0,''



def hash_and_sign(folder,file_prefix,keyID,logger,additional_files=[]):
    logger.info('sign files in folder ' + folder)
    logger.info('hash files')
    os.chdir(folder)
    _, hashs = cmd_exec('sha256sum',os.listdir(folder),logger,True)
    for f in additional_files:
        logger.info('hash additional file '+f)
        f_path,f_name=os.path.split(f)
        os.chdir(f_path)
        try:
            h = sp.check_output(['sha256sum',f_name], stderr=sp.STDOUT).decode("utf-8") 
        except sp.CalledProcessError as err:
            logger.fatal('could not hash file '+str(f) + \
                        '\n cmd: '+str(err.cmd)+'\n output: '+ \
                        str(err.output) + '\n returncode: ' + \
                        str(err.returncode))
            return err.returncode
        hashs += h
                
    hash_file = folder+'/'+file_prefix+'_sha256sums.txt'
    with open(hash_file,'w') as f:
        f.write(hashs)
    logger.info('sign hash file')
    return cmd_exec('gpg',['-a','-u',keyID,'--output',hash_file+'.asc','--detach-sig',hash_file],logger)    
    

if __name__ == '__main__':

    if len(sys.argv) == 2:
        print()
        print('Using '+ sys.argv[1]+' as configuration file')
        config_path = sys.argv[1]
    elif len(sys.argv) == 1:
        config_path = os.path.split(sys.argv[0])[0]
        config_path += '/prepare_BaseDisk_update.config.json'
        print('No configuration file givn.')
        print('Assuming '+config_path+' as configuration file.')
    else:
        print()
        print('Too many arguments!')
        print()
        print('Usage:')
        print(os.path.split(sys.argv[0])[1]+'   [configuration file]')
        print()
        print('The configuration file must be json encoded.')
        print('Use gen_prep_bd_update_conf.py to create it.')
        print()
        exit(1)        
    try:        
        with open(config_path,'r') as f:
            config = json.load(f) 
    except:
        print()
        print('Could not read or decode '+config_path)        
        print()
        print('Usage:')
        print(os.path.split(sys.argv[0])[1]+'   [configuration file]')
        print()
        print('The configuration file must be json encoded.')
        print('Use gen_prep_bd_update_conf.py to create it.')
        print()
        exit(1)        

# Test for lockfile to prevent paralell execution
    if os.path.exists(config['base_paths']['lock_file']):
        print('prepare_old_base_disk_update.py is already running \n lockfile: '+config['base_paths']['lock_file'])
        exit(0)
    os.mknod(config['base_paths']['lock_file'])
    
  
# Increment ComponentMajor (base-disk's main version number)
    config['actual_version']['ComponentMajor'] += 1  

# Create new base-disk context
    base_disk = {'version':config['actual_version']}

    # create output folder; name based on ComponentMajor version number
    base_disk['image_identifier']='base-disk_'+str(base_disk['version']['ComponentMajor'])
    base_disk['image_dir']=config['base_paths']['image_dir']+'/'+base_disk['image_identifier']    

    if True:    
        print('create the target directory "' + base_disk['image_dir'])   
        try:
            os.mkdir(base_disk['image_dir'])
        except:
            print('FATAL: Could not create target directory!')
            exit(1)

    base_disk['log_dir']=config['base_paths']['log_dir']+'/'+base_disk['image_identifier']
    print('create the log directory "' + base_disk['log_dir'])   
    try:
        os.mkdir(base_disk['log_dir'])
    except:
        print('FATAL: Could not create log directory!')
        exit(1)


    #====================== LOGGING ==========================================#
    # create logger 
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    # create file handler which logs even debug messages
    fh = logging.FileHandler(base_disk['log_dir']+'/build_base-disk.log')
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
    
    logger.info('>>>>>>>>>>> START BUILDING A NEW BASE-DISK >>>>>>>>>>>>>>>>')
    if True:    
        # Build new base-disk
        base_disk.update(build_BaseDisk(config['build_config'],base_disk,logger))
            # this adds:
            # base_disk['flat_vmdk_path']
            # base_disk['capabilities_path']
        
        logger.info('create rdiff signature')
        # Build rdiff signature
        base_disk['signature_path'] = config['base_paths']['signature_dir']+'/'+ \
            base_disk['image_identifier']+'_flat.vmdk.signature' 
        logger.info('try to build signaturefile for '+base_disk['flat_vmdk_path'])
        cmd_exec('rdiff',['signature', base_disk['flat_vmdk_path'],
                          base_disk['signature_path']],logger)
        
        # generate .vmdk file       
        base_disk['vmdk_path'] = base_disk['image_dir'] + '/' +base_disk['image_identifier']+'.vmdk'
        logger.info('create base-disk description file '+ base_disk['vmdk_path'])
        with open(config['VMDK_template']['path'],'r') as f:
            vmdk_template=f.read()
        _,image_name = os.path.split(base_disk['flat_vmdk_path'])
        vmdk_template = vmdk_template.replace(config['VMDK_template']['magic_word'],image_name)
        with open(base_disk['vmdk_path'],'w') as f:
            f.write(vmdk_template)                    
        
        # gen , shasums and PGP-signature 
        hash_and_sign(base_disk['image_dir'],base_disk['image_identifier'],config['signing_key'],logger)
        
        # create zip
#        os.chdir(config['base_paths']['image_dir'])            
#        logger.info('create '+base_disk['image_identifier']+'.7z')
#        cmd_exec('zip',['-r', base_disk['image_identifier']+'.7z' , base_disk['image_identifier']],logger)
#        base_disk['zip_path']= base_disk['image_dir']+'.7z'
        os.chdir(base_disk['image_dir'])
        logger.info('create '+base_disk['image_identifier']+'.7z')
        cmd_exec('7z',list(flatten(['a',base_disk['image_identifier']+'.7z' , os.listdir(base_disk['image_dir'])])),logger)
        cmd_exec('mv',[base_disk['image_dir']+'/'+base_disk['image_identifier']+'.7z', config['base_paths']['image_dir']],logger)
        base_disk['zip_path'] = config['base_paths']['image_dir']+'/'+base_disk['image_identifier']+'.7z'
        
        if config['upload_zips']:
            logger.info('upload '+base_disk['zip_path'])
            copyFile2DeliverServer(base_disk['zip_path'],'',config['delivery_server'],logger)
    
    base_disk['deltas'] = []       
    
    if len(config['base-disks']) > 0:
        # check for old base-diks 
        if len(config['base-disks']) > config['max_base-disks']:
            logger.info('There are '+str(len(config['base-disks']))+ \
                ' base-disks.\nDelete oldest '+str(len(config['base-disks']) \
                - config['max_base-disks'])+' base-disks.')
            
            while len(config['base-disks']) > config['max_base-disks']:
                to_remove = config['base-disks'].pop(0)
                logger.info('delete '+to_remove['image_identifier'])
                cmd_exec('rm',['-rf',to_remove['image_dir']],logger)
                cmd_exec('rm',[to_remove['signature_path']],logger)
                cmd_exec('rm',[to_remove['zip_path']],logger)
                cmd_exec('rm',[to_remove['log_dir']],logger)

                for d in to_remove['deltas']:
                    cmd_exec('rm',['-rf',d['dir_path']],logger)
                    cmd_exec('rm',[d['zip_path']],logger)
                        
        
        logger.info('build deltas now')
        for old_base_disk in  config['base-disks']:
        # build new delta
            delta_info={}
            delta_info['identifier'] = 'base-disk_delta_' + str(old_base_disk['version']['ComponentMajor']) + '-' + str(base_disk['version']['ComponentMajor'])
            delta_info['dir_path'] = config['base_paths']['delta_dir'] + '/'+ delta_info['identifier']
            logger.info('create new delta directory '+delta_info['dir_path'])
            os.mkdir(delta_info['dir_path'])      
            delta_info['delta_file_path'] = delta_info['dir_path']+'/'+delta_info['identifier']+'.flat.rdiff'
            logger.info('create delta file '+delta_info['delta_file_path'])
            # create delta file                   
            cmd_exec('rdiff',['delta', old_base_disk['signature_path'], 
                             base_disk['flat_vmdk_path'], 
                             delta_info['delta_file_path']],logger)
                             
            logger.info('copy files to '+delta_info['dir_path'])
            shutil.copy2(base_disk['capabilities_path'],delta_info['dir_path'])
            shutil.copy2(base_disk['vmdk_path'],delta_info['dir_path'])
            # hash and sign delta dir
            hash_and_sign(delta_info['dir_path'],delta_info['identifier'],config['signing_key'],logger,[base_disk['flat_vmdk_path']])
            #TODO: add hash of basedisk

            # create zip
#            os.chdir(config['base_paths']['delta_dir'])            
#            logger.info('create '+delta_info['identifier']+'.7z')
#            cmd_exec('zip',['-r', delta_info['identifier']+'.7z' , delta_info['identifier']] ,logger)
#            delta_info['zip_path']= delta_info['dir_path']+'.7z'
            os.chdir(delta_info['dir_path'])
            logger.info('create '+delta_info['identifier']+'.7z')
            cmd_exec('7z',list(flatten(['a', delta_info['identifier']+'.7z', os.listdir(delta_info['dir_path'])])),logger)
            cmd_exec('mv',[delta_info['dir_path']+'/'+delta_info['identifier']+'.7z', config['base_paths']['delta_dir']],logger)
            delta_info['zip_path'] = config['base_paths']['delta_dir']+'/'+delta_info['identifier']+'.7z'
            
            if config['upload_zips']:
                logger.info('upload '+delta_info['zip_path'])
                copyFile2DeliverServer(delta_info['zip_path'],'',config['delivery_server'],logger)

            base_disk['deltas'].append(delta_info)
            
    else:
        logger.info('No old base-disks, nothing to do.') 
        
    # write config and exit        
    config['base-disks'].append(base_disk)
    logger.info('write new config')
    with open(config_path,'w') as f:
        json.dump(config, f) 
    os.remove(config['base_paths']['lock_file'])
    exit(0)
