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

#include "PmCommandExec.h"

#include <QRegularExpression>

PmCommandExec::PmCommandExec()
{ 
  running_ = false;
  currentCommand_ = NULL;
}

PmCommandExec::~PmCommandExec()
{
  disconnectSignalsAndSlots();
  proc_.kill();
}

void PmCommandExec::connectSignalsAndSlots()
{
  connect(&proc_,
          &QProcess::readyReadStandardOutput,
          this,
          &PmCommandExec::slotReadyReadStandardOutput);

  connect(&proc_,
          &QProcess::readyReadStandardError,
          this,
          &PmCommandExec::slotProcessReadyReadStandardError);

  connect(&proc_,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)> (&QProcess::finished),
          this,
          &PmCommandExec::slotProcessFinished);

}

void PmCommandExec::disconnectSignalsAndSlots()
{
  disconnect(&proc_,
             &QProcess::readyReadStandardOutput,
             this,
             &PmCommandExec::slotReadyReadStandardOutput);

  disconnect(&proc_,
             &QProcess::readyReadStandardError,
             this,
             &PmCommandExec::slotProcessReadyReadStandardError);

  disconnect(&proc_,
             static_cast<void (QProcess::*)(int, QProcess::ExitStatus)> (&QProcess::finished),
             this,
             &PmCommandExec::slotProcessFinished);
}

int PmCommandExec::getCostsAll()
{
  return costsAll_;
}

int PmCommandExec::getCostsFinished()
{
  return costsFinished_;
}

PmCommand* PmCommandExec::getCurrentCommand()
{
  return currentCommand_;
}

QString PmCommandExec::getLastCommandLastLineStdErr()
{
  return lastCommandLastLineStdErr_;
}

QString PmCommandExec::getLastCommandLastLineStdOut()
{
  return lastCommandLastLineStdOut_;
}

QString PmCommandExec::getLastCommandStdErr()
{
  return lastCommandStdErr_;
}

QString PmCommandExec::getLastCommandStdOut()
{
  return lastCommandStdOut_;
}

bool PmCommandExec::isRunning()
{
  return running_;
}

bool PmCommandExec::setCommands( QList<PmCommand *>& parCommands )
{
  if( running_ )
  {
    return false;
  }

  commands_ = parCommands; // Copy the Pointers, we delete them here when we are finished
  currentCommand_ = NULL;
  lastCommandStdOut_ = "";
  lastCommandStdErr_ = "";
  lastCommandExitCode_ = 0;
  costsAll_ = 0;
  costsFinished_ = 0;
  userAborted_=false;
  internalAborted_=false;

  foreach( PmCommand* cmd, commands_ )
  {
    costsAll_ += cmd->getAllExecutionCosts();
  }

  return true;
}

void PmCommandExec::start()
{
  running_ = true;

  // start executing the first command
  slotStartNextCommand();
}

void PmCommandExec::abort()
{
  userAborted_=true;
  running_ = false;
  proc_.kill();
  writeFromStandardErr("user aborted process");

  // skip other commands
  while( commands_.size() )
  {
    delete commands_.takeFirst();
  }

  emit signalFinished( aborted );
}


void PmCommandExec::writeFromStandardErr( QString parOut )
{
  IERR(removeLastLF(parOut));

  allStderr_ += parOut;
  lastCommandStdErr_ += parOut;
  lastCommandLastLineStdErr_ = parOut;

  emit signalWriteFromStandardErr( parOut );
}

void PmCommandExec::writeFromStandardOut(QString parOut)
{
  allStdout_ += parOut;
  lastCommandStdOut_ += parOut;
  lastCommandLastLineStdOut_ = parOut;

  if (currentCommand_ != NULL && !currentCommand_->getHideStdOut())
  {
    ILOG(removeLastLF(parOut));
    emit signalWriteFromStandardOut( parOut );
  }
}

QString PmCommandExec::removeLastLF(QString parMsg)
{
  if (parMsg.length() == 0)
    return "";

  if (parMsg[parMsg.length()-1] == '\n')
    return parMsg.left(parMsg.length()-1);
  else
    return parMsg;
}

void PmCommandExec::slotCommandFinished()
{
  // this slot is usualy used by a timer
  slotProcessFinished(0, QProcess::NormalExit);
}

