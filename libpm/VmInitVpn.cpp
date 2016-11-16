/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
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

#include "VmInitVpn.h"

// These constants are also used in the file:
//   packaging/build_base-disk/pm_files/etc/systemd/system/openvpn_pm.service
const QString VmInitVpn::VM_OPENVPN_CONFIG_DIRECTORY_PATH = "/home/vmConfig";
const QString VmInitVpn::VM_OPENVPN_STATIC_FILE_NAME = "openvpn_pm.ovpn";

VmInitVpn::VmInitVpn( const ConfigVPN& configVpn )
{
  configVpn_ = configVpn;
}


VmInitVpn::~VmInitVpn()
{}


QString VmInitVpn::getCommandVmLinkConfig()
{
  // Example: ln -s /home/vmConfig/vpn/germany.conf /home/vmConfig/vpn/openvpn_pm.ovpn

  return
    "ln -s '" + VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn/" + getRandomConfigFileName() + "' "
      +         VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn/" + VM_OPENVPN_STATIC_FILE_NAME;

}


QString VmInitVpn::getCommandVmConnect()
{
  return "systemctl start openvpn_pm.service";

}


QString VmInitVpn::getHostConfigDirectoryPath()
{
  return configVpn_.configPath;

}

QString VmInitVpn::getHostConfigDirectoryName()
{
  return configVpn_.directoryName;

}

QString VmInitVpn::getRandomConfigFileName()
{
  int pos = randInt( 0, configVpn_.configFiles.count() - 1 );
  return configVpn_.configFiles.at(pos);
}
