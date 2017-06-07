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

#include "utils.h"
#include "UserConfig.h"
#include "SystemConfig.h"
#include "VmMaskUserConfig.h"
#include "VmMaskCurrentConfig.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QSet>

VmMaskUserConfig::VmMaskUserConfig():
  name_("NOT_INITIALIZED"),
  vmName_("NOT_INITIALIZED"),
  fullName_("NOT_INITIALIZED"),
  description_("NOT_INITIALIZED"),
  vmMaskId_(-1),
  color_("NOT_INITIALIZED"),
  networkConnectionType_("NOT_INITIALIZED"),
  flash_(false),
  java_(false)
{
}

bool VmMaskUserConfig::initWithBaseDiskCapabilities(const QJsonObject& parBaseDiskCapabilities)
{
  // TODO: add error handling

  // get available fonts
  QJsonArray fontArray = parBaseDiskCapabilities["fonts"].toArray();
  for (const QJsonValue &font : fontArray)
    fontsInBaseDisk_.append(font.toString());

  // get available locales
  QJsonArray localeArray = parBaseDiskCapabilities["locales"].toArray();
  for (const QJsonValue &locale : localeArray)
    localesInBaseDisk_.append(locale.toString());

  // get available timezones
  QJsonArray timeZonesArray = parBaseDiskCapabilities["timezones"].toArray();
  for (const QJsonValue &timeZone : timeZonesArray)
    timeZonesInBaseDisk_.append(timeZone.toString());

  // get available browsers
  browsersInBaseDisk_ = parBaseDiskCapabilities["browser"].toObject().keys();

  // TODO: olaf: Why is this here? Delete?
  // check installed language pack (basedisk capabilities?)
  /*
  // Choose one Language
  browserLanguage = "en"; // init
  QStringList browserLanguages = parCurConfigVmMask->browserLanguages.split(',', QString::SkipEmptyParts);
  if (browserLanguages.size())
  {
    browserLanguage = browserLanguages[randombytes_uniform(browserLanguages.size())];
  }
  */

  return true;
}

bool VmMaskUserConfig::setConfigurationDefaultsAndCheckForErrors(const QJsonObject& parBaseDiskCapabilities)
{
  // Important remark: It's better that the startup failes, than we use inproper fingerprints

  if (!initWithBaseDiskCapabilities(parBaseDiskCapabilities))
    return false;

  // Validate if BaseDisk has enough items

  if (timeZonesInBaseDisk_.size() == 0)
  {
    IERR("no time zones are available");
    return false;
  }

  if (localesInBaseDisk_.size() == 0)
  {
    IERR("no locales are available");
    return false;
  }

  if (fontsInBaseDisk_.size() < 10)
  {
    IERR("too less fonts available in the BaseDisk");
    return false;
  }

  if (browsersInBaseDisk_.size() == 0)
  {
    IERR("no browsers are available");
    return false;
  }

  // Now interset the config with the items available in the BaseDisk

  if (timeZones_.count() == 0)
  {
    // ok if none configured -> then choose from all
    timeZones_ = timeZonesInBaseDisk_;
  }
  else
  {
    // Check if configured time zones are available
    QSet<QString> intersection = timeZones_.toSet().intersect(timeZonesInBaseDisk_.toSet());
    if(intersection.size() == 0)
    {
      IERR("No intersection between user defined time zones and available time zones found for " + name_);
      return false;
    }
  }

  if (locales_.count() == 0)
  {
    // ok if none configured -> then choose from all
    locales_ = localesInBaseDisk_;
  }
  else
  {
    // Check if configured locales are available
    QSet<QString> intersection = locales_.toSet().intersect(localesInBaseDisk_.toSet());
    if(intersection.size() == 0)
    {
      IERR("No intersection between user defined locales and available locales found for " + name_);
      return false;
    }
  }

  if (fonts_.count() == 0)
  {
    // ok if none configured -> then choose from all
    fonts_ = fontsInBaseDisk_;
  }
  else
  {
    // Check if configured fonts are available
    QSet<QString> intersection = fonts_.toSet().intersect(fontsInBaseDisk_.toSet());
    if(intersection.size() == 0)
    {
      IERR("No intersection between user defined fonts and available fonts found for " + name_);
      return false;
    }
  }

  if (browsers_.count() == 0)
  {
    // ok if none configured -> then choose from all
    browsers_ = browsersInBaseDisk_;
  }
  else
  {
    // Check if configured browsers are available
    QSet<QString> intersection = browsers_.toSet().intersect(browsersInBaseDisk_.toSet());
    if(intersection.size() == 0)
    {
      IERR("No intersection between user defined browsers and available browsers found for " + name_);
      return false;
    }
  }

  // If the user has configured less than 2 dns servers we choose defaults (from <https://freedns.zone>)
  // TODO: olaf: use in /etc/resolvconf/resolv.conf.d/tail
  if (dnsServers_.count() < 2)
  {
    dnsServers_.append("37.235.1.174");
    dnsServers_.append("37.235.1.177");
  }

  // if the user has not configured a browser Language, we set a default
  // TODO: set intl.accept_languages in about:config
  if (browserLanguages_.count() == 0)
  {
    browserLanguages_.append("en-US, en");
  }

  // If the user has configured no ip-address-providers servers we choose some defaults
  if (ipAddressProviders_.count() == 0)
  {
    ipAddressProviders_.append("http://ident.me");
    ipAddressProviders_.append("http://wtfismyip.com/text");
    ipAddressProviders_.append("http://icanhazip.com");
    ipAddressProviders_.append("http://checkip.amazonaws.com");
  }

  // TODO: olaf: check vpn config

  // TODO: olaf: Check for different colors, ...

  return true;
}

