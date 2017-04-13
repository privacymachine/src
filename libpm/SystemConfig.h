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
#include <QDateTime>
#include "PmVersion.h"

class QSettings;

class SystemConfig
{
  public:
    SystemConfig(QString iniFile);
    virtual ~SystemConfig();

    void readFromFileOrSetDefaults();
    bool write();

    // getter / setter

    // Path to the BaseDisk with slash (i.e. /opt/privacymachine/BaseDisk/)
    QString getBaseDiskPath() { return baseDiskPath_; }
    void setBaseDiskPath(QString baseDiskPath) { baseDiskPath_ = baseDiskPath; }

    // Current BaseDisk name (i.e. 'BaseDisk_1' for the files BaseDisk_1.vmdk, BaseDisk_1_flat.vmdk, BaseDisk_1_capabilities.json)
    QString getBaseDiskName() { return baseDiskName_; }
    void setBaseDiskName(QString baseDiskName) { baseDiskName_ = baseDiskName; }

    // Current BaseDisk Version (i.e. 0.10.1.0 for the first BaseDisk of beta2 release)
    PmVersion getBaseDiskVersion() { return baseDiskVersion_; }
    void setBaseDiskVersion(PmVersion baseDiskVersion) { baseDiskVersion_ = baseDiskVersion; }
    void setBaseDiskVersion(QString baseDiskVersion) { baseDiskVersion_ = PmVersion::fromString(baseDiskVersion); }

    // Path to the Binary with slash (i.e. /opt/privacymachine/)
    QString getBinaryPath() { return binaryPath_; }
    void setBinaryPath(QString binaryPath) { binaryPath_ = binaryPath; }

    // not neccesarry? Current release name (i.e. 'PrivacyMachine beta2' )
    QString getBinaryName() { return binaryName_; }
    void setBinaryName(QString binaryName) { binaryName_ = binaryName; }

    // Current Binary Version (i.e. 0.10.1.0 for the first Binary of beta2 release)
    PmVersion getBinaryVersion() { return binaryVersion_; }
    void setBinaryVersion(PmVersion binaryVersion) { binaryVersion_ = binaryVersion; }
    void setBinaryVersion(QString binaryVersion) { binaryVersion_ = PmVersion::fromString(binaryVersion); }

    // Path to the Config with slash (i.e. /home/max/.config/privacymachine/)
    QString getConfigPath() { return configPath_; }
    void setConfigPath(QString configPath) { configPath_ = configPath; }

    // not neccesarry? Current Config name (i.e. 'Config_1' )
    QString getConfigName() { return configName_; }
    void setConfigName(QString configName) { configName_ = configName; }

    // Current Config Version (i.e. 0.10.1.0 for the first Config of beta2 release)
    PmVersion getConfigVersion() { return configVersion_; }
    void setConfigVersion(PmVersion configVersion) { configVersion_ = configVersion; }
    void setConfigVersion(QString configVersion) { configVersion_ = PmVersion::fromString(configVersion); }

    // Time in Seconds on cold boot untill screens shows some X-releated Display
    int getMachineBootUpTime() { return machineBootUpTime_; }
    void setMachineBootUpTime(int time) { machineBootUpTime_ = time; }

    // Time in Seconds on cold boot untill a special prcocess(daemon) is started up
    int getMachineServiceStartupTime()  { return machineServiceStartupTime_; }
    void setMachineServiceStartupTime(int time) { machineServiceStartupTime_ = time; }

    // Time in Seconds to restore a running Snapshot
    int getMachineRestoreTime()  { return machineRestoreTime_; }
    void setMachineRestoreTime(int time) { machineRestoreTime_ = time; }

    int getWaitTimeAfterCreateSnapshot() { return waitTimeAfterCreateSnapshot_; }
    void setWaitTimeAfterCreateSnapshot(int time) { waitTimeAfterCreateSnapshot_ = time; }

    int getWaitTimeAfterPowerOff() { return waitTimeAfterPowerOff_; }
    void setWaitTimeAfterPowerOff(int time) { waitTimeAfterPowerOff_ = time; }

    int getCopyScriptsPerSshTime() { return copyScriptsPerSshTime_; }
    void setCopyScriptsPerSshTime(int time) { copyScriptsPerSshTime_ = time; }

    QStringList getConfiguredVmMaskNames() { return configuredVmMaskNames_; }
    void setConfiguredVmMaskNames(QStringList vmMasks)  {configuredVmMaskNames_ = vmMasks; }

  private:

    QString iniFile_;
    int machineBootUpTime_;
    int machineServiceStartupTime_;
    int machineRestoreTime_;
    int waitTimeAfterCreateSnapshot_;
    int waitTimeAfterPowerOff_;
    int copyScriptsPerSshTime_;
    QSettings* pSettings_;
    QString baseDiskPath_;
    PmVersion baseDiskVersion_;
    QString baseDiskName_;
    QString configPath_;
    PmVersion configVersion_;
    QString configName_;
    QString binaryPath_;
    PmVersion binaryVersion_;
    QString binaryName_;


    QStringList configuredVmMaskNames_;
};
