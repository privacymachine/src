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

#include <QSettings>
#include <QCoreApplication>
#include <QTextCodec>
#include <QFile>
#include <QRegExp>

#include <iostream>
using namespace std;


bool ConfigVmMask::parseBrowsers( QString browsers )
{
  QStringList rawBrowserList = browsers.split( ',', QString::SkipEmptyParts );

  foreach( QString browser, rawBrowserList )
    browserList.append(browser.toLower());

  return true;

}


UserConfig::UserConfig(QString parIniFile, QString parUserConfigDir, QString parInstallDir):
  iniFile_(parIniFile),
  userConfigDir_(parUserConfigDir),
  installDir_(parInstallDir)
{
}

UserConfig::~UserConfig()
{
  foreach(ConfigVmMask* curVmMask, configuredVmMasks_)
  {
    delete curVmMask;
  }
  configuredVmMasks_.clear();

  foreach(ConfigVPN* curVPNConfig, configuredVPNs_)
  {
    delete curVPNConfig;
  }
  configuredVPNs_.clear();
}

QString UserConfig::convertIniValueToString(QVariant parVar)
{
  // Unforutnatly Values with "," are returned as QStringList

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
  bool success = true;
  // Important Windows-Notes:
  // Ini-File has to start with a commented line (starting with ';') because the default notepad.exe writes UTF-8 with BOM (Byte Order Mark-Header)
  // Because of a QT-Bug the first line after the BOM-Header will be ignored: https://bugreports.qt-project.org/browse/QTBUG-23381
  // Also it's important not to modify this file inside qt, because the remarks will be removed -> and the first line will be ignored

  // Important Linux-Notes:
  // A global call needed in i.e. main(): QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  
  QSettings settingsRead(iniFile_, QSettings::IniFormat);
  settingsRead.setIniCodec("UTF-8");

  // By reading the PMConfigVersion we can check if the ini file exists 
  int pmConfigVerion = settingsRead.value("PRIVACYMACHINE/PMConfigVersion", 0).toInt();
  if (pmConfigVerion == 0)
  {
    IERR("Configuration file not found: " + iniFile_);
    return false;
  }

  foreach (QString section, settingsRead.childGroups())
  {
    QString vmMaskPrefix = "VmMask_";
    QString vpnPrefix = "VPN_";
    if (section.startsWith(vmMaskPrefix))
    {
      ConfigVmMask* curVmMask = new ConfigVmMask();
      QString vmMaskName = section.mid(vmMaskPrefix.length());
      curVmMask->name = vmMaskName;
      curVmMask->vmName = "pm_" + vmMaskPrefix + vmMaskName;
      curVmMask->fullName = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/FullName"));
      curVmMask->description = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Description"));
      QString argbHex = settingsRead.value( vmMaskPrefix + vmMaskName + "/Color", QString( "#FF000000" ) ).toString();
      curVmMask->color.setNamedColor( argbHex );
      QString netConf = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/NetworkConnection","none"));
      curVmMask->networkConnection = netConf;
      // On failure, success becomes false, but we still attempt to parse the rest in order to identify as many errors
      // as possible in one go.
      if ( netConf!="TOR" && netConf!="LocalIp" && !netConf.startsWith("VPN_") )
      {
         IERR("In configuration file section ["+section+"]: "+netConf+" is not a valid NetworkConnection");
         success=false;
      }

      curVmMask->ipAddressProviders = 
        convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/IpAddressProviders"));
      curVmMask->dnsServers =
        convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/DnsServer", QString("37.235.1.174,37.235.1.177")));
      QString locales = convertIniValueToString( settingsRead.value( vmMaskPrefix + vmMaskName + "/Locales") );
      curVmMask->localeList = locales.split( ',', QString::SkipEmptyParts );
      curVmMask->browserLanguages = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Languages"));
      curVmMask->java = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Java"));
      curVmMask->flash = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Flash"));
      curVmMask->thirdPartyCookies = 
        convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ThirdPartyCookies"));
      curVmMask->parseBrowsers( convertIniValueToString( settingsRead.value(vmMaskPrefix + vmMaskName + "/Browsers") ) );
      curVmMask->scriptOnStartup = 
        convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ScriptOnStartup"));
      curVmMask->scriptOnShutdown = 
        convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ScriptOnShutdown"));

      configuredVmMasks_.append(curVmMask);
    }
    else if (section.startsWith(vpnPrefix))
    {
      ConfigVPN* curVPNConfig = new ConfigVPN();
      QString vpnName = section.mid(vpnPrefix.length());
      curVPNConfig->name = vpnName;

      curVPNConfig->vpnType = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/Type"));
      if (curVPNConfig->vpnType == "OpenVPN")
      {
        QString configFilesSearch = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/ConfigFiles"));
        QString configPath;
        QString directoryName;
        QString configFilter;

        if (!parseOpenVpnConfigFiles(configFilesSearch, configPath, directoryName, configFilter))
        {
          success = false;
        }
        curVPNConfig->configPath = configPath;
        curVPNConfig->directoryName = directoryName;

        if (!findOpenVpnConfigFiles(curVPNConfig, configPath, configFilter))
        {
          success = false;
        }
      }
      else if ( section == "VPN_VPNGate")
      {
        if (curVPNConfig->vpnType != "VPNGate")
        {
          IERR("In configuration file section ["+section+"]: Type has to be VPNGate");
          success = false;
        }
      }
      else
      {
        IERR("In configuration file section ["+section+"]: "+curVPNConfig->vpnType+" is not a supportet VPN-Type");
        success = false;
      }
      configuredVPNs_.append( curVPNConfig );      
    }
    else if (section == "UPDATE")
    {
      updateConfiguration_.appcastPM = convertIniValueToString(settingsRead.value(section+"/PMUpdateUrl","no URL"));
      updateConfiguration_.appcastBaseDisk = convertIniValueToString(settingsRead.value(section+"/BaseDiskUpdateUrl","no URL"));
      if(updateConfiguration_.appcastPM.size() < 10 || !updateConfiguration_.appcastPM.contains("://") )
      {
        IERR("In configuration file section ["+section+"]: PMUpdateUrl="+updateConfiguration_.appcastPM+": not a valid URL");
        success = false;
      }
      if(updateConfiguration_.appcastBaseDisk.size() < 10 || !updateConfiguration_.appcastBaseDisk.contains("://") )
      {
        IERR("In configuration file section ["+section+"]: BaseDiskUpdateUrl="+updateConfiguration_.appcastBaseDisk+": not a valid URL");
        success = false;
      }
    }
    else if( section!="PRIVACYMACHINE" && section!="TOR" && section!="LocalIp")
    {
      IERR("In configuration file: ["+section+"] is not a valid section");
      success = false;
    }
  }

  foreach (ConfigVmMask* vmMask, configuredVmMasks_)
  {
    QString netConf = vmMask->networkConnection;
    if ( netConf.startsWith("VPN_") )
    {
      bool vpnConfigured = false;
      foreach (ConfigVPN* vpnConfig, configuredVPNs_)
      {
        if(netConf == "VPN_"+vpnConfig->name)
        {
          vpnConfigured=true;
          break;
        }
      }
      if(!vpnConfigured)
      {
        IERR("In configuration file section [VmMask_"+vmMask->name+"]:  NetworkConnection "+netConf+" is not a defined VPN");
        success = false;
      }
    }
  }

  return success;
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

bool UserConfig::findOpenVpnConfigFiles(ConfigVPN* parConfigVPN, QString parConfigPath, QString parConfigFilter)
{
  QDir configDir(parConfigPath);
  if (!configDir.exists())
  {
    IERR("VPN-Config dir does not exist: " + parConfigPath);
    return false;
  }

  configDir.setNameFilters( QStringList( parConfigFilter ) );
  parConfigVPN->configFiles = configDir.entryList( QDir::Files | QDir::Readable | QDir::CaseSensitive );

  if (parConfigVPN->configFiles.count() == 0)
  {
    IERR("No VPN-Config(" + parConfigFilter + ") file found in: " + parConfigPath);
    return false;
  }

  return true;
}

QList<ConfigVmMask*>& UserConfig::getConfiguredVmMasks()
{
  return configuredVmMasks_;
}

QList<ConfigVPN*>& UserConfig::getConfiguredVPNs()
{
  return configuredVPNs_;
}