/// This function generates a new random fingerprint (returns NULL on error)
VmMaskCurrentConfig* VmMaskUserConfig::diceNewVmMaskConfig(VmMaskStaticConfig* parStaticConfig)
{
  VmMaskCurrentConfig* newConfig = new VmMaskCurrentConfig(parStaticConfig);

  // choose a random timezone
  newConfig->setTimeZone(timeZones_[randombytes_uniform(timeZones_.size())]);

  // choose number of random fonts to install (3-10)
  int numFontsToInstall = randombytes_uniform(8) + 3;
  if (numFontsToInstall > fontsInBaseDisk_.size())
    numFontsToInstall = fontsInBaseDisk_.size();

  // choose the fonts
  QSet<QString> fontsToInstallSet; // use a QSet because we need a list of unique fonts
  while(fontsToInstallSet.size() < numFontsToInstall)
  {
    fontsToInstallSet.insert(fontsInBaseDisk_[randombytes_uniform(fontsInBaseDisk_.size())]);
  }
  newConfig->setFonts(fontsToInstallSet.toList());

  // choose one locale
  newConfig->setLocale(locales_[randombytes_uniform(locales_.size())]);

  // choose 2 random dns servers
  QSet<QString> dnsServerSet;
  while ( dnsServerSet.size() < 2)
  {
    dnsServerSet.insert(dnsServers_[randombytes_uniform(dnsServers_.size())]);
  }
  newConfig->setDnsServers(dnsServerSet.toList());

  // choose one browser
  newConfig->setBrowser(browsers_[randombytes_uniform(browsers_.size())]);

  // copy ip address providers in different order
  QStringList ipAddressProvidersShuffled;
  QStringList ipAddressProvidersToShuffle;
  ipAddressProvidersToShuffle = ipAddressProviders_;
  while ( ipAddressProvidersToShuffle.size() > 0)
  {
    int elementNr = randombytes_uniform(ipAddressProvidersToShuffle.size());
    QString provider = ipAddressProviders_[elementNr];
    ipAddressProvidersShuffled.append(provider);
    ipAddressProvidersToShuffle.removeAt(elementNr);
  }
  newConfig->setIpAddressProviders(ipAddressProvidersShuffled);

  // Choose a unique display size (subtract some pixels)
  newConfig->setSubtractDisplayWidth(randombytes_uniform(25)+8); // 8-32
  newConfig->setSubtractDisplayHeight(randombytes_uniform(9)+8); // 8-16

  // some values are just copy&paste
  newConfig->setColor(color_);
  newConfig->setJava(java_);
  newConfig->setFlash(flash_);
  newConfig->setBrowserLanguages(browserLanguages_);
  newConfig->setScriptOnShutdown(scriptOnShutdown_);
  newConfig->setScriptOnStartup(scriptOnStartup_);
  newConfig->setThirdPartyCookies(thirdPartyCookies_);
  newConfig->setVpnConfig(vpnConfig_);

  return newConfig;
}

