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

#include "utils.h"
#include "UserConfig.h"
#include "SystemConfig.h"
#include "PMInstanceConfiguration.h"

#include <QStringList>


QList<QString> *PMInstanceConfiguration::timeZones_ = NULL;
QList<QString> *PMInstanceConfiguration::locales_ = NULL;


PMInstanceConfiguration::PMInstanceConfiguration(ConfigVmMask *parCurConfigVmMask,
                                                 UserConfig *parWholeConfig,
                                                 SystemConfig *parSystemConfig)
{
  systemConfig = parSystemConfig;
  name = parCurConfigVmMask->name;
  vmName = parCurConfigVmMask->vmName;
  vmMaskCreated = false;
  fullName = parCurConfigVmMask->fullName;
  fontCount = randInt(0,10);

  // Choose one Language
  language = "en"; // init
  QStringList languages = parCurConfigVmMask->languages.split(',', QString::SkipEmptyParts);
  if (languages.count())
  {
    int pos = randInt(0,languages.count()-1);
    language = languages[pos];
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
  java = parCurConfigVmMask->java;
  flash = parCurConfigVmMask->flash;
  thirdPartyCookies = parCurConfigVmMask->thirdPartyCookies;

  // Choose a unique display size (subtract some pixels)
  subtractDisplayWidth = randInt(8,32);
  subtractDisplayHeight = randInt(8,16);

  /*
  // Seems odd-numbered values for display sizes produces real problems
  // -> round up
  subtractDisplayWidth += (subtractDisplayWidth & 1);
  subtractDisplayHeight += (subtractDisplayHeight & 1);

  // HACK: For testing FreeRDP, we want reproduceable screen sizes
  subtractDisplayWidth = 10;
  subtractDisplayHeight = 10;
  */

  // Choose one Browser
  browser = "Firefox"; // init
  QStringList browsers = parCurConfigVmMask->browsers.split(',', QString::SkipEmptyParts);
  if (browsers.count())
  {
    int pos = randInt(0,browsers.count()-1);
    browser = browsers[pos];
  }

  scriptOnStartup = parCurConfigVmMask->scriptOnStartup;
  scriptOnShutdown = parCurConfigVmMask->scriptOnShutdown;
}



void PMInstanceConfiguration::init()
{
  if( timeZones_ == NULL )
  {
    initializeTimeZones();
    
  }
  
  if( timeZones_->count() > 0 )
  {
    timeZone_ = timeZones_->at( randInt( 0, timeZones_->count()-1 ) );
  }
  else
  {
    timeZone_ = "Europe/London";
  }

  
  if( locales_ == NULL )
  {
    initializeLocales();
    
  }
  
  if( locales_->count() > 0 )
  {
    locale_ = locales_->at( randInt( 0, locales_->count()-1 ) );
  }
  else
  {
    locale_ = "en_GB.utf8";
  }

}


void PMInstanceConfiguration::initializeTimeZones()
{
  timeZones_ = new QList< QString >();
  QFile timeZonesFile( "pm_data/timezones.txt" );
  if( timeZonesFile.open( QFile::ReadOnly ) )
  {
    QTextStream timeZonesStream( &timeZonesFile );

    QString currentLine = "";
    while( !timeZonesStream.atEnd() )
    {
      currentLine = timeZonesStream.readLine();
      timeZones_->append( currentLine );

    }

    timeZonesFile.close();

  }

}

void PMInstanceConfiguration::initializeLocales()
{
  locales_ = new QList< QString >();
  QFile localesFile( "pm_data/locales.txt" );
  if( localesFile.open( QFile::ReadOnly ) )
  {
    QTextStream localesStream( &localesFile );

    QString currentLine = "";
    while( !localesStream.atEnd() )
    {
      currentLine = localesStream.readLine();
      locales_->append( currentLine );

    }

    localesFile.close();

  }

}


QString PMInstanceConfiguration::toString()
{
  return "\"" + fullName + "\" - " 
      + QObject::tr("Language") + ": " + language + ", " 
      + QObject::tr("Locale") + ": " + locale_ + ", " 
      + QObject::tr("Time Zone") + ": " + timeZone_ + ", " 
      + QObject::tr("Additional Fonts") + ": " + QString::number( fontCount ) + "/359";

}
