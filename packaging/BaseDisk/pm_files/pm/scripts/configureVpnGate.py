#! /usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Sat Nov 12 22:00:52 2016

@author: olaf
"""


from __future__ import absolute_import
from __future__ import division
from __future__ import generators
from __future__ import nested_scopes
from __future__ import print_function
from __future__ import unicode_literals
from __future__ import with_statement


def ping(host):
    """
    Returns True if host responds to a ping request
    """
    import os, platform

    # Ping parameters as function of OS
    ping_str = "-n 1" if  platform.system().lower()=="windows" else "-c 1"

    # Ping
    return os.system("ping " + ping_str + " " + host) == 0
    

import sys
import base64
import os
import random

if len(sys.argv) != 2:
    print('ERROR: this script requires the path to the vpngate csv gate as parameter')
    exit(1)

with open(sys.argv[1],'r') as f:
    lines = f.readlines()

csv = []
#HostName,IP,Score,Ping,Speed,CountryLong,CountryShort,NumVpnSessions,Uptime,TotalUsers,TotalTraffic,LogType,Operator,Message,OpenVPN_ConfigData_Base64
for i in range(len(lines)):
    if len(lines[i]) >1000:
        csv.append(lines[i].decode('utf-8').split(','))


while len(csv) != 0:
    server=random.sample(csv,1)[0]
    csv.remove(server)
    vpn_config=base64.b64decode(server[-1]).replace('\r','')
    if vpn_config.find('OpenVPN') == -1:
        continue
    ip_loc = vpn_config.find('\nremote')
    ip = vpn_config[ip_loc+8:]
    ip_loc = ip.find('\n')
    ip = ip[:ip_loc]
    ip = ip.split(' ')[0]
    print('testing ip',ip)
    if ping(ip):
        print('success')
        os.mkdir('/home/vmConfig')
        os.mkdir('/home/vmConfig/vpn/')
        # ugly hack
        vpn_config += '\ndhcp-option DNS 37.235.1.174\n'
        with open('/home/vmConfig/vpn/openvpn_pm.ovpn','w') as f:
            f.write(vpn_config)  
        exit(0)
    
print('ERROR: no valid vpn server found!')
exit(1)


