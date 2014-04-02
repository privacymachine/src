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

#include "utils.h"
#include "UserConfig.h"
#include "SystemConfig.h"
#include "PMInstanceConfiguration.h"

#include <QStringList>

PMInstanceConfiguration::PMInstanceConfiguration(ConfigUseCase *parCurConfigUseCase,
                                                 UserConfig *parWholeConfig,
                                                 SystemConfig *parSystemConfig)
{
  systemConfig=parSystemConfig;

  QString vpnPrefix = "VPN_";

  name = parCurConfigUseCase->name;
  fullName = parCurConfigUseCase->fullName;

  // Choose one Language
  language = "en"; // init
  QStringList languages = parCurConfigUseCase->languages.split(',', QString::SkipEmptyParts);
  if (languages.count())
  {
    int pos = randInt(0,languages.count()-1);
    language = languages[pos];
  }

  networkConnection = parCurConfigUseCase->networkConnection;

  usedVPNConfig.name = ""; // init as 'not used'
  usedVPNConfig.vpnType = "";
  if (parCurConfigUseCase->networkConnection.startsWith(vpnPrefix))
  {
    QString vpnName = parCurConfigUseCase->networkConnection.mid(vpnPrefix.length());
    foreach(ConfigVPN* curVPNConfig, parWholeConfig->getConfiguredVPNs())
    {
      if (curVPNConfig->name == vpnName)
      {
        usedVPNConfig = *curVPNConfig; // create a copy of the struct
      }
    }
  }
  java = parCurConfigUseCase->java;
  flash = parCurConfigUseCase->flash;
  thirdPartyCookies = parCurConfigUseCase->thirdPartyCookies;

  // Choose one Browser
  browser = "Firefox"; // init
  QStringList browsers = parCurConfigUseCase->browsers.split(',', QString::SkipEmptyParts);
  if (browsers.count())
  {
    int pos = randInt(0,browsers.count()-1);
    browser = browsers[pos];
  }

  scriptOnStartup = parCurConfigUseCase->scriptOnStartup;
  scriptOnShutdown = parCurConfigUseCase->scriptOnShutdown;
}
