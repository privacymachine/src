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

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QColor>

class TestUserConfigOpenVPN;

struct ConfigVmMask
{
  /// Attempts to parse the given comma-separated list of browsers into \c browserList.
  /// This method also makes sure that all entries within \c browserList exactly match the corresponding binary name.
  /// So, in subsequent calls, these can be used for starting the individual browsers.
  bool parseBrowsers( QString browsers );

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

struct ConfigVPN
{
    QString name;             // from Name of Section
    QString directoryName;    // something like 'vpn_name'                    based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QString configPath;       // something like '/path/vpn_name'              based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QStringList configFiles;  // something like '/path/vpn_name/Germany.ovpn' based on ConfigFiles-Option '/path/vpn_name/*.ovpn'
    QString vpnType;
};

struct ConfigUpdate
{
    QString appcastPM;
    QString appcastBaseDisk;
};

class UserConfig
{
  friend class TestUserConfigOpenVPN;

  public:
    UserConfig(QString parIniFile, QString parUserConfigDir, QString parInstallDir);
    virtual ~UserConfig();

    QList<ConfigVmMask*>& getConfiguredVmMasks();
    QList<ConfigVPN*>& getConfiguredVPNs();
    ConfigUpdate getUpdateConfig(){ return updateConfiguration_; }

    bool readFromFile();

  private:
    QString convertIniValueToString(QVariant parVar);
    bool parseOpenVpnConfigFiles(QString parConfigFilesSearch, QString& parConfigPath, QString& parDirectoryName, QString& parConfigFilter);
    bool findOpenVpnConfigFiles(ConfigVPN* parConfigVPN, QString parConfigPath, QString parConfigFilter);

    QString iniFile_;
    QString userConfigDir_;
    QString installDir_;
    QList<ConfigVmMask*> configuredVmMasks_;
    QList<ConfigVPN*> configuredVPNs_;
    ConfigUpdate updateConfiguration_;
};
