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

#include "utils.h"
#include "UserConfig.h"
#include "VmMaskUserConfig.h"

#include <QSettings>
#include <QCoreApplication>
#include <QTextCodec>
#include <QFile>
#include <QRegExp>

#include <iostream>
using namespace std;

UserConfig::UserConfig(QString parIniFile, QString parUserConfigDir, QString parInstallDir):
  iniFile_(parIniFile),
  userConfigDir_(parUserConfigDir),
  installDir_(parInstallDir)
{
}

UserConfig::~UserConfig()
{
  while (!configuredVmMasks_.isEmpty())
  {
    VmMaskUserConfig* vmMaskUserConfig = configuredVmMasks_.takeLast();
    delete vmMaskUserConfig;
  }

  while (!configuredVPNs_.isEmpty())
  {
    VpnConfig* vpnConfig = configuredVPNs_.takeLast();
    delete vpnConfig;
  }
}

bool UserConfig::parseBrowsers(QString browsers, QStringList& browserList)
{
  QStringList rawBrowserList = browsers.split( ',', QString::SkipEmptyParts );

  foreach( QString browser, rawBrowserList )
    browserList.append(browser.toLower());

  return true;
}


QString UserConfig::convertIniValueToString(QVariant parVar)
{
  // Unforutnately Values with "," are returned as QStringList

  QString retVal = "";
  if (parVar.isValid() && !parVar.isNull())
  {
    if (QString(parVar.typeName()) == "QStringList")
      retVal = parVar.toStringList().join(",");
    else
      retVal = parVar.toString();
  }

  return retVal;
}

