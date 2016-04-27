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

#pragma once

#include <QTime>
#include <QDebug>
#include <QVariant>
#include <QDir>
#include <QThread>
#include "PMCommand.h"

#ifndef PM_WINDOWS
#include <unistd.h>
#endif

extern bool globalSensitiveLoggingEnabed; // declared in utils.cpp

#define ILOG(message) {  qDebug() << qPrintable(message); }
#define ILOG_SENSITIVE(message) {if (globalSensitiveLoggingEnabed) { qDebug() << qPrintable(message); } }
#define IWARN(message) { qWarning() << qPrintable(message); }
#define IERR(message) { qCritical() << qPrintable(message); }

bool ExecShort(QString cmd, QString* outOutput, bool checkExitCode, int secondsToRespond = 3, bool doNotLog = false);
bool ExecShort(QString cmd, QStringList& args, QString* allOutput, bool checkExitCode, int secondsToRespond = 3, bool doNotLog = false);

const char constPrivacyMachineVersion[] = "0.2-alpha";

// to display an endless loop i.e.: "for (;;)" -> "for(ever)"
#define ever ;;

template <class T> class VPtr
{
  public:
    static T* asPtr(QVariant v)
    {
      return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
      return qVariantFromValue((void *) ptr);
    }
};

bool RunningOnWindows();

QString currentTimeStampAsISO();
QString currentTimeStampAsISOFileName();

bool removeDir(QString fullPath);
bool removeMountFolder(QString mountPath);

class SleeperThread : public QThread
{
  public:
    static void msleep(unsigned long msecs)
    {
      QThread::msleep(msecs);
    }
};

int randInt(int low, int high);
qint64 getFreeDiskSpace(QString dir);

bool determineVirtualBoxInstallation( QString& vboxManagePath, /* empty if virtualbox is not installed */
                                      QString& vboxVersion, /* i.e 5.0.10r104061 */
                                      QString& vboxDefaultMachineFolder, /* path were new vms are created */
                                      bool& vboxExtensionPackInstalled /* true, if the extension pack is installed */
                                     );

PMCommand* GetPMCommandForSshCmd(QString user, QString server, QString port, QString passwd, QString command);
PMCommand* GetPMCommandForScp2VM(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir);
PMCommand* GetPMCommandForScp2Host(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir);
