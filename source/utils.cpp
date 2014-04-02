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
#include <math.h>

#include <QProcess>

bool ExecShort(QString cmd, QStringList& args, QString* allOutput, bool checkExitCode, int secondsToRespond, bool doNotLog)
{

  // quote command with spaces
  if (cmd.contains(' '))
    cmd = "\"" + cmd + "\"";

  QString combinedCmd = cmd;
  if (args.length() > 0)
    combinedCmd = cmd + " " + args.join(" ");

  return ExecShort(combinedCmd, allOutput, checkExitCode, secondsToRespond, doNotLog);
}

bool ExecShort(QString cmd, QString* allOutput, bool checkExitCode, int secondsToRespond, bool doNotLog)
{
  if (!doNotLog)
    ILOG("process start: " + cmd);

  *allOutput = "";
  QProcess proc;
  proc.start(cmd);

  int msec = -1; // blocking wait
  if (secondsToRespond > 0)
    msec = secondsToRespond * 1000;

  if (!proc.waitForFinished(msec)) // abort after one second
  {
    IERR("process takes to loong to respond, or start failed.");
    return false;
  }

  *allOutput = proc.readAllStandardOutput();
  *allOutput += proc.readAllStandardError();

  if (checkExitCode && proc.exitCode() != 0)
  {
    IWARN("process return code: " + QString::number(proc.exitCode()));
    return false;
  }
  return true;
}

bool RunningOnWindows()
{
#ifdef Q_WS_WIN
  return true;
#else
  return false;
#endif
}

QString currentTimeStampAsISO()
{
  QString now = QDate::currentDate().toString(Qt::ISODate) + " " + QTime::currentTime().toString(Qt::ISODate);
  return now;
}

QString currentTimeStampAsISOFileName()
{
  QString now = QDate::currentDate().toString(Qt::ISODate) + "_" + QTime::currentTime().toString(Qt::ISODate);
  now = now.replace(':', '-');
  return now;
}

int randInt(int low, int high)
{
  // Random number between low and high
  return qrand() % ((high + 1) - low) + low;
}
