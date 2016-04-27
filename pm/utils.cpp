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
#include <math.h>

#include <QProcess>
#include <QRegularExpression>
#include <QtGlobal>
#if QT_VERSION >= 0x050400
  #include <QStorageInfo>
#else
  #ifdef PM_WINDOWS
    compile-error: windows build has to use a actual version of qt (>= 5.4)
  #else
    #include <sys/statvfs.h>
  #endif
#endif

bool globalSensitiveLoggingEnabed = false;

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


qint64 getFreeDiskSpace(QString dir)
{
  #if QT_VERSION >= 0x050400
    #include <QStorageInfo>
  QStorageInfo df(dir);
  return df.bytesFree();
  #else
    #ifndef PM_WINDOWS
      struct statvfs info;
      int retCode = statvfs(dir.toUtf8().constData(), &info);

      quint64 diskFree = 0;
      if(retCode == 0)
      {
        diskFree = info.f_bsize * info.f_bavail;
      }

      return diskFree;
    #endif
  #endif
}

// Function determines properties of the installed virtualbox version
// the returncode false means: not installed
bool determineVirtualBoxInstallation( QString& parVboxCommand, /* contains full path on windows */
                                      QString& parVboxVersion, /* i.e 5.0.10r104061 */
                                      QString& parVboxDefaultMachineFolder, /* path were new vms are created */
                                      bool& parVboxExtensionPackInstalled /* true, if the extension pack is installed */
                                     )
{
  // Init some values
  parVboxCommand = ""; // means error
  parVboxVersion = "0.0.0";
  parVboxDefaultMachineFolder = "";
  parVboxExtensionPackInstalled = false;

  QString vboxcommand = "";
  if (RunningOnWindows())
  {
    // We can't use QSettings-Registry-Methods here because we are running under WOW64
    // so we get it from the environment variable 'VBOX_INSTALL_PATH'
    vboxcommand = QProcessEnvironment::systemEnvironment().value("VBOX_INSTALL_PATH", "C:\\VirtualBoxDir_NOTFOUND\\");
    vboxcommand+= "VBoxManage.exe";
  }
  else
  {
    // on Linux it is usually on the path
    vboxcommand = "vboxmanage";
  }

  QString allOutput;
  QStringList args;

  args.clear();
  args.append("--version");
  if (!ExecShort(vboxcommand, args, &allOutput, true))
  {
    IERR("Unable to find vboxmange on the PATH, Is VirtualBox installed?");
    return false;
  }
  parVboxVersion = allOutput.trimmed();
  parVboxCommand = vboxcommand;

  args.clear();
  args.append("list");
  args.append("systemproperties");
  if (!ExecShort(vboxcommand, args, &allOutput, true))
  {
    IERR("unable to detect the virtualbox properties");
    return false;
  }


  QRegularExpression regExpDefaultMachineFolder("^(Default machine folder:\\s*)(.*)$", QRegularExpression::MultilineOption);
  QRegularExpressionMatch match;

  match = regExpDefaultMachineFolder.match(allOutput);
  if (match.hasMatch())
  {
    parVboxDefaultMachineFolder = match.captured(2);
  }
  else
  {
    IERR("unable to detect the default virtual machine folder");
    return false;
  }

  QRegularExpression regExpExtensionPack("^(Remote desktop ExtPack:\\s*)(.*)$", QRegularExpression::MultilineOption);

  match = regExpExtensionPack.match(allOutput);
  if (match.hasMatch())
  {
    QString extPack = match.captured(2);
    if (extPack.length() > 0)
      parVboxExtensionPackInstalled = true;
  }
  else
  {
    IERR("unable to detect if the virtualbox extension pack is installed");
    return false;
  }

  return true;
}

// Fire up a Command over SSH
PMCommand* GetPMCommandForSshCmd(QString user, QString server, QString port, QString passwd, QString command)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "echo n | plink.exe";
  args.append("-ssh");
  args.append("-pw");
  args.append(passwd);
  args.append("-P");
#else
  cmd = "sshpass";
  args.append("-p");
  args.append(passwd);
  args.append("ssh");
  args.append("-o");
  args.append("StrictHostKeyChecking=no");
  args.append("-o");
  args.append("UserKnownHostsFile=/dev/null");
  args.append("-o");
  args.append("IdentitiesOnly=yes");
  args.append("-p");
#endif
  args.append(port);
  args.append(user+"@"+server);
  args.append(command);
  return new PMCommand(cmd, args, true, false);
}

// Copy a folder to VM
PMCommand* GetPMCommandForScp2VM(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "echo n | pscp.exe";
  args.append("-pw");
  args.append(passwd);
#else
  cmd = "sshpass";
  args.append("-p");
  args.append(passwd);
  args.append("scp");
  args.append("-o");
  args.append("StrictHostKeyChecking=no");
  args.append("-o");
  args.append("UserKnownHostsFile=/dev/null");
  args.append("-o");
  args.append("IdentitiesOnly=yes");
#endif
  args.append("-p"); // preserve
  args.append("-r"); // recursive
  args.append("-P"); // port
  args.append(port);
  args.append(localDir);
  args.append(user+"@"+server+":"+remoteDir);
  PMCommand* curCommand = new PMCommand(cmd, args, true, false);
  return curCommand;
}

// Copy a folder to Host
PMCommand* GetPMCommandForScp2Host(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "echo n | pscp.exe";
  args.append("-pw");
  args.append(passwd);
#else
  cmd = "sshpass";
  args.append("-p");
  args.append(passwd);
  args.append("scp");
  args.append("-o");
  args.append("StrictHostKeyChecking=no");
  args.append("-o");
  args.append("UserKnownHostsFile=/dev/null");
  args.append("-o");
  args.append("IdentitiesOnly=yes");
#endif
  args.append("-p"); // preserve
  args.append("-r"); // recursive
  args.append("-P"); // port
  args.append(port);
  args.append(user+"@"+server+":"+remoteDir);
  args.append(localDir);
  PMCommand* curCommand = new PMCommand(cmd, args, true, false);
  return curCommand;
}
