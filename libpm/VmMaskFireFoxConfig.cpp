#include "VmMaskFireFoxConfig.h"

VmMaskFireFoxConfig::VmMaskFireFoxConfig()
{
}

// Define possible firefox preference possibilities
QList< QPair<QString, QList<QVariant> > > VmMaskFireFoxConfig::configValueList_ =
    QList< QPair<QString, QList<QVariant> > >()


/* PREF: general referrer
 * http://kb.mozillazine.org/Network.http.sendRefererHeader
 * VALUE 0: Never send the Referer header or set document.referrer. (NOT USED, can cause Problems)
 * VALUE 1: Send the Referer header when clicking on a link, and set document.referrer for the following page.
 * VALUE 2: Send the Referer header when clicking on a link or loading an image,
 *          and set document.referrer for the following page.
 */
<< qMakePair(QString("network.http.sendRefererHeader"),QList<QVariant>() << 1 << 2)

/* PREF: HTTPS referrer
 * http://kb.mozillazine.org/Network.http.sendSecureXSiteReferrer
 * VALUE is bool
 */
<< qMakePair(QString("network.http.sendSecureXSiteReferrer"),QList<QVariant>() << true << false)

/* PREF: DoNotTrack header
 * http://kb.mozillazine.org/Privacy.donottrackheader.enabled
 * VALUE is bool
 */
<< qMakePair(QString("privacy.donottrackheader.enabled"),QList<QVariant>() << true << false);
