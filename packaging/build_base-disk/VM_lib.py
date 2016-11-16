# -*- coding: utf-8 -*-
"""
Created on Sun May 10 22:32:05 2015

@author: olaf
"""

import subprocess as sp
import time


##  global logging functions
def logCmdError(logger, problemDescription, cmd, output, errorDesc):
   msg = problemDescription + ' \n  cmd: ' + cmd
   output = output.strip()
   if len(output) > 0:
      msg += '\n    output: ' + output
   msg += '\n    returncode: ' + errorDesc
   logger.fatal(msg)


def logCmdSuccess(logger, description, output=''):
   msg = 'SUCCESS: ' + description
   output = output.strip()
   if len(output) > 0:
      msg += '\n    output: ' + output
   logger.info(msg)


# class VM
class VM:
   
    def __init__(self,owner,vmuser,vmname,sshkey,port,url,logger,bootDelay):
        self.owner = owner        
        self.vmuser = vmuser
        self.vmname = vmname
        self.sshcmdkey = sshkey
        self.port = port
        self.logger = logger
        self.url = url
        self.bootDelay = bootDelay
        self.isonline()

            

    #===================== check if vm is online ==============================
        
    def isonline(self):   
        maxTries=3
        self.logger.info('test if VM '+self.vmname+' is online')
        str1 = ''
        for n in range(maxTries):
            try:
                str1 = sp.check_output(['/usr/bin/virsh', 'list'],
                                        stderr=sp.STDOUT).decode("utf-8")        
            except sp.CalledProcessError as err:
                self.logger.fatal('command: "/usr/bin/virsh list" failed \n cmd: '+ \
                                  str(err.cmd)+'\n output: '+str(err.output) + \
                                  '\n returncode: ' +str(err.returncode))                          
            except OSError as err:
                self.logger.fatal('command: "/usr/bin/virsh list" failed: '+err.strerror)
            
            if (str1.find(self.vmname) < 0):
                self.online = False
                logCmdSuccess(self.logger,'"/usr/bin/virsh list": '+self.vmname+' is offline')
                return self.online
                
            logCmdSuccess(self.logger,'"/usr/bin/virsh list": ' +self.vmname+ \
                          ' seems to be online, now test ssh')

            tmp = self.sshcmd('echo test',True)
            if tmp[0]==0:
                logCmdSuccess(self.logger,'VM ' +self.vmname+' is online')
                self.online = True                 
                return self.online                
            
            elif n<maxTries-1:     
                self.logger.info('ssh test failed, will try again after '+str(self.bootDelay)+' secounds')
                time.sleep(self.bootDelay)
                continue
            else:
                self.logger.error('ssh test failed '+str(n)+' times, VM '+self.vmname+' will be destroyed!')
                try:
                    str1 = sp.check_output(['/usr/bin/virsh', 'destroy', self.vmname],
                                            stderr=sp.STDOUT).decode("utf-8")        
                except sp.CalledProcessError as err:
                    self.logger.fatal('command: "/usr/bin/virsh destroy '+ \
                                      self.vmname+'" failed \n cmd: '+ \
                                      str(err.cmd)+'\n output: '+ \
                                      str(err.output) + '\n returncode: ' + \
                                      str(err.returncode))                          
                except OSError as err:
                    self.logger.fatal('command: "/usr/bin/virsh destroy '+self.vmname+'" failed: '+err.strerror)                
                     
                self.online=False
                return self.online                
                
                

        
        
        

    #======================== fire up a command in vm =========================
        
    def sshcmd(self,cmd,returnOutput=False):
        self.logger.info('VM '+self.vmname+': sshcmd: '+cmd)
        params = ['/usr/bin/ssh',
                  '-p', self.port,
                  '-i', self.sshcmdkey,
                  '-o', 'UserKnownHostsFile=/dev/null',
                  '-o', 'StrictHostKeyChecking=no',
                  self.vmuser+'@'+self.url, 
                  cmd]
        try:
            str1 = sp.check_output(params, stderr=sp.STDOUT).decode("utf-8")
        except sp.CalledProcessError as err:
            logCmdError(self.logger,'sshcmd failed', str(err.cmd), str(err.output), str(err.returncode))
            if returnOutput:         
                return err.returncode,err.output
            else:
                return err.returncode
        except OSError as err:
            self.logger.fatal('sshcmd failed: '+err.strerror)
            if returnOutput:         
                return err.errno,err.strerror
            else:
                return err.strerror
        else:
            logCmdSuccess(self.logger,'VM '+self.vmname+': sshcmd: '+cmd, str1)

        if returnOutput:
            return (0,str1)
        else:
            return 0                             


    def start(self):    
        self.logger.info('start the VM '+self.vmname)
        try:
            str1 = sp.check_output(['/usr/bin/virsh','start',
                                    self.vmname],
                                    stderr=sp.STDOUT).decode("utf-8")
        except sp.CalledProcessError as err:
            self.logger.fatal('could not start VM '+self.vmname+'\n cmd: '+ \
                              str(err.cmd)+'\n output: '+str(err.output)+ \
                              '\n retunrcode: '+ str(err.returncode))
            return err.returncode
        except OSError as err:
            self.logger.fatal('could not start VM '+self.vmname+': '+err.strerror)
            return 3
        else:
            logCmdSuccess(self.logger,'started the VM '+self.vmname, str1)
            return 0     
    
    def stop(self):
        if self.online or (not self.online and self.isonline()):
            self.logger.info('shutdown the VM '+self.vmname)
            self.sshcmd('shutdown -hP now')
            time.sleep(self.bootDelay)
            self.isonline()
        return 0
            
    
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
                                    stderr=sp.STDOUT).decode("utf-8")

        except sp.CalledProcessError as err:
            self.logger.fatal('cold not copy '+ ssh_identify_str +' to '+ \
                              target+'\n cmd: '+str(err.cmd)+'\n output: '+ \
                              str(err.output) + '\n returncode: ' + \
                              str(err.returncode))
            return err.returncode
        self.logger.info('SUCCESS: '+self.vmname+': copied files\n output:'+str1)
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
                                    stderr=sp.STDOUT).decode("utf-8")

        except sp.CalledProcessError as err:
            self.logger.fatal('cold not copy '+source+' to '+ssh_identify_str+ \
                              '\n cmd: '+str(err.cmd)+'\n output: '+ \
                              str(err.output) + '\n returncode: ' +\
                              str(err.returncode))
            return err.returncode
        self.logger.info('SUCCESS: '+self.vmname+': copied files\n output:'+str1)
        return 0
