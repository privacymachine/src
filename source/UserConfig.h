﻿/*==============================================================================
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

#include <QString>
#include <QVariant>

struct ConfigUseCase
{
    QString name; // from Name of Section
    QString fullName;
    QString networkConnection;
    QString languages; // separated with ','
    QString java;
    QString flash;
    QString thirdPartyCookies;
    QString browsers;
    QString scriptOnStartup;
    QString scriptOnShutdown;
};

struct ConfigVPN
{
    QString name; // from Name of Section
    QString configFiles;
    QString vpnType;
    QString login;
    QString password;
};


class UserConfig
{
  public:
    UserConfig();
    virtual ~UserConfig();

    QList<ConfigUseCase*>& getConfiguredUseCases();
    QList<ConfigVPN*>& getConfiguredVPNs();

    bool readFromFile();

  private:
    QString convertIniValueToString(QVariant parVar);
    QList<ConfigUseCase*> configuredUseCases_;
    QList<ConfigVPN*> configuredVPNs_;
};
