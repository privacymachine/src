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
#include "PmCommand.h"
#include "PmCommandExec.h"

#include <QPixmap>

// init as ShellCommand
PmCommand::PmCommand(QString parCmd,
                     QStringList& parArgs,
                     bool parShowStdErrAsStdOut,
                     bool parShowStdOutAsStdErr,
                     QString parDesc,
                     bool parHideStdOut)
{
  type_ = shellCommand;
  description_ = parDesc;
  executionCost_ = 10; // some default
  timeoutMilliseconds_ = 1000;
  hideStdOut_ = parHideStdOut;

  shellCmd_ = new PmShellCommand();

  shellCmd_->Cmd = parCmd;
  shellCmd_->Args = QStringList(parArgs); // create a copy
  shellCmd_->ShowStdErrAsStdOut = parShowStdErrAsStdOut;
  shellCmd_->ShowStdOutAsStdErr = parShowStdOutAsStdErr;
  shellCmd_->IgnoreErrors = false; // rarly used
}

// init as BootUpDetection
PmCommand::PmCommand(QString parCmd, QStringList& parArgs, QString parPngFileName, QString parDesc)
{
  type_ = bootUpDetection;
  description_ = parDesc;
  executionCost_ = 10; // some default
  timeoutMilliseconds_ = 1000;
  hideStdOut_ = false;

  shellCmd_ = new PmShellCommand();
  shellCmd_->Cmd = parCmd;
  shellCmd_->Args = QStringList(parArgs); // create a copy
  shellCmd_->ShowStdErrAsStdOut = false;
  shellCmd_->ShowStdOutAsStdErr = false;
  shellCmd_->IgnoreErrors = false; // rarly used
  pngFileName_ = parPngFileName;
}

// init as Sleep
PmCommand::PmCommand(int parMillisec)
{
  type_ = sleepCommand;
  description_ = "Sleeping for " + QString::number( parMillisec ) + " milliseconds.";
  executionCost_ = 10; // some default
  timeoutMilliseconds_ = parMillisec;
  shellCmd_ = nullptr;
  hideStdOut_ = false;
}


// init as RemoveDirCommand
PmCommand::PmCommand(QString parFolder)
{
  type_ = removeDirCommand;
  description_ = "Remove folder: " + parFolder;
  folderName_ = parFolder;
  shellCmd_ = nullptr;
}


PmCommand::~PmCommand()
{
  if (shellCmd_)
  {
    delete shellCmd_;
    shellCmd_ = nullptr;
  }
}

bool PmCommand::hasBootUpFinished()
{
  int curHeight = 0;
  if (QFile::exists(pngFileName_))
  {
    QPixmap curScreenshot(pngFileName_);
    curHeight = curScreenshot.size().height();
  }

  ILOG("current height of bootup-screen is " + QString::number(curHeight));

  if (curHeight >= 600)
  {
    // seems the display is initialized! -> we are ready
    return true;
  }
  else
  {
    return false;
  }
}

// returns true if we should retry the current command one second later
bool PmCommand::shouldRetryInOneSecond()
{
  if (retries_ > 0)
    retries_ -= 1;
  else
    retries_ = 0;

  // If there are retries left, the command wants to be executed again. The caller then can decide, depending on last
  // exit code / status, whether the command actually will be repeated.
  return retries_ > 0;
}

int PmCommand::popAllExecutionCosts()
{
  int ret = executionCost_;
  executionCost_ = 0;

  return ret;
}

int PmCommand::popExecutionCosts()
{
  int ret = 10;

  if (ret > executionCost_)
    ret = executionCost_;

  executionCost_ -= ret;

  return ret;
}


void PmCommand::executeRemoveDirCommand()
{
  QDir toRemove( folderName_ );
  ILOG("removeDirCommand: " + folderName_)
  if (toRemove.exists())
  {
    ILOG("removeRecursively()")
    if (!toRemove.removeRecursively())
    {
      IWARN("removeRecursively() returned an error");
    }
  }
}

bool PmCommand::start(QProcess* parProc, QObject* parReceiver)
{
  if (type_ == removeDirCommand)
  {
    // we can execute this command immediately
    executeRemoveDirCommand();

    // We use a singleShot-Timer for galvanic isolation
    QTimer::singleShot( 0, parReceiver, SLOT( slotCommandFinished() ) );
  }
  else if (type_ == sleepCommand)
  {
    // For sleeping, we do not really spawn a new process, but simply start a timer.
    QTimer::singleShot( timeoutMilliseconds_, parReceiver, SLOT( slotCommandFinished() ) );
  }
  else if( shellCmd_ != nullptr )
  {
    QString toExecute = shellCmd_->Cmd + " " + shellCmd_->Args.join(" ");
    ILOG("Execute PmCommand: " + toExecute);
    // Attempt to execute command.
    parProc->start(shellCmd_->Cmd, shellCmd_->Args);
    if (!parProc->waitForStarted())
    {
      QString err = parProc->errorString();
      QString msg = "Error starting process: '" + toExecute + "' resulted in:\n" + err;
      IERR(msg);
      return false;
    }
  }
  else
  {
    QString message = "No command provided for command type " + QString::number( type_ ) + ".";
    IERR( message );
    return false;
  }

  return true;
}

bool PmCommand::executeBlocking(bool parCheckExitCode)
{
  if (type_ == removeDirCommand)
  {
    // we can execute this command immediately
    executeRemoveDirCommand();
  }
  else if (type_ == sleepCommand)
  {
    SleeperThread::msleep(timeoutMilliseconds_);
  }
  else if( shellCmd_ != nullptr )
  {
    // logging is done in ExecShort()
    QString allOutput = "";
    return ExecShort(shellCmd_->Cmd, shellCmd_->Args, &allOutput, parCheckExitCode, 10, false);
  }
  else
  {
    QString message = "No command provided for command type " + QString::number( type_ ) + ".";
    IERR( message );
    return false;
  }
  return true;
}

bool PmCommand::getIgnoreErrors()
{
  if (shellCmd_)
    return shellCmd_->IgnoreErrors;
  else
   return false;
}

void PmCommand::setIgnoreErrors( bool ignoreErrors )
{
  if (shellCmd_)
    shellCmd_->IgnoreErrors = ignoreErrors;
}
