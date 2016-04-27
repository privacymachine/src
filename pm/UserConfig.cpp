/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

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

UserConfig::UserConfig()
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
  // Important Windows-Notes:
  // Ini-File has to start with a commented line (starting with ';') because the default notepad.exe writes UTF-8 with BOM (Byte Order Mark-Header)
  // Because of a QT-Bug the first line after the BOM-Header will be ignored: https://bugreports.qt-project.org/browse/QTBUG-23381
  // Also it's important not to modify this file inside qt, because the remarks will be removed -> and the first line will be ignored

  // Important Linux-Notes:
  // A global call needed in i.e. main(): QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

  // TODO: After GLT16: use Localisation for choosing the correct Ini-File
  QString iniFile = QCoreApplication::applicationDirPath() + "/PrivacyMachine_de.ini";
  
  QSettings settingsRead(iniFile, QSettings::IniFormat);
  settingsRead.setIniCodec("UTF-8");

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
      curVmMask->networkConnection = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/NetworkConnection"));
      curVmMask->languages = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Languages"));
      curVmMask->java = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Java"));
      curVmMask->flash = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Flash"));
      curVmMask->thirdPartyCookies = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ThirdPartyCookies"));
      curVmMask->browsers = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/Browsers"));
      curVmMask->scriptOnStartup = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ScriptOnStartup"));
      curVmMask->scriptOnShutdown = convertIniValueToString(settingsRead.value(vmMaskPrefix + vmMaskName + "/ScriptOnShutdown"));

      configuredVmMasks_.append(curVmMask);
    }
    else if (section.startsWith(vpnPrefix))
    {
      ConfigVPN* curVPNConfig = new ConfigVPN();
      QString vpnName = section.mid(vpnPrefix.length());
      curVPNConfig->name = vpnName;
      curVPNConfig->configFiles = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/ConfigFiles"));
      curVPNConfig->vpnType = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/VPNType"));
      curVPNConfig->login = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/Login"));
      curVPNConfig->password = convertIniValueToString(settingsRead.value(vpnPrefix + vpnName + "/Password"));
      ILOG_SENSITIVE("read Password from ini file: " + curVPNConfig->password);
      // TODO: read encrypted Password here
      delete (curVPNConfig);
    }
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
