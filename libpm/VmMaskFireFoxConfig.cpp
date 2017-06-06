#include "VmMaskFireFoxConfig.h"
#include "utils.h"

VmMaskFireFoxConfig::VmMaskFireFoxConfig()
{
  configValueList_ = QList< QPair<QString, QVariant> >();
}

QString VmMaskFireFoxConfig::getPrefs()
{
  QString prefs="";

  if(configValueList_.size() > 0)
  {
    prefs += "# This firefox configuration was automatically created by PrivacyMachine\n\n";

    for(auto setting : configValueList_)
    {
      QString settingName = setting.first;
      QVariant settingValue = setting.second;

      // append the setting
      prefs += "user_pref(";
      prefs += "\"" + settingName + "\", ";
      prefs += (settingValue.type() == QVariant::String) ? "\""+settingValue.toString()+"\"" : settingValue.toString();
      prefs += ");\n";
    }
  }

  return prefs;
}

void VmMaskFireFoxConfig::diceNewVmMaskFireFoxConfig()
{
  ILOG("Randomizing firefox configuration");
  configValueList_.clear();

  // append static configuration values
  for(auto setting : staticConfigValueList_)
  {
    QString settingName = setting.first;
    QVariant settingValue = setting.second;

    // append the setting
    configValueList_ << qMakePair(settingName, settingValue);

    // log the setting
    QString logMsg = "firefox setting: \"" + settingName + "\" = ";
    logMsg += (settingValue.type() == QVariant::String) ? "\""+settingValue.toString()+"\"" : settingValue.toString();
    ILOG(logMsg);
  }

  // randomize and append variable configuration values
  for(auto setting : variableConfigValueList_)
  {
    QString settingName = setting.first;
    QList<QVariant>  settingValueList = setting.second;

    // randomize the setting
    auto settingValue = settingValueList[randombytes_uniform(settingValueList.size())];

    // append the setting
    configValueList_ << qMakePair(settingName, settingValue);

    // log the setting
    QString logMsg = "firefox setting: \"" + settingName + "\" = ";
    logMsg += (settingValue.type() == QVariant::String) ? "\""+settingValue.toString()+"\"" : settingValue.toString();
    ILOG(logMsg);
  }
}

// ################################################################################################
// Define possible firefox preference possibilities
// ################################################################################################
QList< QPair<QString, QList<QVariant> > > VmMaskFireFoxConfig::variableConfigValueList_ =
    QList< QPair<QString, QList<QVariant> > >()

/* PREF: general referrer
 * http://kb.mozillazine.org/Network.http.sendRefererHeader
 * VALUE 0: Never send the Referer header or set document.referrer. (NOT USED, can cause Problems)
 * VALUE 1: Send the Referer header when clicking on a link, and set document.referrer for the following page.
 * VALUE 2: Send the Referer header when clicking on a link or loading an image,
 *          and set document.referrer for the following page.
 */
<< qMakePair( QString("network.http.sendRefererHeader"), QList<QVariant>() << 1 << 2 )

/* PREF: HTTPS referrer
 * http://kb.mozillazine.org/Network.http.sendSecureXSiteReferrer
 * VALUE is bool
 */
<< qMakePair( QString("network.http.sendSecureXSiteReferrer"), QList<QVariant>() << true << false )

/* PREF: DoNotTrack header
 * http://kb.mozillazine.org/Privacy.donottrackheader.enabled
 * VALUE is bool
 */
<< qMakePair( QString("privacy.donottrackheader.enabled"), QList<QVariant>() << true << false );


// ################################################################################################
// Define static firefox preferences
// ################################################################################################
QList< QPair<QString, QVariant> > VmMaskFireFoxConfig::staticConfigValueList_=
    QList< QPair<QString, QVariant> >()

/* PREF: start FF with a blanc page
 * http://kb.mozillazine.org/Browser.startup.page
 * http://kb.mozillazine.org/Startup.homepage_override_url
 * http://kb.mozillazine.org/Browser.startup.homepage_override.mstone
 */
<< qMakePair( QString("browser.startup.page"), QVariant(0) )
<< qMakePair( QString("startup.homepage_override_url"), QVariant("about:blank") )
<< qMakePair( QString("browser.startup.homepage_override.mstone"), QVariant("99.9.9") )

/* PREF: Disable collection/sending of the health report (healthreport.sqlite*)
 * https://support.mozilla.org/en-US/kb/firefox-health-report-understand-your-browser-perf
 * https://gecko.readthedocs.org/en/latest/toolkit/components/telemetry/telemetry/preferences.html
 */
<< qMakePair( QString("datareporting.healthreport.uploadEnabled"), QVariant(false) )
<< qMakePair( QString("datareporting.healthreport.service.enabled"), QVariant(false) )
<< qMakePair( QString("datareporting.policy.dataSubmissionEnabled"), QVariant(false) )

/* PREF: Disable Heartbeat  (Mozilla user rating telemetry)
 * https://wiki.mozilla.org/Advocacy/heartbeat
 * https://trac.torproject.org/projects/tor/ticket/19047
 */
<< qMakePair( QString("browser.selfsupport.url"), QVariant("") )

/* PREF: Disable Mozilla telemetry/experiments
 * https://wiki.mozilla.org/Platform/Features/Telemetry
 * https://wiki.mozilla.org/Privacy/Reviews/Telemetry
 * https://wiki.mozilla.org/Telemetry
 * https://www.mozilla.org/en-US/legal/privacy/firefox.html#telemetry
 * https://support.mozilla.org/t5/Firefox-crashes/Mozilla-Crash-Reporter/ta-p/1715
 * https://wiki.mozilla.org/Security/Reviews/Firefox6/ReviewNotes/telemetry
 * https://gecko.readthedocs.io/en/latest/browser/experiments/experiments/manifest.html
 * https://wiki.mozilla.org/Telemetry/Experiments
 */
<< qMakePair( QString("toolkit.telemetry.enabled"), QVariant(false) )
<< qMakePair( QString("toolkit.telemetry.unified"), QVariant(false) )
<< qMakePair( QString("toolkit.telemetry.archive.enabled"), QVariant(false) )
<< qMakePair( QString("experiments.supported"), QVariant(false) )
<< qMakePair( QString("experiments.enabled"), QVariant(false) )
<< qMakePair( QString("experiments.manifest.uri"), QVariant("") )

/* PREF: disable 3rd-party cookies
 * http://kb.mozillazine.org/Network.cookie.cookieBehavior
 */
<< qMakePair( QString("network.cookie.cookieBehavior"), QVariant(1) );
