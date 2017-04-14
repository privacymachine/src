﻿/*==============================================================================
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

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QColor>

#include "VmMaskUserConfig.h"

class TestUserConfigOpenVPN;
class VmMaskUserConfig;
class VpnConfig;

/*
/// @todo: removeMe
struct ConfigVmMask
{

  QString name; // from Name of Section
  QString fullName;
  /// A description of what use case the VM Mask is intended for.
  QString description;
  QColor color;
  // The name of the virtual machine for this VM Mask
  QString vmName;
  QString networkConnection;
  QString ipAddressProviders;
  /// Locales as configured by user.
  QStringList localeList;
  QString browserLanguages; // separated with ','
  QString java;
  QString flash;
  QString thirdPartyCookies;
  /// Browsers as configured by user.
  QStringList browserList;
  /// DNS-Server as configured by user
  QString dnsServers;
  QString scriptOnStartup;
  QString scriptOnShutdown;
};
*/

struct ConfigUpdate
{
    QString AppcastPM;
    QString AppcastBaseDisk; /// @todo: remove because not used anymore
};

class UserConfig
{
  friend class TestUserConfigOpenVPN;

  public:
    UserConfig(QString parIniFile, QString parUserConfigDir, QString parInstallDir);
    virtual ~UserConfig();

    QList<VmMaskUserConfig*>& getConfiguredVmMasks();
    QList<VpnConfig*>& getConfiguredVPNs();
    ConfigUpdate getUpdateConfig(){ return updateConfiguration_; }

    bool readFromFile();
    bool setDefaultsAndValidateConfiguration(const QJsonObject& parBaseDiskCapabilities);

  private:
    QString convertIniValueToString(QVariant parVar);
    bool parseOpenVpnConfigFiles(QString parConfigFilesSearch, QString& parConfigPath, QString& parDirectoryName, QString& parConfigFilter);
    bool findOpenVpnConfigFiles(VpnConfig* parConfigVPN, QString parConfigPath, QString parConfigFilter);
    bool parseBrowsers(QString browsers, QStringList& browserList);

    QString iniFile_;
    QString userConfigDir_;
    QString installDir_;
    QList<VmMaskUserConfig*> configuredVmMasks_;
    QList<VpnConfig*> configuredVPNs_;
    ConfigUpdate updateConfiguration_;
};