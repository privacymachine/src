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
#include "PmData.h"

#include <math.h>
#include <cerrno>

#include <QProcess>
#include <QRegularExpression>
#include <QCryptographicHash>
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

// Included here, not in utils.h, to resolve dependency cycle
#include "VmMaskInstance.h"


bool ExecShort(QString parCmd, QStringList& parArgs, QString* parAllOutput, bool parCheckExitCode, int parSecondsToRespond, bool parDoNotLog)
{
  // quote command with spaces
  if (parCmd.contains(' '))
    parCmd = "\"" + parCmd + "\"";

  QString combinedCmd = parCmd;
  if (parArgs.length() > 0)
    combinedCmd = parCmd + " " + parArgs.join(" ");

  return ExecShort(combinedCmd, parAllOutput, parCheckExitCode, parSecondsToRespond, parDoNotLog);
}

bool ExecShort(QString cmd, QString* allOutput, bool checkExitCode, int secondsToRespond, bool doNotLog)
{
  int msec = -1; // blocking wait
  if (secondsToRespond > 0)
    msec = secondsToRespond * 1000;

  if (!doNotLog)
  {
    if (secondsToRespond <= 0 )
    {
      ILOG("process start(blocking): " + cmd);
    }
    else
    {
      ILOG("process start(t_max=" + QString::number(secondsToRespond) + "s): " + cmd);
    }
  }

  *allOutput = "";
  QProcess proc;
  proc.start(cmd);

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
#ifdef PM_WINDOWS
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

QString removeLastLF( QString parMsg )
{
  if (parMsg.length() == 0)
    return "";

  if (parMsg[parMsg.length()-1] == '\n')
    return parMsg.left(parMsg.length()-1);
  else
    return parMsg;

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
    #else
      return 0; // under Windows we use a actual Qt version
    #endif
  #endif
}

// Function determines properties of the installed virtualbox version
void determineVirtualBoxInstallation( bool& parVboxInstalled, /* false, if virtualbox is not installed */
                                      QString& parVboxCommand, /* contains full path on windows */
                                      QString& parVboxVersion, /* i.e 5.0.10r104061 */
                                      QString& parVboxDefaultMachineFolder, /* path were new vms are created */
                                      bool& parVboxExtensionPackInstalled /* true, if the extension pack is installed */
                                     )
{
  // Init some values
  parVboxInstalled = false;
  parVboxCommand = ""; // means error
  parVboxVersion = "0.0.0";
  parVboxDefaultMachineFolder = "";
  parVboxExtensionPackInstalled = false;
  QString allOutput;
  QStringList args;

  QString vboxcommand = PmData::getInstance().getVBoxCommand();

  // On Windows we can do an early check if vboxmanage is installed
  if (RunningOnWindows())
  {
    if (vboxcommand.contains("NOTFOUND"))
    {
      IERR("Environment variable VBOX_INSTALL_PATH or VBOX_MSI_INSTALL_PATH is not defined: is VirtualBox installed?");
      return;
    }
  }

  args.clear();
  args.append("--version");
  if (!ExecShort(vboxcommand, args, &allOutput, true))
  {
    IERR("Unable to find vboxmange on the PATH, Is VirtualBox installed?");
    return;
  }
  parVboxInstalled = true;
  parVboxVersion = allOutput.trimmed();
  parVboxCommand = vboxcommand;

  args.clear();
  args.append("list");
  args.append("systemproperties");
  if (!ExecShort(vboxcommand, args, &allOutput, true, 10)) // vboxmanage needs some time sometimes
  {
    IERR("unable to detect the virtualbox properties");
    return;
  }


  QRegularExpression regExpDefaultMachineFolder("^(Default machine folder:\\s*)(.*)$", QRegularExpression::MultilineOption);
  QRegularExpressionMatch match;

  match = regExpDefaultMachineFolder.match(allOutput);
  if (match.hasMatch())
  {
    parVboxDefaultMachineFolder = match.captured(2).trimmed();
  }
  else
  {
    IERR("unable to detect the default virtual machine folder");
    return;
  }

  QRegularExpression regExpExtensionPack("^(Remote desktop ExtPack:\\s*)(Oracle VM VirtualBox Extension Pack).*$", QRegularExpression::MultilineOption);

  match = regExpExtensionPack.match(allOutput);
  if (match.lastCapturedIndex() == 2)
  {
    parVboxExtensionPackInstalled = true;
  }
  else
  {
    IERR("virtualbox extension pack is not installed");
    return;
  }
}

// Fire up a Command over SSH
PmCommand* GetPmCommandForSshCmd(QString user, QString server, QString port, QString passwd, QString command)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "cmd.exe";
  args.append("/c");
  args.append("echo");
  args.append("n"); // means: ignore host key
  args.append("|");
  args.append("plink.exe");
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
  args.append( "UserKnownHostsFile=/dev/null" );
  args.append("-o");
  args.append("IdentitiesOnly=yes");
  args.append("-p");
#endif
  args.append(port);
  args.append(user+"@"+server);
  args.append(command);
  return new PmCommand(cmd, args, true, false, "");
}

