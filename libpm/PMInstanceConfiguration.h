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

#include "UserConfig.h"
#include <QRegExp>
#include <QJsonObject>
#include <QStringList>

class SystemConfig;

/// Static information of what a PMInstance can look like.
/// This could be e.g. "run Firefox with languages German, French and English, because these are the
/// languages I understand as an end user".
/// In other words, this contains all the randomisable configuration data.
/// Ultimately, each individual PMInstanceConfiguration adds to the entropy of PMs in use by all users. 
class PMInstanceConfiguration
{
  public:
    /// \param parWholeConfig Holds the VPN configuration, which might be needed by parCurConfigVmMask.
    explicit PMInstanceConfiguration(ConfigVmMask *parCurConfigVmMask, UserConfig *parWholeConfig, SystemConfig *parSystemConfig_, const QJsonObject& parBaseDiskCapabilities);

    /// obfuscate the properties
    void init();


    /// String representation of the configuration, restricted to currently implemented fields. All other fields are
    /// omitted.
    QString toString();

    /// Browsers as configured by user
    QStringList browserListUser;
    /// Locales as configured by user.
    QStringList localeListUser;

    /// Browser chosen randomly from intersection ofuser preference and avaiable browsers.

    QString flash;
    QString fullName;
    /// Color used for highlighting the VM-Mask, e.g. drawing a coloured frame around the VM-Mask.
    QColor color;
    QString hostName;           // Not used? To delete?
    QString java;               // Not used now

    QString browserLanguage;    // Not used? To delete?
    QString name;
    QString networkConnection;  // Not used? To delete?
    QStringList ipAddressProviders;
    unsigned short rdpPort;
    QString scriptOnShutdown;
    QString scriptOnStartup;
    unsigned short sshPort;
    unsigned short subtractDisplayHeight;
    unsigned short subtractDisplayWidth;
    SystemConfig *systemConfig;
    QString thirdPartyCookies;  // Not used? To delete?
    ConfigVPN usedVPNConfig;
    QString vmName;
    /// <c>true</c> if the corresponding VM-Mask is created. Helps in e.g. disabling the "Start" button for creating a
    /// VM-Mask. This actually tolerates hitting the "Start" button when the VM-Mask is not shown anymore, but its VM is
    /// still running, which is considered beneficial: Allows for starting VMs in background and only attaching VM-Masks
    /// later on.
    bool vmMaskCreated;

    /// Uniquely identifies the corresponding VM-Mask. Used e.g. for updating UI elements like radio buttons.
    int vmMaskId;

    /// randomly choosen properties for obfuscation
    QString timeZone;
    QString locale;
    QString browser;
    QStringList fonts;
    QStringList dnsServers;


  private:
    /// Static lists of avaiable properties which are obfuscated
    /// TODO: optimize this, using singeltons
    QStringList timeZoneList_;
    QStringList localeList_;
    QStringList fontList_;
    QStringList browserList_;
    QStringList dnsServerList_;

};
