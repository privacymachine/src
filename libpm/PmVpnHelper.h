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

#pragma once

#include <vector>
#include <QDir>

#include "utils.h"
#include "UserConfig.h"

using std::string;

/// Helper functions for VPN connections.
class PmVpnHelper
{
  public:
    /// \param configVpn At the moment, the only ConfigVPN::vpnType supported is OpenVPN
    PmVpnHelper(QSharedPointer<VmMaskInstance> parVmMaskInstance);

    virtual ~PmVpnHelper();

    /// Once the VM Mask has been prepared for the VPN connection, we can connect to the VPN.
    QString getCommandVmConnect();

    /// \brief VPN initialisation: prepare configuration, download settings
    /// \param parCmdList  out: Commands will appended to this List
    /// \param parVmMaskInstance in: for getting all VmMask-Data
    /// \return false on error
    bool addCmdToInitVPN(QList<PmCommand*>& parCmdList);

    /// \brief Initialize systemd configuration (depends on configured browser)
    /// \param parCmdList  out: Commands will appended to this List
    /// \param parBrowser
    /// \return false on error
    bool addCmdToInitSystemD(QList<PmCommand*>& parCmdList, QString parBrowser);

    static const QString VM_OPENVPN_CONFIG_DIRECTORY_PATH;
    static const QString VM_OPENVPN_STATIC_FILE_NAME;

  private:

    /// Returns a random selected configuration file.
    QString getRandomConfigFileName();

    QSharedPointer<VmMaskInstance> vmMaskInstance_;

    /// Used for determining configuration directory path and VPN connection command (from vmMaskInstance_)
    VpnConfig vpnConfig_;

};