bool UserConfig::readFromFile()
{
  // Important Windows-Notes:
  // Ini-File has to start with a commented line (starting with ';') because the default notepad.exe writes UTF-8 with BOM (Byte Order Mark-Header)
  // Because of a QT-Bug the first line after the BOM-Header will be ignored: https://bugreports.qt-project.org/browse/QTBUG-23381
  // Also it's important not to modify this file inside qt, because the remarks will be removed -> and the first line will be ignored
  
  QSettings settingsRead(iniFile_, QSettings::IniFormat);
  settingsRead.setIniCodec("UTF-8");

  // By reading the 'PMConfigVersion' we can check if the ini file exists
  int pmConfigVerion = settingsRead.value("PRIVACYMACHINE/PMConfigVersion", 0).toInt();
  if (pmConfigVerion == 0)
  {
    IERR("Configuration file not found: " + iniFile_);
    return false;
  }

  foreach (QString section, settingsRead.childGroups())
  {
    if (section.startsWith(constVmMaskPrefix))
    {
      VmMaskUserConfig* vmMaskUserConfig = new VmMaskUserConfig();
      QString vmMaskName = section.mid(QString(constVmMaskPrefix).length());
      QString sec = constVmMaskPrefix + vmMaskName; // section in Ini-File

      vmMaskUserConfig->setName(vmMaskName);
      vmMaskUserConfig->setFullName(convertIniValueToString(settingsRead.value(sec + "/FullName")));
      vmMaskUserConfig->setDescription(convertIniValueToString(settingsRead.value(sec + "/Description")));
      vmMaskUserConfig->setColor(settingsRead.value(sec + "/Color", QString("#FF000000")).toString());
      vmMaskUserConfig->setNetworkConnectionType(convertIniValueToString(settingsRead.value(sec + "/NetworkConnection", "NoNetworkConfigured")));

      QString ipAddressProviders = convertIniValueToString(settingsRead.value(sec + "/IpAddressProviders"));
      vmMaskUserConfig->setIpAddressProviders(ipAddressProviders.split(',', QString::SkipEmptyParts));

      QString dnsServers = convertIniValueToString(settingsRead.value(sec + "/DnsServer"));
      vmMaskUserConfig->setDnsServers(dnsServers.split(',', QString::SkipEmptyParts));

      QString locales = convertIniValueToString( settingsRead.value( sec + "/Locales") );
      vmMaskUserConfig->setLocales(locales.split(',', QString::SkipEmptyParts));

      QString browserLang = convertIniValueToString(settingsRead.value(sec + "/Languages"));
      vmMaskUserConfig->setBrowserLanguages(browserLang.split(',', QString::SkipEmptyParts));

      vmMaskUserConfig->setJava(settingsRead.value(sec + "/Java").toBool());
      vmMaskUserConfig->setFlash(settingsRead.value(sec + "/Flash").toBool());
      vmMaskUserConfig->setThirdPartyCookies(convertIniValueToString(settingsRead.value(sec + "/ThirdPartyCookies")));

      QString browsers = convertIniValueToString(settingsRead.value(sec + "/Browsers"));
      vmMaskUserConfig->setBrowsers(browsers.split(',', QString::SkipEmptyParts));

      vmMaskUserConfig->setScriptOnStartup(convertIniValueToString(settingsRead.value(sec + "/ScriptOnStartup")));
      vmMaskUserConfig->setScriptOnShutdown(convertIniValueToString(settingsRead.value(sec + "/ScriptOnShutdown")));

      configuredVmMasks_.append(vmMaskUserConfig);
    }
    else if (section.startsWith(constIniVpnPrefix))
    {
      VpnConfig* curVPNConfig = new VpnConfig();
      QString vpnName = section.mid(QString(constIniVpnPrefix).length());
      curVPNConfig->Name = vpnName;

      curVPNConfig->VpnType = convertIniValueToString(settingsRead.value(constIniVpnPrefix + vpnName + "/Type"));
      if (curVPNConfig->VpnType == "OpenVPN")
      {
        QString configFilesSearch = convertIniValueToString(settingsRead.value(constIniVpnPrefix + vpnName + "/ConfigFiles"));
        QString configPath;
        QString directoryName;
        QString configFilter;

        if (!parseOpenVpnConfigFiles(configFilesSearch, configPath, directoryName, configFilter))
        {
          return false;
        }
        curVPNConfig->ConfigPath = configPath;
        curVPNConfig->DirectoryName = directoryName;

        if (!findOpenVpnConfigFiles(curVPNConfig, configPath, configFilter))
        {
          return false;
        }
      }
      else if ( section == "VPN_VPNGate")
      {
        if (curVPNConfig->VpnType != "VPNGate")
        {
          IERR("In configuration file section [" + section + "]: Type has to be VPNGate");
          return false;
        }
      }
      else
      {
        IERR("In configuration file section [" + section + "]: "+curVPNConfig->VpnType+" is not a supportet VPN-Type");
        return false;
      }
      configuredVPNs_.append( curVPNConfig );      
    }
    else if (section == "UPDATE")
    {
      appcastUrl_ = convertIniValueToString(settingsRead.value(section+"/PMUpdateUrl","no URL"));
      if( !appcastUrl_.isValid() )
      {
        IERR("In configuration file section [" + section + "]: PMUpdateUrl="+appcastUrl_.toString()+": not a valid URL");
        return false;
      }
    }
    else if( section!="PRIVACYMACHINE" && section!="TOR" && section!="LocalIp")
    {
      IERR("In configuration file: [" + section + "] is not a valid section");
      return false;
    }
  }
  return true;
}

