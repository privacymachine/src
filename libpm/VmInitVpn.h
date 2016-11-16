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

#pragma once

#include <vector>
#include <QDir>

#include "utils.h"
#include "UserConfig.h"

using std::string;

/// Initializes a VM Mask with a VPN connection.
class VmInitVpn
{
  public:
    /// \param configVpn At the moment, the only ConfigVPN::vpnType supported is OpenVPN
    VmInitVpn( const ConfigVPN& configVpn );

    virtual ~VmInitVpn();

    /// Once the VM Mask has been prepared for the VPN connection, we can connect to the VPN.
    QString getCommandVmConnect();

    /// Creates a link to the selected configuration file, naming it as is expected by openvpn_pm.service.
    /// The openvpn_pm.service does not take the configuration file as argument, but looks for a specific file.
    QString getCommandVmLinkConfig();

    /// The directory on the host which contains the configuration files.
    QString getHostConfigDirectoryPath();

    /// The directory name on the host which contains the configuration files.
    QString getHostConfigDirectoryName();

    static const QString VM_OPENVPN_CONFIG_DIRECTORY_PATH;
    static const QString VM_OPENVPN_STATIC_FILE_NAME;

  private:

    /// Returns a random selected configuration file.
    QString getRandomConfigFileName();

    /// Used for determining configuration directory path and VPN connection command.
    ConfigVPN configVpn_;

};

