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
#include "SystemConfig.h"

#include <QSettings>
#include <QCoreApplication>
#include <QTextCodec>
#include <QFile>
#include <QRegExp>
#include <QVariant>
#include <QProcessEnvironment>

#include <iostream>
using namespace std;

SystemConfig::SystemConfig()
{
}

SystemConfig::~SystemConfig()
{
  if (pSettings_)
  {
    delete pSettings_;
    pSettings_ = 0;
  }
}

bool SystemConfig::init()
{
  if (!read())
    return false;

  if (!determineVBoxCommand())
    return false;

  return true;
}

bool SystemConfig::read()
{
  // Under Windows the Registry is used
  QString iniFile = QCoreApplication::applicationDirPath() + "/PrivacyMachineInternals.ini";
  pSettings_ = new QSettings(iniFile, QSettings::IniFormat);
  pSettings_->setIniCodec("UTF-8");

  pSettings_->beginGroup("Update");
  machineBootUpTime_ = pSettings_->value("MachineBootUpTime", QVariant(200)).toInt();
  machineServiceStartupTime_ = pSettings_->value("MachineServiceStartupTime", QVariant(100)).toInt();
  machineRestoreTime_ = pSettings_->value("MachineRestoreTime", QVariant(5)).toInt();
  waitTimeAfterCreateSnapshot_ = pSettings_->value("WaitTimeAfterCreateSnapshot", QVariant(10)).toInt();
  waitTimeAfterPowerOff_ = pSettings_->value("WaitTimeAfterPowerOff", QVariant(5)).toInt();
  lastUpdateTime_ = pSettings_->value("LastUpdateTime", QDateTime::fromMSecsSinceEpoch(0)).toDateTime();
  pSettings_->endGroup();

  return true;
}

bool SystemConfig::write()
{
  pSettings_->beginGroup("Update");
  pSettings_->setValue("MachineBootUpTime", machineBootUpTime_);
  pSettings_->setValue("MachineServiceStartupTime", machineServiceStartupTime_);
  pSettings_->setValue("MachineRestoreTime", machineRestoreTime_);
  pSettings_->setValue("WaitTimeAfterCreateSnapshot", waitTimeAfterCreateSnapshot_);
  pSettings_->setValue("WaitTimeAfterPowerOff", waitTimeAfterPowerOff_);
  pSettings_->setValue("LastUpdateTime", lastUpdateTime_);
  pSettings_->endGroup();

  return true;
}

bool SystemConfig::determineVBoxCommand()
{
  if (RunningOnWindows())
  {
    // We can't use QSettings-Registry-Methods here because we are running under WOW64
    // so we get it from the environment variable 'VBOX_INSTALL_PATH'
    vboxCommand_ = QProcessEnvironment::systemEnvironment().value("VBOX_INSTALL_PATH", "C:\\VirtualBoxDir_NOTFOUND\\");
    vboxCommand_ += "VBoxManage.exe";
  }
  else
  {
    // on Linux it is usually on the path
    vboxCommand_ = "vboxmanage";
  }
  return true;
}


