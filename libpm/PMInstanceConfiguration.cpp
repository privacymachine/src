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

#include "utils.h"
#include "UserConfig.h"
#include "SystemConfig.h"
#include "PMInstanceConfiguration.h"


#include <QJsonArray>
#include <QJsonValue>
#include <QSet>



PMInstanceConfiguration::PMInstanceConfiguration(ConfigVmMask *parCurConfigVmMask,
                                                 UserConfig *parWholeConfig,
                                                 SystemConfig *parSystemConfig,
                                                 const QJsonObject& parBaseDiskCapabilities)
{
  systemConfig = parSystemConfig;
  name = parCurConfigVmMask->name;
  vmName = parCurConfigVmMask->vmName;
  vmMaskCreated = false;
  fullName = parCurConfigVmMask->fullName;
  color = parCurConfigVmMask->color;
  localeListUser = parCurConfigVmMask->localeList;
  browserListUser = parCurConfigVmMask->browserList;

  // get avaiable fonts
  QJsonArray fontArray = parBaseDiskCapabilities["fonts"].toArray();
  foreach (const QJsonValue &font, fontArray)
    fontList_.append(font.toString());

  // get avaiable locales
  QJsonArray localeArray = parBaseDiskCapabilities["locales"].toArray();
  foreach (const QJsonValue &locale, localeArray)
    localeList_.append(locale.toString());

  // get avaiable timezones
  QJsonArray timeZonesArray = parBaseDiskCapabilities["timezones"].toArray();
  foreach (const QJsonValue &timeZone, timeZonesArray)
    timeZoneList_.append(timeZone.toString());

  // get avaiable browsers
  browserList_ = parBaseDiskCapabilities["browser"].toObject().keys();

  // setup DNS-Server-List
  dnsServerList_ = parCurConfigVmMask->dnsServers.split(',', QString::SkipEmptyParts);

  // Why is this here? Delete?
  // Choose one Language
  browserLanguage = "en"; // init
  QStringList browserLanguages = parCurConfigVmMask->browserLanguages.split(',', QString::SkipEmptyParts);
  if (browserLanguages.size())
  {
    int pos = randInt(0,browserLanguages.size()-1);
    browserLanguage = browserLanguages[pos];
  }

  networkConnection = parCurConfigVmMask->networkConnection;

  usedVPNConfig.name = ""; // init as 'not used'
  usedVPNConfig.vpnType = "";
  QString vpnPrefix = "VPN_";
  if (parCurConfigVmMask->networkConnection.startsWith(vpnPrefix))
  {
    QString vpnName = parCurConfigVmMask->networkConnection.mid(vpnPrefix.length());
    foreach(ConfigVPN* curVPNConfig, parWholeConfig->getConfiguredVPNs())
    {
      if (curVPNConfig->name == vpnName)
      {
        usedVPNConfig = *curVPNConfig; // create a copy of the struct
      }
    }
  }
  ipAddressProviders = parCurConfigVmMask->ipAddressProviders.split(',', QString::SkipEmptyParts);
  java = parCurConfigVmMask->java;
  flash = parCurConfigVmMask->flash;
  thirdPartyCookies = parCurConfigVmMask->thirdPartyCookies;

  // Initialize random seed (based on current time and pointer)
  pm_srand(this);

  // Choose a unique display size (subtract some pixels)
  subtractDisplayWidth = randInt(8,32);
  subtractDisplayHeight = randInt(8,16);

  scriptOnStartup = parCurConfigVmMask->scriptOnStartup;
  scriptOnShutdown = parCurConfigVmMask->scriptOnShutdown;
}



void PMInstanceConfiguration::init()
{
  // Initialize random seed (based on current time and pointer)
  pm_srand(this);

  // choose random timezone
  timeZone = timeZoneList_[randInt(0,timeZoneList_.size()-1)];

  // choose random fonts to install
  int numFontsToInstall = randInt((fontList_.size()<3) ? fontList_.size()-1 : 3, (fontList_.size()<15) ? fontList_.size()-1 : 15);
  QSet<QString> fontsToInstall;
  while(fontsToInstall.size() < numFontsToInstall)
    fontsToInstall.insert(fontList_[randInt(0,fontList_.size()-1)]);
  fonts = fontsToInstall.toList();

  // choose a random locale from the intersection from user choosen locales and avaiable locales
  QSet<QString> localeIntersection = localeListUser.toSet().intersect(localeList_.toSet());
  if(localeIntersection.size() > 0)
    locale = localeIntersection.toList()[randInt(0,localeIntersection.size()-1)];
  else
  {
    IWARN("No intersection between user defined locales and avaiable locales found for " + name);
    locale = localeList_[randInt(0,localeList_.size()-1)];
  }

  // choose up to 2 random dns servers from the user given list
  QSet<QString> dnsServerSet;
  while ( dnsServerSet.size()<2 && dnsServerSet.size()<browserList_.size() )
    dnsServerSet.insert(dnsServerList_[randInt(0,dnsServerList_.size()-1)]);
  dnsServers = dnsServerSet.toList();

  // choose a random browser from the intersection from user choosen browsers and avaiable browsers
  QSet<QString> browserIntersection = browserListUser.toSet().intersect(browserList_.toSet());
  if(browserIntersection.size() > 0)
    browser = browserIntersection.toList()[randInt(0,browserIntersection.size()-1)];
  else
  {
    IWARN("No intersection between user defined browsers and avaiable browsers found for " + name);
    browser = browserList_[randInt(0,browserList_.size()-1)];
  }
}


QString PMInstanceConfiguration::toString()
{
  return "\"" + fullName + "\" - " 
      + QObject::tr("Locale") + ": " + locale + ", "
      + QObject::tr("Time Zone") + ": " + timeZone + ", "
      + QObject::tr("Additional Fonts") + ": " + QString::number(fonts.size()) + "/" +QString::number(fontList_.size());

}
