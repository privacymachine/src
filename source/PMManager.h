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

#pragma once

#include "PMInstance.h"
#include "PMInstanceConfiguration.h"
#include "PMCommand.h"

#include <QObject>
#include <QList>

// forward declarations
class UserConfig;
class SystemConfig;

class PMManager
{
  public:
    explicit PMManager();
    virtual ~PMManager();

    bool init(); // read config
    bool initAllUsecases();
    bool createCommandsForUpdateAllUsecases(QList<PMCommand*>& parCommandsList);
    bool createCommandsToStartInstance(QString parInstanceName, QList<PMCommand*>& parCommandsList);
    bool createCommandsToCloseMachine(QString instanceName, QList<PMCommand*>& parCommandsList);
    QList<QString> getUseCaseNames();
    QList<PMInstance*>& getInstances() { return pm_; }

  private:
    PMInstance* createPMInst(ConfigUseCase* parCurUseCase);
    bool createCommandsForOneInstance(PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList);

    unsigned short nextFreeRDPPort_;

    QList<PMInstance*>                  pm_;
    QList<PMInstanceConfiguration*>     pmConfigs;
    UserConfig*                         configUser_;
    SystemConfig*                       configSystem_;
    QList<QString>                      runningInstances_;
};

