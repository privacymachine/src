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

#include "VmMaskData.h"
#include "PmCommand.h"
#include "PmVpnHelper.h"

#include <QObject>
#include <QList>
#include <QJsonObject>
#include "PmVersion.h"


// forward declarations
class UserConfig;
class SystemConfig;
class QUrl;


///


/**
 * @brief Maintains a database of Use Cases, knows which are configured and which are running at the moment.
 * Think "controller" in MVC.
 */
class PmManager
{
  public:
    explicit PmManager();
    virtual ~PmManager();

    bool initConfiguration(const QString& parPmInstallDir, const QString& parVboxDefaultMachineFolder);
    bool readAndValidateConfiguration();

    bool initAllVmMaskData();
    bool createCommandsToCreateAllVmMasks(QList<PmCommand*>& parCommandsList);

    /// \brief Create a List of Commands which are needed to start a already prepared VmMask (Snapshot)
    /// \param parVmMaskId [in] Id of the VmMask
    /// \param parCommandsList [out] contains all generated Commands
    /// \return false on error
    bool createCommandsToStartVmMask(int parVmMaskId, QList<PmCommand*>& parCommandsList);

    bool createCommandsToCloseVmMask(QString parVmName, QString parVmMaskFullName,QList<PmCommand*>& parCommandsList);
    bool createCommandsToCloseAllVmMasks( QList<PmCommand*>& parCommandsList );
    bool createCommandsToCleanupAllVirtualMachines( QList<PmCommand*>& parCommandsList );
    bool isFirstStart() { return firstStart_; }

    QString baseDiskWithPath();
    QString getBaseDiskDirectoryPath();
    PmVersion getBaseDiskVersion();

    const QList<VmMaskData*>& getVmMaskData() { return vmMaskData_; }

    QUrl getAppcastUrl(){ return configUser_->getAppcastUrl(); }

    QString getPmInstallDir() { return pmInstallDir_; }

    QDir getPmConfigDir() { return pmConfigDir_; }

    SystemConfig* getSystemConfig() { return configSystem_; }



    /// Verifies that for all VM Masks there is the corresponding VM set up in virtual box.
    /// \return \c true if VMs exist for all VM Masks, \c false otherwise.
    bool allVmMasksExist();

    /// checks if the user configuration changed in a way so we need to regenerate the vmMasks
    bool vmMaskRegenerationNecessary();

    /// called after VMMask generation, updates sytemconfig with new instance names
    bool saveConfiguredVmMasks();

    /// true if any BaseDisk is available
    bool isBaseDiskAvailable();

    bool isBaseDiskConfigValid() { return baseDiskConfigIsValid_; }

  private:

    /**
     * @brief XX createCommandsToCreateVmMask
     * @param x parVmMask
     * @param parCommandsList
     * @return
     */

    /*!
     * \brief createCommandsToCreateVmMask
     * \param parVmMask
     * \param parCommandsList
     * \return
     */
    bool createCommandsToCreateVmMask(VmMaskData* parVmMask, QList<PmCommand*>& parCommandsList);

    /// Appends commands that perform browser-specific configuration, e.g. obfuscating firefox plugins.
    void addCommandsToConfigureBrowser(QSharedPointer<VmMaskInstance>& parVmMaskInstance, QList<PmCommand*>& parCommandsList);

    void addCommandsToStartBrowserService(QSharedPointer<VmMaskInstance>& parVmMaskInstance, QList<PmCommand*>& parCommandsList );

    /// TCP-Ports needs to be unique we need a starting port of which we assign free ports bases on a configured VmMask-Index
    unsigned short firstFreeLocalPort_;

    QList<VmMaskData*>                  vmMaskData_;

    /// Eventually, this holds the contents of PrivacyMachine.ini
    /// All these parameters should be configurable by the end user.
    UserConfig*                         configUser_;

    /// Eventually, this holds the contents of PrivacyMachineInternals.ini
    /// All these parameters shall be used for internal persistence, e.g. machine performance
    /// information, which might be useful for starting virtual machines on different hosts.
    SystemConfig*                       configSystem_;

    /// Path to the executeables (used in config as {INSTALL_DIR})
    QString                             pmInstallDir_;

    /// Path to the configuration files of the PrivacyMachine
    QDir                                pmConfigDir_;

    /// This holds the current BaseDiskCapabilities.json and is used for all pre known BaseDisk related information
    QJsonObject                         baseDiskCapabilities_;

    /// Path the folder where VirtualBox creates the VirtualMachines (VmMasks)
    QString                             vboxDefaultMachineFolder_;

    /// True on first start
    bool                                firstStart_;

    /// True if we have a valid: UserConfig, SystemConfig and BaseDisk,
    bool                                baseDiskConfigIsValid_;

};

