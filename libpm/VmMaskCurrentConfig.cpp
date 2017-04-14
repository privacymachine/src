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

#include "VmMaskCurrentConfig.h"

VmMaskCurrentConfig::VmMaskCurrentConfig(VmMaskStaticConfig* parStaticConfig)
{
  staticConfig_ = new VmMaskStaticConfig(parStaticConfig);
}

QString VmMaskCurrentConfig::toString()
{
  return "\"" + getFullName() + "\" - "
      + QObject::tr("Locale") + ": " + locale_ + ", "
      + QObject::tr("Time Zone") + ": " + getTimeZone() + ", "
      + QObject::tr("Additional Fonts") + ": " + QString::number(getFonts().size()) + "/ 4242"; // TODO: bernhard: Get Basedisk-Font-Count

}

VpnConfig VmMaskCurrentConfig::getVpnConfig() const
{
  return vpnConfig_;
}

void VmMaskCurrentConfig::setVpnConfig(const VpnConfig& value)
{
  vpnConfig_ = value;
}

QString VmMaskCurrentConfig::getThirdPartyCookies() const
{
  return thirdPartyCookies_;
}

void VmMaskCurrentConfig::setThirdPartyCookies(const QString& value)
{
  thirdPartyCookies_ = value;
}

unsigned short VmMaskCurrentConfig::getSubtractDisplayWidth() const
{
  return subtractDisplayWidth_;
}

void VmMaskCurrentConfig::setSubtractDisplayWidth(unsigned short value)
{
  subtractDisplayWidth_ = value;
}

unsigned short VmMaskCurrentConfig::getSubtractDisplayHeight() const
{
  return subtractDisplayHeight_;
}

void VmMaskCurrentConfig::setSubtractDisplayHeight(unsigned short value)
{
  subtractDisplayHeight_ = value;
}

QString VmMaskCurrentConfig::getScriptOnStartup() const
{
  return scriptOnStartup_;
}

void VmMaskCurrentConfig::setScriptOnStartup(const QString& value)
{
  scriptOnStartup_ = value;
}

QString VmMaskCurrentConfig::getScriptOnShutdown() const
{
  return scriptOnShutdown_;
}

void VmMaskCurrentConfig::setScriptOnShutdown(const QString& value)
{
  scriptOnShutdown_ = value;
}

QStringList VmMaskCurrentConfig::getIpAddressProviders() const
{
  return ipAddressProviders_;
}

void VmMaskCurrentConfig::setIpAddressProviders(const QStringList& value)
{
  ipAddressProviders_ = value;
}

QStringList VmMaskCurrentConfig::getBrowserLanguages() const
{
  return browserLanguages_;
}

void VmMaskCurrentConfig::setBrowserLanguages(const QStringList& value)
{
  browserLanguages_ = value;
}

bool VmMaskCurrentConfig::getFlash() const
{
  return flash_;
}

void VmMaskCurrentConfig::setFlash(bool value)
{
  flash_ = value;
}

bool VmMaskCurrentConfig::getJava() const
{
  return java_;
}

void VmMaskCurrentConfig::setJava(bool value)
{
  java_ = value;
}

QColor VmMaskCurrentConfig::getColor() const
{
  return color_;
}

void VmMaskCurrentConfig::setColor(const QColor& value)
{
  color_ = value;
}

QStringList VmMaskCurrentConfig::getDnsServers() const
{
  return dnsServers_;
}

void VmMaskCurrentConfig::setDnsServers(const QStringList& value)
{
  dnsServers_ = value;
}

QString VmMaskCurrentConfig::getBrowser() const
{
  return browser_;
}

void VmMaskCurrentConfig::setBrowser(const QString& browser)
{
  browser_ = browser;
}

QStringList VmMaskCurrentConfig::getFonts() const
{
  return fonts_;
}

void VmMaskCurrentConfig::setFonts(const QStringList& value)
{
  fonts_ = value;
}

QString VmMaskCurrentConfig::getLocale() const
{
  return locale_;
}

void VmMaskCurrentConfig::setLocale(const QString& value)
{
  locale_ = value;
}

QString VmMaskCurrentConfig::getTimeZone() const
{
  return timeZone_;
}

void VmMaskCurrentConfig::setTimeZone(const QString& value)
{
  timeZone_ = value;
}

QString VmMaskCurrentConfig::getFullName() const
{
  return staticConfig_.FullName;
}

QString VmMaskCurrentConfig::getVmName() const
{
  return staticConfig_.VmName;
}

QString VmMaskCurrentConfig::getName() const
{
  return staticConfig_.Name;
}


unsigned short VmMaskCurrentConfig::getRdpPort()
{
  return staticConfig_.RdpPort;
}

unsigned short VmMaskCurrentConfig::getSshPort()
{
  return staticConfig_.SshPort;
}
