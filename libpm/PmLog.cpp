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

#include "PmLog.h"
#include "utils.h"

#include <QFile>
#include <QDir>
#include <QTextStream>

PmLog::PmLog():
  logFileName_(""),
  sensitiveLogging_(false)
{
}

void PmLog::logMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  QString txt;
  switch (type)
  {
    case QtDebugMsg:
      txt = QString("%1").arg(msg);
      break;
    case QtWarningMsg:
      txt = QString("Warning: %1").arg(msg);
      break;
    case QtCriticalMsg:
      txt = QString("Critical: %1").arg(msg);
      break;
    case QtFatalMsg:
      txt = QString("Fatal: %1").arg(msg);
  }

  // Also write to the Console
  if (RunningOnWindows())
    fprintf(stderr, "%s\n", msg.toLatin1().data());
  else
    fprintf(stderr, "%s\n", msg.toUtf8().data());

  QFile outFile(logFileName_);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
  {
    QTextStream ts(&outFile);
    ts << txt << endl;
  }

  if (type == QtFatalMsg) abort();
}


bool PmLog::initAndRotateLogfiles(QString parUserConfigDir)
{
  logFileName_ = "";
  QString logMainDir = parUserConfigDir + "/logs";

  // Create the logs directory in the user home folder
  QDir logDir(logMainDir);
  if (!logDir.exists())
  {
    qDebug() << "create log directory: " << logMainDir;
    if (!logDir.mkpath(".")) // creates subpaths also
    {
      QString systemError = getLastErrorMsg();
      qCritical() << "failed to create user configuration directory: " << systemError;
      return false;
    }
  }

  logFileName_ = logMainDir + "/PrivacyMachineLog.txt";

  // logrotation
  if (QFile::exists(logFileName_))
  {
    for (int i = 5; i >= 1; i--)
    {
      if (QFile::exists(logFileName_ + "." + QString::number(i)))
      {
        // delete .5
        if (i == 5)
        {
          QFile::remove(logFileName_ + "." + QString::number(i));
        }
        else
        {
          // rename .4 -> .5  ...
          for (int j = i; j >=1; j--)
            QFile::rename(logFileName_ + "." + QString::number(j), logFileName_ + "." + QString::number(j+1));

          break;
        }
      }
    }
    // rename to ".1"
    QFile::rename(logFileName_, logFileName_ + ".1");
  }

  return true;
}

