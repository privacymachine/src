/*==============================================================================
        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

                     Licensed under the EUPL, Version 1.1
     European Commission - subsequent versions of the EUPL (the "Licence");
        You may not use this work except in compliance with the Licence.
                  You may obtain a copy of the Licence at:
                        http://ec.europa.eu/idabc/eupl

 Unless required by applicable law or agreed to in writing, software distributed
              under the Licence is distributed on an "AS IS" basis,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
      See the Licence for the specific language governing permissions and
                        limitations under the Licence.
==============================================================================*/

#include "PmVpnHelper.h"
#include "VmMaskInstance.h"

// These constants are also used in the file:
//   packaging/BaseDisk/pm_files/etc/systemd/system/openvpn_pm.service
const QString PmVpnHelper::VM_OPENVPN_CONFIG_DIRECTORY_PATH = "/home/vmConfig";
const QString PmVpnHelper::VM_OPENVPN_STATIC_FILE_NAME = "openvpn_pm.ovpn";

PmVpnHelper::PmVpnHelper(QSharedPointer<VmMaskInstance> parVmMaskInstance)
{
  vmMaskInstance_ = parVmMaskInstance;
  vpnConfig_ = vmMaskInstance_->getConfig()->getVpnConfig();
}

PmVpnHelper::~PmVpnHelper()
{
}

QString PmVpnHelper::getRandomConfigFileName()
{
  // TODO: bernhard: move to VmMaskUserConfig::diceNewVmMaskConfig()
  int pos = randombytes_uniform(vpnConfig_.ConfigFiles.count());
  return vpnConfig_.ConfigFiles.at(pos);
}

bool PmVpnHelper::addCmdToInitVPN(QList<PmCommand*>& parCmdList)
{
  QString vmMaskFullName = vmMaskInstance_->getConfig()->getFullName();
  int sshPort = vmMaskInstance_->getConfig()->getSshPort();
  QString desc;
  PmCommand* curCmd;

  if( vpnConfig_.VpnType == "OpenVPN" )
  {
    // The OpenVPN-Service always expect a file /home/vmConfig/vpn/openvpn_pm.ovpn
    // scp behaves a little weird so we copy the files in the following way:
    //   ssh vmname -c mkdir -p /home/vmConfig/tmp
    //   scp ~./conf/vpn/MyVpn vmname:/home/vmConfig/tmp
    //      -> results in directory: /home/vmConfig/tmp/MyVpn
    //   ssh vmname -c mv /home/vmConfig/tmp/MyVpn /home/vmConfig/vpn
    //   ssh vmname -c ln -s /home/vmConfig/vpn/germany.ovpn /home/vmConfig/vpn/openvpn_pm.ovpn

    desc = "Creating VPN configuration directory inside VM-Mask '" + vmMaskFullName + "'";
    curCmd = genSshCmd("mkdir -p " + PmVpnHelper::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp", sshPort);
    curCmd->setDescription(desc);
    curCmd->setExecutionCosts(200);
    parCmdList.append(curCmd);

    desc = "Copy OpenVPN configuration files to VM-Mask '" + vmMaskFullName + "'";
    curCmd = genScpCmd( vpnConfig_.ConfigPath,
                        PmVpnHelper::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp/",
                        sshPort);
    curCmd->setDescription(desc);
    curCmd->setExecutionCosts(200);
    parCmdList.append(curCmd);

    desc = "Move VPN configuration directory to the expected location";
    curCmd = genSshCmd("mv '" + PmVpnHelper::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp/" + vpnConfig_.DirectoryName + "' " + PmVpnHelper::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn", sshPort);
    curCmd->setDescription(desc);
    parCmdList.append(curCmd);

    desc = "Create link for of random choosen VPN-config-file to to default name";
    curCmd = genSshCmd("ln -s '" + VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn/" + getRandomConfigFileName() + "' " +
                       VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn/" + VM_OPENVPN_STATIC_FILE_NAME,
                       sshPort);
    curCmd->setDescription(desc);
    parCmdList.append(curCmd);
  }
  else // "VPNGate"
  {
    // start a script which downloads the list of currently available VPNGate-Servers and configures a
    // file named /home/vmConfig/openvpn_pm.ovpn
    desc = "Downloading VPNGate server list";
    curCmd = genSshCmd("wget -O /tmp/vpngate.csv http://www.vpngate.net/api/iphone/", sshPort);
    curCmd->setDescription(desc);
    curCmd->setExecutionCosts(1000);
    curCmd->setRetries(3); // because server seams sometimes inresponsive
    parCmdList.append(curCmd);

    desc = "Creating a OpenVPN configuration file for VPN-Gate";
    /// todo: @olaf: use configred dns-server as parameter
    curCmd = genSshCmd("/pm/scripts/configureVpnGate.py /tmp/vpngate.csv", sshPort);
    curCmd->setDescription(desc);
    curCmd->setExecutionCosts(1000);
    parCmdList.append(curCmd);
  }

  return true;
}

bool PmVpnHelper::addCmdToInitSystemD(QList<PmCommand*>& parCmdList, QString parBrowser)
{
  QString vmMaskFullName = vmMaskInstance_->getConfig()->getFullName();
  int sshPort = vmMaskInstance_->getConfig()->getSshPort();
  QString desc;
  PmCommand* curCmd;

  desc = "Set up openvpn_log trigger for VM-Mask '" + vmMaskFullName + "'";
  curCmd = genSshCmd("systemctl start openvpn_log.path", sshPort );
  curCmd->setDescription(desc);
  curCmd->setExecutionCosts(20);
  parCmdList.append(curCmd);

  desc = "Set up " + parBrowser + " trigger for VM-Mask '" + vmMaskFullName + "'";
  curCmd = genSshCmd("systemctl start " + parBrowser + ".path", sshPort);
  curCmd->setDescription(desc);
  curCmd->setExecutionCosts(20);
  parCmdList.append(curCmd);

  // Trigger VPN connection(openvpn_pm.service), which triggers:
  // * Log output display in dedicated xterm window (openvpn_log.service)
  // * an update of resolv.conf on successful connection, which in turn triggers
  // * firefox starts (firefox.service) because in firefox.path /etc/resolv.conf gets triggered (also depends on openvpn_pm.service)

  /*
  openvpn_log.service:
         dependsOn openvpn_log.path which gets activated when the file /var/log/openvpn.log gets changed
         starts a xterm showing the file /var/log/openvpn.log
  firefox.service dependsOn openvpn_pm.service AND firefox.path which gets activated when /etc/resolv.conf gets changed
  openvpn_pm.service starts the openvpn
  */

  // TODO: rename systemd files:
  // firefox.service     -> pmFirefoxAutoStartAfterOpenVPN.service
  // firefox.path        -> pmFirefoxAutoStartAfterOpenVPN.path
  // openvpn_log.service -> pmOpenVpnLogViewer.service
  // openvpn_log.path    -> pmOpenVpnLogViewer.path
  // openvpn_pm.service  -> pmOpenVpnConnection.service

  desc = "Start OpenVPN connection for VM-Mask '" + vmMaskFullName + "'";
  curCmd = genSshCmd("systemctl start openvpn_pm.service", sshPort);
  curCmd->setDescription(desc);
  curCmd->setExecutionCosts(200);
  parCmdList.append(curCmd);

  return true;
}
