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

#include "UserConfig.h"
#include <QRegExp>
#include <QJsonObject>
#include <QStringList>

class SystemConfig;
class VmMaskCurrentConfig;
struct VmMaskStaticConfig;

struct VpnConfig
{
    QString Name;             // from Name of Section
    QString DirectoryName;    // something like 'vpn_name'                    based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QString ConfigPath;       // something like '/path/vpn_name'              based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QStringList ConfigFiles;  // something like '/path/vpn_name/Germany.ovpn' based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QString VpnType;          // possible values: OpenVPN/VPNGate/LocalIp/TOR
};

/// Static information of what a VmMaskInstance can look like.
/// This could be e.g. "run Firefox with languages German, French and English, because these are the
/// languages I understand as an end user".
/// In other words, this contains all the randomisable configuration data.
/// Ultimately, each individual VmMaskInstanceUserConfig adds to the entropy of PMs in use by all users.
class VmMaskUserConfig
{
  public:
    /// \param parWholeConfig Holds the VPN configuration, which might be needed by parCurConfigVmMask.
    explicit VmMaskUserConfig();

    bool initWithBaseDiskCapabilities(const QJsonObject& parBaseDiskCapabilities);

    /// obfuscate the VmMask properties by choosing a random fingerprint
    VmMaskCurrentConfig* diceNewVmMaskConfig(VmMaskStaticConfig* parStaticConfig);

    /// \brief check if the User has configured all in the correct way
    /// \brief  on error PrivacyMachine aborts
    /// \return false on error
    bool setConfigurationDefaultsAndCheckForErrors(const QJsonObject& parBaseDiskCapabilities);


    // getter/setter

    QString getName() const;
    void setName(const QString& parName);

    QString getVmName() const;
    void setVmName(const QString& parVmName);

    QString getFullName() const;
    void setFullName(const QString& parFullName);

    QString getDescription() const;
    void setDescription(const QString& parDescription);

    /// Uniquely identifies the corresponding VM-Mask. Used e.g. for updating UI elements like radio buttons.
    int getVmMaskId() const;
    void setVmMaskId(int parIndexOfRadioButtons);

    /// Color used for highlighting the VM-Mask, e.g. drawing a coloured frame around the VM-Mask.
    QColor getColor() const;
    void setColor(const QColor& color);

    /// possible values: OpenVPN/VPNGate/LocalIp/TOR
    QString getNetworkConnectionType() const;
    void setNetworkConnectionType(const QString& networkConnectionType);

    const VpnConfig* getVpnConfig() const;
    void setVpnConfig(const VpnConfig& value);

    QStringList getIpAddressProviders() const;
    void setIpAddressProviders(const QStringList& ipAddressProviders);

    QStringList getDnsServers() const;
    void setDnsServers(const QStringList& dnsServerList);

    QStringList getBrowsers() const;
    void setBrowsers(const QStringList& parBrowserList);

    QString getThirdPartyCookies() const;
    void setThirdPartyCookies(const QString& thirdPartyCookies);

    bool getFlash() const;
    void setFlash(bool parFlash);

    bool getJava() const;
    void setJava(bool parJava);

    QString getScriptOnShutdown() const;
    void setScriptOnShutdown(const QString& scriptOnShutdown);

    QString getScriptOnStartup() const;
    void setScriptOnStartup(const QString& scriptOnStartup);

    QStringList getLocales() const;
    void setLocales(const QStringList& locales);

    QStringList getFonts() const;

    QStringList getBrowserLanguages() const;
    void setBrowserLanguages(const QStringList& parBrowserLanguages);

  private:
    QString name_;
    QString vmName_;
    QString fullName_;
    QString description_;
    int vmMaskId_;
    QColor color_;
    QString networkConnectionType_;
    VpnConfig vpnConfig_;

    // possible values selected from the user
    QStringList timeZones_;
    QStringList locales_;
    QStringList fonts_;
    QStringList browsers_;
    QStringList dnsServers_;
    QStringList browserLanguages_;    // TODO: set intl.accept_languages in about:config
    QStringList ipAddressProviders_;

    // possible values from the BaseDisk
    QStringList timeZonesInBaseDisk_;
    QStringList localesInBaseDisk_;
    QStringList fontsInBaseDisk_;
    QStringList browsersInBaseDisk_;

    // TODO: implement or remove:
    bool flash_;
    bool java_;
    QString scriptOnShutdown_;
    QString scriptOnStartup_;
    QString thirdPartyCookies_;  // TODO: -> about:config + DoNotTrack
};
