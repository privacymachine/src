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

#pragma once

#include "PMInstance.h"
#include "PMInstanceConfiguration.h"
#include "PMCommand.h"

#include <QObject>
#include <QList>

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

    bool init(QString pmInstallPath); // read config
    bool initAllVmMasks();
    bool createCommandsUpdateAllVmMasks(QList<PMCommand*>& parCommandsList);
    bool createCommandsStart(QString parVmMaskName, QList<PMCommand*>& parCommandsList);
    bool createCommandsClose(PMInstance* parPmInstance, QList<PMCommand*>& parCommandsList);
    bool createCommandsCloseAllVms( QList<PMCommand*>& parCommandsList );
    QList<ConfigVmMask*>& getConfiguredVmMasks();
    QList<PMInstance*>& getInstances() { return pm_; }
    QString getPmInstallPath() { return pmInstallPath_; }

    /// Verifies that for all VM Masks there is the corresponding VM set up in virtual box.
    /// \return \c true if VMs exist for all VM Masks, \c false otherwise.
    bool vmsExist();

  private:
    PMInstance* createPMInstance(ConfigVmMask* parCurVmMask);
    bool createCommandsCloneStartCreateSnapshot(PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList);

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

    /// Path to the executeables
    QString                             pmInstallPath_;
};