void PmCommandExec::slotStartNextCommand()
{
  if( !commands_.size() )
  {
    return;
  }

  currentCommand_ = commands_.front();

  writeFromStandardOut(currentTimeStampAsISO() + " Starting: " + currentCommand_->getDescription() + "\n");
  emit signalStartingNextCommand();

  if( !currentCommand_->start( &proc_, this ) )
  {
    emit signalStartingNextCommandFailed();

  }
}

void PmCommandExec::slotReadyReadStandardOutput()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  if (internalAborted_) return; // can occour if some messages are in the queue after an error has happened

  proc_.setReadChannel(QProcess::StandardOutput);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (commands_.size())
  {
    if (commands_.front()->getShellCommand()->ShowStdOutAsStdErr)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardErr(newOutput);
  else
    writeFromStandardOut(newOutput);
}

void PmCommandExec::slotProcessReadyReadStandardError()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  if (internalAborted_) return; // can occour if some messages are in the queue after an error has happened

  proc_.setReadChannel(QProcess::StandardError);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (commands_.size())
  {
    if (commands_.front()->getShellCommand()->ShowStdErrAsStdOut)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardOut(newOutput);
  else
    writeFromStandardErr(newOutput);
}

void PmCommandExec::slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus)
{ 

  if(userAborted_ // can occour if some messages are in the queue after the user has aborted
     || internalAborted_ ) // can occour if some messages are in the queue after an error has happened
  {
    return; 
  }

  if (commands_.size())
  {
    currentCommand_ = commands_.front();

    // Initializing pattern to "" will make the regex match by default, so that regex checks pass by default if there 
    // was no regex pattern given.
    QRegularExpression regex("");
    if( currentCommand_->getRegexPattern() != NULL )
    {
      regex.setPattern(currentCommand_->getRegexPattern());
    }
    QRegularExpressionMatch match = regex.match(lastCommandStdOut_);

    bool commandHasFinishedSuccessfully = false;
    switch(currentCommand_->getType())
    {
      case shellCommand: // fall through
      case pollingShellCommand:
        if (currentCommand_->getShellCommand()->IgnoreErrors)
        {
          commandHasFinishedSuccessfully = true;
        }
        else if (parExitStatus == QProcess::NormalExit && parExitCode == 0 && match.hasMatch())
        {
          commandHasFinishedSuccessfully = true;
        }
        break;

      case sleepCommand:
        commandHasFinishedSuccessfully = true;
        break;

      case bootUpDetection:
         commandHasFinishedSuccessfully = currentCommand_->hasBootUpFinished();
        break;

      case removeDirCommand:
        commandHasFinishedSuccessfully = true;
        break;
    }

    if (commandHasFinishedSuccessfully)
    {
      if (currentCommand_->getType() == sleepCommand)
        signalWriteSuccess("sleep finished\n");
      else if (currentCommand_->getType() == bootUpDetection)
        signalWriteSuccess("boot up finished successfully\n");
      else if (currentCommand_->getType() == removeDirCommand)
        signalWriteSuccess("removeDirCommand finished successfully\n");
      else if (currentCommand_->getShellCommand()->IgnoreErrors)
        signalWriteSuccess("command finished (without error check)\n");
      else
        signalWriteSuccess("command finished successfully\n");
    }
    else
    {
      // Should we poll again?
      if ( currentCommand_->getType() == bootUpDetection ||
           (currentCommand_->getType() == pollingShellCommand && currentCommand_->shouldRetryInOneSecond()))
      {
        // update the progress bar and continue polling
        costsFinished_ += currentCommand_->popExecutionCosts();
        emit signalUpdateProgress();

        // start the same command again, take the command's timeout
        QTimer::singleShot( currentCommand_->getTimeoutMilliseconds(), this, SLOT( slotStartNextCommand() ) );
        return;
      }
    }

    // all types of commands have finished now

    if (!commandHasFinishedSuccessfully)
    {
      writeFromStandardErr("command failed: '" + currentCommand_->getDescription() + "', exitCode: " + QString::number(parExitCode) + "\n");

      // remove other commands and free memory
      while (commands_.size())
        delete commands_.takeFirst();

      running_ = false;
      internalAborted_ = true;
      emit signalFinished(failed);
      return;
    }

    // update the progress bar
    costsFinished_ += currentCommand_->popAllExecutionCosts();
    signalUpdateProgress();

    // delete the current command, because it has finished successfully
    delete commands_.takeFirst();
  }

  // start the next command
  if (commands_.size())
  {
    slotStartNextCommand();
  }
  else
  {
    running_ = false;
    // all commands executed, caller can delete this Widget
    emit signalFinished(success); 
  }
}
