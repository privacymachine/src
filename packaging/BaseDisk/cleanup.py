#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
Created on Thu Apr  6 23:44:34 2017

@author: olaf
"""
import json
import os
import shutil

if __name__ == '__main__':

    config_path="buildBaseDiskConfig.json"
    
    try:        
        with open(config_path,'r') as f:
            config = json.load(f) 
    except:
        print()
        print('Could not read or decode '+config_path)        
        print()
        print('Are you in the right directory?')
        print()
        exit(1)        
                
        
    if os.path.exists(config['base_paths']['lock_file']):
        print('It seems prepare_old_base_disk_update.py is running \n lockfile: '+config['base_paths']['lock_file'])
        print('Only continue if you are sure that prepare_old_base_disk_update.py is not running!')

        answer = input('Continue? [y|N]  ')
        if not answer.lower().startswith('y'):
            exit(0)
        os.remove(config['base_paths']['lock_file'])
        print()
        
        
    print('This Operation will delete all already builded BaseDisks!')
    answer = input('Continue? [y|N]  ')
    if not answer.lower().startswith('y'):
        exit(0)
    
    def removeAndRecreate(dir_path):
        if os.path.isdir(dir_path):
            shutil.rmtree(dir_path)
            os.mkdir(dir_path)

    removeAndRecreate(config['base_paths']['delta_dir'])
    removeAndRecreate(config['base_paths']['signature_dir'])    
    removeAndRecreate(config['base_paths']['image_dir'])
    removeAndRecreate(config['base_paths']['log_dir'])
    
    config['BaseDisks']=[]
    
    with open(config_path,'w') as f:
        json.dump(config, f)