/// "Overload" that assumes "root" as user, and which takes VM-related values from pVmMaskInstance.
PmCommand* GetPmCommandForSshCmdVmMaskInstance(QSharedPointer<VmMaskInstance>& parVmMaskInstance, QString command )
{
  return GetPmCommandForSshCmd( PmData::getInstance().getBaseDiskRootUser(), PmData::getInstance().getPmServerIp(), QString::number( parVmMaskInstance->getConfig()->getSshPort() ), PmData::getInstance().getBaseDiskRootUserPassword(), command );
}

/// "Overload" that assumes "root" as user, and the specific port
PmCommand* GetPmCommandForSshCmdVmMaskInstance(int parSshPort, QString command )
{
  return GetPmCommandForSshCmd( PmData::getInstance().getBaseDiskRootUser(), PmData::getInstance().getPmServerIp(), QString::number(parSshPort), PmData::getInstance().getBaseDiskRootUserPassword(), command );
}

PmCommand* genSshCmd(QString parCommand, int parSshPort)
{
  return GetPmCommandForSshCmd( PmData::getInstance().getBaseDiskRootUser(), PmData::getInstance().getPmServerIp(), QString::number(parSshPort), PmData::getInstance().getBaseDiskRootUserPassword(), parCommand);
}

PmCommand* genSshCmdLiveUser(QString parCommand, int parSshPort)
{
  return GetPmCommandForSshCmd( PmData::getInstance().getBaseDiskLiveUser(), PmData::getInstance().getPmServerIp(), QString::number(parSshPort), PmData::getInstance().getBaseDiskLiveUserPassword(), parCommand);
}

PmCommand* genScpCmd(QString parLocalDir, QString parRemoteDir, int parSshPort)
{
  return GetPmCommandForScp2VM( PmData::getInstance().getBaseDiskRootUser(), PmData::getInstance().getPmServerIp(), QString::number(parSshPort), PmData::getInstance().getBaseDiskRootUserPassword(), parLocalDir, parRemoteDir);
}


// Copy a folder to VM
PmCommand* GetPmCommandForScp2VM(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "cmd.exe";
  args.append("/c");
  args.append("echo");
  args.append("n"); // means: ignore host key
  args.append("|");
  args.append("pscp.exe");
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
  args.append( "UserKnownHostsFile=/dev/null" );
  args.append("-o");
  args.append("IdentitiesOnly=yes");
#endif
  args.append("-p"); // preserve
  args.append("-r"); // recursive
  args.append("-P"); // port
  args.append(port);
  args.append(localDir);
  args.append(user+"@"+server+":"+remoteDir);
  PmCommand* curCommand = new PmCommand(cmd, args, true, false, "");
  return curCommand;
}

// Copy a folder to Host
PmCommand* GetPmCommandForScp2Host(QString user, QString server, QString port, QString passwd, QString localDir, QString remoteDir)
{
  QString cmd;
  QStringList args;

#ifdef PM_WINDOWS
  cmd = "cmd.exe";
  args.append("/c");
  args.append("echo");
  args.append("n"); // means: ignore host key
  args.append("|");
  args.append("pscp.exe");
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
  args.append( "UserKnownHostsFile=/dev/null" );
  args.append("-o");
  args.append("IdentitiesOnly=yes");
#endif
  args.append("-p"); // preserve
  args.append("-r"); // recursive
  args.append("-P"); // port
  args.append(port);
  args.append(user+"@"+server+":"+remoteDir);
  args.append(localDir);
  PmCommand* curCommand = new PmCommand(cmd, args, true, false, "");
  return curCommand;
}

#ifdef PM_WINDOWS
QString getLastErrorMsg()
{
  LPWSTR bufPtr = nullptr;
  DWORD err = GetLastError();
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM |
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr, err, 0, (LPWSTR)&bufPtr, 0, nullptr);
  const QString result =
      (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                 QString("Unknown Error %1").arg(err);
  LocalFree(bufPtr);
  return result;
}
#else
QString getLastErrorMsg()
{
  int errorNumber = errno;
  return QString(QString::number(errorNumber) + ":" + QString(strerror(errorNumber)));
}

#endif

/// \brief get the config dir as QDir
/// \brief: example for linux: /home/johndoe/.config/privacymachine
/// \brief: example for windows: %UserProfile%\PrivacyMachine
/// \return the config dir which depends on the os

std::string getPmDefaultConfigQDir()
{
  QString pmConfigPath = QDir::homePath();
  #if (PM_WINDOWS)
    pmConfigPath += "/PrivacyMachine";
  #else
    pmConfigPath += "/.config/privacymachine";
  #endif

  return pmConfigPath.toStdString();
}


std::string calculateConfigDirId(std::string configDir)
{
  QDir configQDir{ QString::fromStdString(configDir) };
  // We use md5 here because its not security relevant.
  QCryptographicHash hash(QCryptographicHash::Md5);
  hash.addData(configQDir.absolutePath().toLocal8Bit());
  QString pmConfigId = QString::fromLocal8Bit(hash.result().toHex());
  pmConfigId.truncate(5);
  return pmConfigId.toStdString();
}
