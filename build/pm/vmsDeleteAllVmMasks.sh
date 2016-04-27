#!/bin/bash
# this scripts removes all the virtual machines created by the PrivacyMachine

# stop the runnung vms
vmName=`vboxmanage list runningvms | grep pm_VmMask_ | tail -n1 | sed -e 's@\s{.*}@@' | sed -e 's@\"@@g'`
until [ -d $vmName ]; do
  echo "stopping vm: $vmName"
  vboxmanage controlvm $vmName poweroff
  vmName=`vboxmanage list runningvms | grep pm_VmMask_ | tail -n1 | sed -e 's@\s{.*}@@' | sed -e 's@\"@@g'`
  sleep 2
done 

# delete the vms completely
vmName=`vboxmanage list vms | grep pm_VmMask_ | tail -n1 | sed -e 's@\s{.*}@@' | sed -e 's@\"@@g'`
until [ -d $vmName ]; do
  echo "delete vm: $vmName"
  vboxmanage unregistervm $vmName --delete
  vmName=`vboxmanage list vms | grep pm_VmMask_ | tail -n1 | sed -e 's@\s{.*}@@' | sed -e 's@\"@@g'`
  sleep 2
done 