bool UserConfig::setDefaultsAndValidateConfiguration(const QJsonObject& parBaseDiskCapabilities)
{
  int vmMaskId = 0;

  foreach (VmMaskUserConfig* vmMaskUserConfig, configuredVmMasks_)
  {
    // Set the VmMask-Id (=same of radio buttons)
    vmMaskUserConfig->setVmMaskId(vmMaskId);

    if (!vmMaskUserConfig->setConfigurationDefaultsAndCheckForErrors(parBaseDiskCapabilities))
    {
      IERR("The Configuration file section [VmMask_"+vmMaskUserConfig->getName() + "] is invalid");
      return false;
    }

    // validate the network configurations for each VmMask
    QString connectionType = vmMaskUserConfig->getNetworkConnectionType();
    if ( connectionType.startsWith("VPN_") ) // VPNGate or custom OpenVPN-Provider
    {
      bool vpnConfigured = false;
      foreach (const VpnConfig* vpnConfig, configuredVPNs_)
      {
        if(connectionType == "VPN_" + vpnConfig->Name)
        {
          vmMaskUserConfig->setVpnConfig(*vpnConfig);
          vpnConfigured=true;
          break;
        }
      }
      if(!vpnConfigured)
      {
        IERR("In configuration file section [VmMask_"+vmMaskUserConfig->getName() + "]:  NetworkConnection " + connectionType + " is not a defined VPN");
        return false;
      }
    }
    else if ( connectionType == "TOR" )
    {
      VpnConfig config;
      config.VpnType = "TOR";
      config.Name = "TOR";
      vmMaskUserConfig->setVpnConfig(config);
    }
    else if ( connectionType == "LocalIp" )
    {
      VpnConfig config;
      config.VpnType = "LocalIp";
      config.Name = "LocalIp";
      vmMaskUserConfig->setVpnConfig(config);
    }
    else if ( connectionType == "NoNetworkConfigured" )
    {
      // we default to LocalIp
      vmMaskUserConfig->setNetworkConnectionType("LocalIp");

      VpnConfig config;
      config.VpnType = "LocalIp";
      config.Name = "LocalIp";
      vmMaskUserConfig->setVpnConfig(config);
    }
    vmMaskId++;
  }

  return true;
}

bool UserConfig::parseOpenVpnConfigFiles(QString parConfigFilesSearch, QString& parConfigPath, QString& parDirectoryName, QString& parConfigFilter)
{
  // examples of valid option 'ConfigFiles':
  //   /some/path/*.ovpn
  //   /some/path/provider.*
  //   {INSTALL_DIR}/vpn/*.conf
  //   {USER_CONFIG_DIR}/vpn/*.conf
  //   C:\some\path\provider.*
  //   /some/path/germany.conf
  //   C:\some\path\germany.conf

  parConfigFilesSearch.replace(QString("{INSTALL_DIR}"), installDir_);
  parConfigFilesSearch.replace(QString("{USER_CONFIG_DIR}"), userConfigDir_);

  QRegExp slashOrBackslash("[/\\\\]");

  QList<QString> configPaths = parConfigFilesSearch.split(slashOrBackslash);
  if (configPaths.count() <= 2)
  {
    IERR("No valid parameter for VPN-ConfigFiles: path separator count: 0");
    return false;
  }

  parDirectoryName = configPaths.at(configPaths.count()-2);

  parConfigFilter = configPaths.at(configPaths.count()-1);
  int posSlashBeforeFilter = parConfigFilesSearch.lastIndexOf(slashOrBackslash);
  if (posSlashBeforeFilter <= 2)
  {
    IERR("No valid parameter for VPN-ConfigFiles: path too small");
    return false;
  }
  parConfigPath = parConfigFilesSearch.left(posSlashBeforeFilter);

  return true;
}

bool UserConfig::findOpenVpnConfigFiles(VpnConfig* parConfigVPN, QString parConfigPath, QString parConfigFilter)
{
  QDir configDir(parConfigPath);
  if (!configDir.exists())
  {
    IERR("VPN-Config dir does not exist: " + parConfigPath);
    return false;
  }

  configDir.setNameFilters( QStringList( parConfigFilter ) );
  parConfigVPN->ConfigFiles = configDir.entryList( QDir::Files | QDir::Readable | QDir::CaseSensitive );

  if (parConfigVPN->ConfigFiles.count() == 0)
  {
    IERR("No VPN-Config(" + parConfigFilter + ") file found in: " + parConfigPath);
    return false;
  }

  return true;
}

QList<VmMaskUserConfig*>& UserConfig::getConfiguredVmMasks()
{
  return configuredVmMasks_;
}

QList<VpnConfig*>& UserConfig::getConfiguredVPNs()
{
  return configuredVPNs_;
}
