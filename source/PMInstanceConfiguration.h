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

#include "UserConfig.h"

class SystemConfig;

class PMInstanceConfiguration
{
  public:
    explicit PMInstanceConfiguration(ConfigUseCase *parCurConfigUseCase, UserConfig *parWholeConfig, SystemConfig *parSystemConfig_);

    unsigned short rdpPort;
    QString hostName;
    QString name;
    QString fullName;
    QString language;
    QString networkConnection;
    ConfigVPN usedVPNConfig;
    QString java;
    QString flash;
    QString thirdPartyCookies;
    QString browser;
    QString scriptOnStartup;
    QString scriptOnShutdown;
    SystemConfig *systemConfig;
};
