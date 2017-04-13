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

#include <QString>
#include <QStringList>
#include <QDateTime>

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

    // Current BaseDisk name (i.e. 'BaseDisk_0' for the files BaseDisk_0.vmdk, BaseDisk_0_flat.vmdk, BaseDisk_0_capabilities.json)
    QString getBaseDiskName() { return baseDiskName_; }
    void setBaseDiskName(QString baseDiskName) { baseDiskName_ = baseDiskName; }

    // Current BaseDisk name (i.e. 'BaseDisk_0' for the files BaseDisk_0.vmdk, BaseDisk_0_flat.vmdk, BaseDisk_0_capabilities.json)
    QString getBaseDiskVersion() { return baseDiskVersion_; }
    void setBaseDiskVersion(QString baseDiskVersion) { baseDiskVersion_ = baseDiskVersion; }

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

    QDateTime getLastUpdateTime() { return lastUpdateTime_; }
    void setLastUpdateTime(QDateTime time) { lastUpdateTime_ = time; }

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
    QDateTime lastUpdateTime_;
    QSettings* pSettings_;
    QString baseDiskPath_;
    QString baseDiskVersion_;
    QString baseDiskName_;
    QStringList configuredVmMaskNames_;
};
