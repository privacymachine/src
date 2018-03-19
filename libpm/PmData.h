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

#ifndef PMDATA_H
#define PMDATA_H

#include <QString>
#include <QDir>

// Singelton used storage of global used Data (Not Threadsafe)
// Singelton-Pattern from http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class PmData
{
  public:
    static PmData& getInstance()
    {
      static PmData instance_; // Guaranteed to be destroyed. Instantiated on first use.
      return instance_;
    }

    PmData(PmData const&)             = delete; // not allowed for Singeltons (feature of C++11)
    PmData(PmData&&)                  = delete; // not allowed for Singeltons (feature of C++11)
    PmData& operator=(PmData const&)  = delete; // not allowed for Singeltons (feature of C++11)
    PmData& operator=(PmData&&)       = delete; // not allowed for Singeltons (feature of C++11)


    /// getter / setter

    /// The install dir path is determined of the path of the running executeable
    void setInstallDirPath(QString path) { installDirPath_ = path; }
    QString getInstallDirPath() const { return installDirPath_; }

    QString getVBoxCommand() { return vboxCommand_; }

    /// Sets the configDir_ of the current instance and calculates the pmConfigId_
    void setPmConfigDirPath(QString configDir) { configDirPath_ = configDir; }
    QDir getPmConfigDir() const { return QDir{configDirPath_}; }
    QString getPmConfigDirPath() const { return configDirPath_; }

    void setPmServerIp(QString pmServerIp) { pmServerIp_ = pmServerIp; }
    QString getPmServerIp() const { return pmServerIp_; }

    void setBaseDiskRootUser(QString baseDiskRootUser) { baseDiskRootUser_ = baseDiskRootUser; }
    QString getBaseDiskRootUser() const { return baseDiskRootUser_; }

    void setBaseDiskRootUserPassword(QString baseDiskRootUserPassword) { baseDiskRootUserPassword_ = baseDiskRootUserPassword; }
    QString getBaseDiskRootUserPassword() const { return baseDiskRootUserPassword_; }

    void setBaseDiskLiveUser(QString baseDiskLiveUser) { baseDiskLiveUser_ = baseDiskLiveUser; }
    QString getBaseDiskLiveUser() const { return baseDiskLiveUser_; }

    void setBaseDiskLiveUserPassword(QString baseDiskLiveUserPassword) { baseDiskLiveUserPassword_ = baseDiskLiveUserPassword; }
    QString getBaseDiskLiveUserPassword() const { return baseDiskLiveUserPassword_; }

    void setVmMaskPrefix(QString vmMaskPrefix) { vmMaskPrefix_ = vmMaskPrefix; }
    QString getVmMaskPrefix() const { return vmMaskPrefix_; }

    void setPmVmMaskPrefix(QString vmPmMaskPrefix) { vmPmMaskPrefix_ = vmPmMaskPrefix; }
    QString getPmVmMaskPrefix() const { return vmPmMaskPrefix_; }

    void setVpnPrefix(QString vpnPrefix) { vpnPrefix_ = vpnPrefix; }
    QString getVpnPrefix() const { return vpnPrefix_; }

    void setVmSnapshotName(QString vmSnapshotName) { vmSnapshotName_ = vmSnapshotName; }
    QString getVmSnapshotName() const { return vmSnapshotName_; }

    void setPmUserConfigFilePath(QString pmUserConfigFilePath) { pmUserConfigFilePath_ = pmUserConfigFilePath; }
    QString getPmUserConfigFilePath() const { return pmUserConfigFilePath_; }

    void setPmInternalConfigFilePath(QString pmInternalConfigFilePath) { pmInternalConfigFilePath_ = pmInternalConfigFilePath; }
    QString getPmInternalConfigFilePath() const { return pmInternalConfigFilePath_; }

    bool completelyFilled();

    void log();

  private:
    PmData();
    QString determineVBoxCommand();

    QString installDirPath_ = "NO VALID ENTRY";
    QString vboxCommand_ = "NO VALID ENTRY";
    QString pmServerIp_ = "NO VALID ENTRY";
    QString baseDiskRootUser_ = "NO VALID ENTRY";
    QString baseDiskRootUserPassword_ = "NO VALID ENTRY";
    QString baseDiskLiveUser_ = "NO VALID ENTRY";
    QString baseDiskLiveUserPassword_ = "NO VALID ENTRY";
    QString vmMaskPrefix_ = "NO VALID ENTRY";
    QString vmPmMaskPrefix_ = "NO VALID ENTRY";
    QString vpnPrefix_ = "NO VALID ENTRY";
    QString vmSnapshotName_ = "NO VALID ENTRY";
    QString pmUserConfigFilePath_ = "NO VALID ENTRY";
    QString pmInternalConfigFilePath_ = "NO VALID ENTRY";
    QString configDirPath_ = "NO VALID ENTRY";
};

#endif // PMDATA_H