QStringList VmMaskUserConfig::getLocales() const
{
  return locales_;
}

void VmMaskUserConfig::setLocales(const QStringList& locales)
{
  locales_ = locales;
}

QStringList VmMaskUserConfig::getBrowserLanguages() const
{
  return browserLanguages_;
}

void VmMaskUserConfig::setBrowserLanguages(const QStringList& parBrowserLanguages)
{
  browserLanguages_ = parBrowserLanguages;
}


QStringList VmMaskUserConfig::getFonts() const
{
  return fonts_;
}

QString VmMaskUserConfig::getScriptOnStartup() const
{
  return scriptOnStartup_;
}

void VmMaskUserConfig::setScriptOnStartup(const QString& scriptOnStartup)
{
  scriptOnStartup_ = scriptOnStartup;
}

QString VmMaskUserConfig::getScriptOnShutdown() const
{
  return scriptOnShutdown_;
}

void VmMaskUserConfig::setScriptOnShutdown(const QString& scriptOnShutdown)
{
  scriptOnShutdown_ = scriptOnShutdown;
}

bool VmMaskUserConfig::getJava() const
{
  return java_;
}

void VmMaskUserConfig::setJava(bool parJava)
{
  java_ = parJava;
}

bool VmMaskUserConfig::getFlash() const
{
  return flash_;
}

void VmMaskUserConfig::setFlash(bool parFlash)
{
  flash_ = parFlash;
}

QString VmMaskUserConfig::getThirdPartyCookies() const
{
  return thirdPartyCookies_;
}

void VmMaskUserConfig::setThirdPartyCookies(const QString& thirdPartyCookies)
{
  thirdPartyCookies_ = thirdPartyCookies;
}

QStringList VmMaskUserConfig::getBrowsers() const
{
  return browsers_;
}

void VmMaskUserConfig::setBrowsers(const QStringList& parBrowsers)
{
  browsers_.clear();
  // Browsers needed to converted to lower case because that's the name inside the BaseDisk
  for (QString val : parBrowsers)
  {
    browsers_.append(val.toLower());
  }
}

QStringList VmMaskUserConfig::getDnsServers() const
{
  return dnsServers_;
}

void VmMaskUserConfig::setDnsServers(const QStringList& parDnsServers)
{
  dnsServers_ = parDnsServers;
}

QStringList VmMaskUserConfig::getIpAddressProviders() const
{
  return ipAddressProviders_;
}

void VmMaskUserConfig::setIpAddressProviders(const QStringList& ipAddressProviders)
{
  ipAddressProviders_ = ipAddressProviders;
}

QString VmMaskUserConfig::getNetworkConnectionType() const
{
  return networkConnectionType_;
}

void VmMaskUserConfig::setNetworkConnectionType(const QString& networkConnectionType)
{
  networkConnectionType_ = networkConnectionType;
}

const VpnConfig* VmMaskUserConfig::getVpnConfig() const
{
  return &vpnConfig_;
}

void VmMaskUserConfig::setVpnConfig(const VpnConfig& value)
{
  vpnConfig_ = value; // create a copy
}

QColor VmMaskUserConfig::getColor() const
{
  return color_;
}

void VmMaskUserConfig::setColor(const QColor& color)
{
  color_ = color;
}

int VmMaskUserConfig::getVmMaskId() const
{
  return vmMaskId_;
}

void VmMaskUserConfig::setVmMaskId(int parIndexOfRadioButtons)
{
  vmMaskId_ = parIndexOfRadioButtons;
}

QString VmMaskUserConfig::getName() const
{
  return name_;
}

void VmMaskUserConfig::setName(const QString& parName)
{
  name_ = parName;
  vmName_ = constPmVmMaskPrefix + parName;
}

QString VmMaskUserConfig::getVmName() const
{
  return vmName_;
}

void VmMaskUserConfig::setVmName(const QString& parVmName)
{
  vmName_ = parVmName;
}

QString VmMaskUserConfig::getFullName() const
{
  return fullName_;
}

void VmMaskUserConfig::setFullName(const QString& parFullName)
{
  fullName_ = parFullName;
}

QString VmMaskUserConfig::getDescription() const
{
  return description_;
}

void VmMaskUserConfig::setDescription(const QString& parDescription)
{
  description_ = parDescription;
}

