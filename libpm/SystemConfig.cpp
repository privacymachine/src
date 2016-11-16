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

SystemConfig::SystemConfig(QString iniFile):
  iniFile_(iniFile)
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
  if (!readFromFile())
    return false;

  vboxCommand_ = determineVBoxCommand();

  return true;
}

bool SystemConfig::readFromFile()
{
  pSettings_ = new QSettings(iniFile_, QSettings::IniFormat);
  pSettings_->setIniCodec("UTF-8");

  pSettings_->beginGroup("BaseDisk");
  baseDiskPath_ = pSettings_->value("BaseDiskPath").toString();
  baseDiskName_ = pSettings_->value("BaseDiskName").toString();
  pSettings_->endGroup();

  pSettings_->beginGroup("Update");
  machineBootUpTime_ = pSettings_->value("MachineBootUpTime", QVariant(200)).toInt();
  machineServiceStartupTime_ = pSettings_->value("MachineServiceStartupTime", QVariant(100)).toInt();
  machineRestoreTime_ = pSettings_->value("MachineRestoreTime", QVariant(5)).toInt();
  waitTimeAfterCreateSnapshot_ = pSettings_->value("WaitTimeAfterCreateSnapshot", QVariant(10)).toInt();
  waitTimeAfterPowerOff_ = pSettings_->value("WaitTimeAfterPowerOff", QVariant(5)).toInt();
  copyScriptsPerSshTime_ = pSettings_->value("CopyScriptsPerSshTime", QVariant(5)).toInt();
  lastUpdateTime_ = pSettings_->value("LastUpdateTime", QDateTime::fromMSecsSinceEpoch(0)).toDateTime();
  // maybe move to seperate group
  configuredVMMaskNames_ = pSettings_->value("ConfiguredBaseDisks").toStringList();
  pSettings_->endGroup();

  return true;
}

bool SystemConfig::write()
{
  pSettings_->beginGroup("BaseDisk");
  pSettings_->setValue("BaseDiskPath", baseDiskPath_);
  pSettings_->setValue("BaseDiskName", baseDiskName_);
  pSettings_->endGroup();

  pSettings_->beginGroup("Update");
  pSettings_->setValue("MachineBootUpTime", machineBootUpTime_);
  pSettings_->setValue("MachineServiceStartupTime", machineServiceStartupTime_);
  pSettings_->setValue("MachineRestoreTime", machineRestoreTime_);
  pSettings_->setValue("WaitTimeAfterCreateSnapshot", waitTimeAfterCreateSnapshot_);
  pSettings_->setValue("WaitTimeAfterPowerOff", waitTimeAfterPowerOff_);
  pSettings_->setValue("LastUpdateTime", lastUpdateTime_);
  // maybe move to seperate group
  pSettings_->setValue("ConfiguredBaseDisks", configuredVMMaskNames_);
  pSettings_->endGroup();

  pSettings_->sync();

  return true;
}

