#!/usr/bin/python
from __future__ import absolute_import, division, print_function, unicode_literals

import os
import sys
import random
import shutil


def rm_r(path):
    if os.path.isdir(path) and not os.path.islink(path):
        shutil.rmtree(path)
    elif os.path.exists(path):
        os.remove(path)



def extract_folder(path,flag=".addon"):
    cont_list = os.listdir(path)
    subfolder_list=[] #list of subfolders
    addons=[] #list of addons
    for inode in cont_list:
        inode_path = path+"/"+inode
        if os.path.isdir(inode_path):
            subfolder_list.append(inode_path)
        else:
            if flag in inode:
                addons.append((inode_path,1))
    return addons, subfolder_list
    
        
def weighted_choice(choices):
    if len(choices)==0:
        return ('',0)
    total = sum(w for c, w in choices)
    r = random.uniform(0, total)
    upto = 0
    for c, w in choices:
       if upto + w >= r:
          return c, total
       upto += w
    assert False, "Shouldn't get here"
    
    
def selectAddons(path,flag=".addon"):
    addons, subfolder = extract_folder(path,flag)
    alladdons = {addon_path for addon_path,wight in addons}
    for folder in subfolder:
        f_addon_choosen,f_addons,f_alladdons = selectAddons(folder,flag)
        addons.append(f_addon_choosen)
        alladdons = alladdons.union(f_alladdons)

    return weighted_choice(addons),addons,alladdons
    
def cleanPath(path):
    if path[-1] == '/':
        path = path[0:-1]
    return path


if __name__ == '__main__':    
   
    if len(sys.argv) < 3 or any(map(lambda s: \
    not any([type(s)==str, type(s)==unicode, type(s)==buffer]) or \
    not os.path.isdir(s), sys.argv[1:3])):
        print("Usage: "+sys.argv[0]+" /path/to/firefox/extensions " + \
        "path/to/addonlists  [Number of addons]" )
        exit(1)
    path_to_extension_dir = cleanPath(sys.argv[1])
    addon_dir = cleanPath(sys.argv[2]) 
    
    firstaddon, addonlist,alladdons = selectAddons(addon_dir,".addon")
    if len(sys.argv)==4 and sys.argv[3]<len(addonlist) and sys.argv[3]>=0:
        number_of_addons_to_select= sys.argv[3]
    else:
        number_of_addons_to_select = random.randint(0,len(addonlist))

    print('choosing '+str(number_of_addons_to_select)+' random addons')
    
    trans_table=dict()
    while len(alladdons) > 0:
        addon_def_file = alladdons.pop()
        with open(addon_def_file) as f:        
            cont_def_file = map(lambda s: s.strip(), f.readlines())           
        trans_table[addon_def_file]=tuple(cont_def_file)
    del alladdons 
    
    alladdons = set()
    for i in trans_table.values():
        alladdons |= set(i)

    selected_addons=set(trans_table[firstaddon[0]])
    while len(selected_addons) < number_of_addons_to_select:
        addon,foo = weighted_choice(addonlist)
        selected_addons |= set(trans_table[addon])
    del addonlist,firstaddon
    
     
    addons_to_delete = alladdons - selected_addons
    
    while len(addons_to_delete) > 0:
        tmp = path_to_extension_dir+'/'+addons_to_delete.pop()
        rm_r(tmp)
