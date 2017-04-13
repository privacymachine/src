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

#include "VmMaskStaticConfig.h"
#include "UserConfig.h"
#include <QRegExp>
#include <QJsonObject>
#include <QStringList>

class PmCommand;

/// Random selected Properties of one VmMaskInstance
class VmMaskCurrentConfig
{
  public:
    VmMaskCurrentConfig(VmMaskStaticConfig* parStaticConfig);

    /// String representation of the configuration, restricted to currently implemented fields. All other fields are omitted.
    QString toString();

    QString getName() const;

    QString getVmName() const;

    QString getFullName() const;

    unsigned short getRdpPort();

    unsigned short getSshPort();

    QString getTimeZone() const;
    void setTimeZone(const QString& value);

    QString getLocale() const;
    void setLocale(const QString& value);

    QStringList getFonts() const;
    void setFonts(const QStringList& value);

    QStringList getDnsServers() const;
    void setDnsServers(const QStringList& value);

    QString getBrowser() const;
    void setBrowser(const QString& browser);

    QColor getColor() const;
    void setColor(const QColor& value);

    bool getJava() const;
    void setJava(bool value);

    bool getFlash() const;
    void setFlash(bool value);

    QStringList getBrowserLanguages() const;
    void setBrowserLanguages(const QStringList& value);

    QStringList getIpAddressProviders() const;
    void setIpAddressProviders(const QStringList& value);

    QString getScriptOnShutdown() const;
    void setScriptOnShutdown(const QString& value);

    QString getScriptOnStartup() const;
    void setScriptOnStartup(const QString& value);

    unsigned short getSubtractDisplayHeight() const;
    void setSubtractDisplayHeight(unsigned short value);

    unsigned short getSubtractDisplayWidth() const;
    void setSubtractDisplayWidth(unsigned short value);

    QString getThirdPartyCookies() const;
    void setThirdPartyCookies(const QString& value);

    VpnConfig getVpnConfig() const;
    void setVpnConfig(const VpnConfig& value);

  private:
    VmMaskStaticConfig staticConfig_;

    /// randomly choosen properties for obfuscation
    QString timeZone_;
    QStringList fonts_;
    QString locale_;
    QStringList dnsServers_;
    QString browser_;
    unsigned short subtractDisplayHeight_;
    unsigned short subtractDisplayWidth_;
    QColor color_; // Color used for highlighting the VM-Mask, e.g. drawing a coloured frame around the VM-Mask.
    bool java_;
    bool flash_;
    QStringList browserLanguages_;   /// @todo: Accept-Language in about:config setzen
    QStringList ipAddressProviders_;
    QString scriptOnShutdown_;
    QString scriptOnStartup_;
    QString thirdPartyCookies_;  // Not used? To delete? -> about:config +  DoNotTrack
    VpnConfig vpnConfig_;
};
