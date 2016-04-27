﻿/*==============================================================================
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
#include "PMCommand.h"

#include <QPixmap>

// init as ShellCommand
PMCommand::PMCommand(QString parCmd, QStringList& parArgs, bool parShowStdErrAsStdOut, bool parShowStdOutAsStdErr)
{
  type_ = shellCommand;
  description_ = "";
  cost_ = 10; // some default
  timeoutMilliseconds_ = 1000;

  shellCmd_ = new PMShellCommand();

  shellCmd_->cmd = parCmd;
  shellCmd_->args = QStringList(parArgs); // create a copy
  shellCmd_->showStdErrAsStdOut = parShowStdErrAsStdOut;
  shellCmd_->showStdOutAsStdErr = parShowStdOutAsStdErr;
  shellCmd_->ignoreErrors = false; // rarly used

}

// init as BootUpDetection
PMCommand::PMCommand(QString parCmd, QStringList& parArgs, QString parPngFileName)
{
  type_ = bootUpDetection;
  description_ = "";
  cost_ = 10; // some default
  timeoutMilliseconds_ = 1000;

  shellCmd_ = new PMShellCommand();
  shellCmd_->cmd = parCmd;
  shellCmd_->args = QStringList(parArgs); // create a copy
  shellCmd_->showStdErrAsStdOut = false;
  shellCmd_->showStdOutAsStdErr = false;
  shellCmd_->ignoreErrors = false; // rarly used
  pngFileName_ = parPngFileName;
}

// init as Sleep
PMCommand::PMCommand(int parMillisec)
{
  type_ = sleepCommand;
  description_ = "Sleeping for " + QString::number( parMillisec ) + " milliseconds.";
  cost_ = 10; // some default
  timeoutMilliseconds_ = parMillisec;
  shellCmd_ = NULL;

}

PMCommand::~PMCommand()
{
  if (shellCmd_)
  {
    delete shellCmd_;
    shellCmd_ = NULL;
  }
}

// returns true if we should retry the current command one second later
bool PMCommand::shouldRetryInOneSecond()
{
  int curHeight = 0;
  switch( type_ )
  {
  case bootUpDetection:
    // we have to check if we don't need a next round...
    if (QFile::exists(pngFileName_))
    {
      QPixmap curScreenshot(pngFileName_);
      curHeight = curScreenshot.size().height();
    }

    ILOG("current height of bootup-screen is " + QString::number(curHeight));

    if (curHeight >= 600)
    {
      // seems the display is initialized! -> we are ready
      return false;
    }
    else
    {
      return true;
    }
    break;

  case pollingShellCommand:
    retries_ = retries_ > 0  ?  retries_ - 1  :  0;
    // If there are retries left, the command wants to be executed again. The caller then can decide, depending on last
    // exit code / status, whether the command actually will be repeated.
    return retries_ > 0;
    break;

  default:
    return false;
    break;
  }

}

int PMCommand::popAllCosts()
{
  int ret = cost_;
  cost_ = 0;

  return ret;
}

int PMCommand::popCosts()
{
  int ret = 10;

  if (ret > cost_)
    ret = cost_;

  cost_ -= ret;

  return ret;
}

bool PMCommand::start(QProcess* parProc, QObject* parReceiver)
{
  // For sleeping, we do not really spawn a new process, but simply start a timer.
  if (type_ == sleepCommand)
  {
    QTimer::singleShot( timeoutMilliseconds_, parReceiver, SLOT( slotCommandFinished() ) );

  }
  else if( shellCmd_ != NULL )
  {
    QString toExecute = shellCmd_->cmd + " " + shellCmd_->args.join(" ");
    ILOG("Execute: " + toExecute);
    // Attempt to execute command.
    parProc->start(shellCmd_->cmd, shellCmd_->args);
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

bool PMCommand::executeBlocking(bool parCheckExitCode)
{
  QString allOutput = "";
  QString toExecute = shellCmd_->cmd + " " + shellCmd_->args.join(" ");
  ILOG("Execute: " + toExecute);
  return ExecShort(shellCmd_->cmd, shellCmd_->args, &allOutput, parCheckExitCode, 10);
}