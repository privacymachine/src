/*==============================================================================
        Copyright (c) 2013-2014 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

 Licensed under the EUPL, Version 1.1 or - as soon they will be approved by the
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
  foreach(ConfigUseCase* curUseCase, configuredUseCases_)
  {
    delete curUseCase;
  }
  configuredUseCases_.clear();

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

  QString iniFile = QCoreApplication::applicationDirPath() + "/PrivacyMachine.ini";
  
  QSettings settingsRead(iniFile, QSettings::IniFormat);
  settingsRead.setIniCodec("UTF-8");

  foreach (QString section, settingsRead.childGroups())
  {
    QString useCasePrefix = "UseCase_";
    QString vpnPrefix = "VPN_";
    if (section.startsWith(useCasePrefix))
    {
      ConfigUseCase* curUseCase = new ConfigUseCase();
      QString useCaseName = section.mid(useCasePrefix.length());
      curUseCase->name = useCaseName;
      curUseCase->fullName = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/FullName"));
      curUseCase->networkConnection = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/NetworkConnection"));
      curUseCase->languages = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/Languages"));
      curUseCase->java = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/Java"));
      curUseCase->flash = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/Flash"));
      curUseCase->thirdPartyCookies = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/ThirdPartyCookies"));
      curUseCase->browsers = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/Browsers"));
      curUseCase->scriptOnStartup = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/ScriptOnStartup"));
      curUseCase->scriptOnShutdown = convertIniValueToString(settingsRead.value(useCasePrefix + useCaseName + "/ScriptOnShutdown"));

      configuredUseCases_.append(curUseCase);
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
      ILOG("read Password from ini file: " + curVPNConfig->password);
      // TODO: read encrypted Password here
    }
  }


  return true;
}

QList<ConfigUseCase*>& UserConfig::getConfiguredUseCases()
{
  return configuredUseCases_;
}

QList<ConfigVPN*>& UserConfig::getConfiguredVPNs()
{
  return configuredVPNs_;
}
