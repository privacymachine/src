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

#include "PMInstance.h"
#include "PMInstanceConfiguration.h"
#include "PMCommand.h"
#include "VmInitVpn.h"

#include <QObject>
#include <QList>
#include <QJsonObject>

// forward declarations
class UserConfig;
class SystemConfig;


/// Maintains a database of Use Cases, knows which are configured and which are running at the
/// moment. Think "controller" in MVC.
class PMManager
{
  public:
    explicit PMManager();
    virtual ~PMManager();

    bool init_1(QString parPmInstallDir, QString parVboxDefaultMachineFolder); // read config
    bool init_2();
    bool initAllVmMasks();
    bool createCommandsUpdateAllVmMasks(QList<PMCommand*>& parCommandsList);
    bool createCommandsStart(QString parVmMaskName, QList<PMCommand*>& parCommandsList);
    bool createCommandsClose(PMInstance* parPmInstance, QList<PMCommand*>& parCommandsList);
    bool createCommandsCloseAllVms( QList<PMCommand*>& parCommandsList );
    bool createCommandsCleanupVirtualBoxVms( QList<PMCommand*>& parCommandsList );
    bool isFirstStart();
    QString baseDiskWithPath();
    QString getBaseDiskDirectoryPath();

    QList<ConfigVmMask*>& getConfiguredVmMasks();
    QList<PMInstance*>& getInstances() { return pm_; }

    QString getPmInstallDir() { return pmInstallDir_; }
    QString getPmUserConfigDir()
    {
      return pmUserConfigDir_;
    }

    ConfigUpdate getUpdateConfig() { return configUser_->getUpdateConfig(); }

    /// Verifies that for all VM Masks there is the corresponding VM set up in virtual box.
    /// \return \c true if VMs exist for all VM Masks, \c false otherwise.
    bool vmsExist();

    /// checks if the user configuration changed in a way so we need to regenerate the vmMasks
    bool vmMaskRegenerationNecessary();

    /// called after VMMask generation, updates sytemconfig with new instance names
    bool saveConfiguredVMMasks();

  private:
    PMInstance* createPMInstance(ConfigVmMask* parCurVmMask);

    bool createCommandsCloneStartCreateSnapshot(PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList);

    /// Appends commands that perform browser-specific configuration, e.g. obfuscating firefox plugins.
    void appendCommandsConfigureBrowser( PMInstance *pPmInstance, QList<PMCommand*>& parCommandsList );

    void appendCommandsToStartBrowserService(PMInstance *pPmInstance, QList<PMCommand*>& parCommandsList );

    /// TCP-Ports needs to be unique we need a starting port of which we assign free ports bases on a configured VmMask-Index
    unsigned short firstFreeLocalPort_;

    QList<PMInstance*>                  pm_;
    QList<PMInstanceConfiguration*>     pmConfigs;

    /// Eventually, this holds the contents of PrivacyMachine.ini
    /// All these parameters should be configurable by the end user.
    UserConfig*                         configUser_;

    /// Eventually, this holds the contents of PrivacyMachineInternals.ini
    /// All these parameters shall be used for internal persistence, e.g. machine performance
    /// information, which might be useful for starting virtual machines on different hosts.
    SystemConfig*                       configSystem_;

    QList<QString>                      runningInstances_;

    /// Path to the executeables (used in config as {INSTALL_DIR})
    QString                             pmInstallDir_;

    /// Path to the configuration dir of the user  (used in config as {USER_CONFIG_DIR})
    QString                             pmUserConfigDir_;

    /// This holds the current base-disk_capabilities and is used for all pre known base-disk related information
    QJsonObject                         baseDiskCapabilities_;

    /// Path the folder where VirtualBox creates the VirtualMachines (VmMasks)
    QString                             vboxDefaultMachineFolder_;

    /// True on first start
    bool                                firstStart_;
};

