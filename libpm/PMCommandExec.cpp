/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
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

#include "PMCommandExec.h"

/*---------------------------------------------*/
/*                PUBLIC:                      */

PMCommandExec::PMCommandExec()
{ 
  running_ = false;
  currentCommand_ = NULL;

}


PMCommandExec::~PMCommandExec()
{
  disconnectSignalsAndSlots();
  proc_.kill();

}


void PMCommandExec::connectSignalsAndSlots()
{
  connect(&proc_,
          SIGNAL(readyReadStandardOutput()),
          this,
          SLOT(slotReadyReadStandardOutput()));

  connect(&proc_,
          SIGNAL(readyReadStandardError()),
          this,
          SLOT(slotProcessReadyReadStandardError()));

  connect(&proc_,
          SIGNAL(finished(int,QProcess::ExitStatus)),
          this,
          SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

}


void PMCommandExec::disconnectSignalsAndSlots()
{ 
  // https://stackoverflow.com/questions/28524925/how-to-disconnect-a-signal-with-a-slot-temporarily-in-qt
  disconnect(&proc_,
          SIGNAL(readyReadStandardOutput()),
          0,
          0);

  disconnect(&proc_,
          SIGNAL(readyReadStandardError()),
          0,
          0);

  disconnect(&proc_,
          SIGNAL(finished(int,QProcess::ExitStatus)),
          0,
          0);

}


int PMCommandExec::getCostsAll()
{
  return costsAll_;

}


int PMCommandExec::getCostsFinished()
{
  return costsFinished_;

}


PMCommand* PMCommandExec::getCurrentCommand()
{
  return currentCommand_;

}


QString PMCommandExec::getLastCommandLastLineStdErr()
{
  return lastCommandLastLineStdErr_;

}


QString PMCommandExec::getLastCommandLastLineStdOut()
{
  return lastCommandLastLineStdOut_;

}


QString PMCommandExec::getLastCommandStdErr()
{
  return lastCommandStdErr_;

}


QString PMCommandExec::getLastCommandStdOut()
{
  return lastCommandStdOut_;

}


bool PMCommandExec::getRunning()
{
  return running_;

}


bool PMCommandExec::setCommands( QList<PMCommand *>& parCommands )
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

  foreach( PMCommand* cmd, commands_ )
  {
    costsAll_ += cmd->getAllCosts();

  }

  return true;

}

void PMCommandExec::start()
{
  running_ = true;

  // start executing the first command
  slotStartNextCommand();
}

void PMCommandExec::abort()
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


void PMCommandExec::writeFromStandardErr( QString parOut )
{
  IERR(removeLastLF(parOut));

  allStderr_ += parOut;
  lastCommandStdErr_ += parOut;
  lastCommandLastLineStdErr_ = parOut;

  emit signalWriteFromStandardErr( parOut );

}


void PMCommandExec::writeFromStandardOut(QString parOut)
{
  ILOG(removeLastLF(parOut));

  allStdout_ += parOut;
  lastCommandStdOut_ += parOut;
  lastCommandLastLineStdOut_ = parOut;

  emit signalWriteFromStandardOut( parOut );

}


/*---------------------------------------------*/
/*                PRIVATE:                     */

QString PMCommandExec::removeLastLF(QString parMsg)
{
  if (parMsg.length() == 0)
    return "";

  if (parMsg[parMsg.length()-1] == '\n')
    return parMsg.left(parMsg.length()-1);
  else
    return parMsg;
}

/*---------------------------------------------*/
/*                SLOTS:                       */

void PMCommandExec::slotCommandFinished()
{
  // this slot is usualy used by a timer
  slotProcessFinished(0, QProcess::NormalExit);
}


void PMCommandExec::slotStartNextCommand()
{
  if( !commands_.size() )
  {
    return;
  }

  currentCommand_ = commands_.front();

  writeFromStandardOut(currentTimeStampAsISO() + " Starting: " + currentCommand_->getDescription() + "\n");
  emit signalStartingNextCommand();
  /*
  // TODO: Bernhard: remove this
  SleeperThread::msleep(1000);
  */
  if( !currentCommand_->start( &proc_, this ) )
  {
    emit signalStartingNextCommandFailed();

  }

}


void PMCommandExec::slotReadyReadStandardOutput()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  if (internalAborted_) return; // can occour if some messages are in the queue after an error has happened

  proc_.setReadChannel(QProcess::StandardOutput);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (commands_.size())
  {
    if (commands_.front()->getShellCommand()->showStdOutAsStdErr)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardErr(newOutput);
  else
    writeFromStandardOut(newOutput);
}


void PMCommandExec::slotProcessReadyReadStandardError()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  if (internalAborted_) return; // can occour if some messages are in the queue after an error has happened

  proc_.setReadChannel(QProcess::StandardError);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (commands_.size())
  {
    if (commands_.front()->getShellCommand()->showStdErrAsStdOut)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardOut(newOutput);
  else
    writeFromStandardErr(newOutput);
}


void PMCommandExec::slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus)
{ 
  // can occour if some messages are in the queue after the user has aborted
  if( userAborted_
  // can occour if some messages are in the queue after an error has happened
    || internalAborted_ ) 
  {
    return; 

  }

  if (commands_.size())
  {
    currentCommand_ = commands_.front();

    // Initializing pattern to "" will make the regex match by default, so that regex checks pass by default if there 
    // was no regex pattern given.
    QRegExp regex( "" );
    if( currentCommand_->getRegexPattern() != NULL )
    {
      regex.setPattern( currentCommand_->getRegexPattern() );

    }

    if( currentCommand_->shouldRetryInOneSecond()
      // Only repeat a pollingShellCommand if the last call failed.
      && !( 
        currentCommand_->getType() == pollingShellCommand
        && parExitStatus == QProcess::NormalExit  
        && parExitCode == 0 
        && regex.indexIn( lastCommandStdOut_ ) >= 0 ) )
    {
      // update the progress bar
      costsFinished_ += currentCommand_->popCosts();
      emit signalUpdateProgress();

      // start the same command again, take the command's timeout
      QTimer::singleShot( currentCommand_->getTimeoutMilliseconds(), this, SLOT( slotStartNextCommand() ) );
      return;
    }

    // update the progress bar
    costsFinished_ += currentCommand_->popAllCosts();
    signalUpdateProgress();

    if (currentCommand_->getType() == sleepCommand)
    {
      signalWriteSuccess("sleep finished\n");
    }
    else if ((currentCommand_->getType() == shellCommand ||
              currentCommand_->getType() == pollingShellCommand)
              && currentCommand_->getShellCommand()->ignoreErrors)
    {
      signalWriteSuccess("command finished (without error check)\n");
    }
    else
    {
      // we need a normal error check
      if (parExitStatus == QProcess::NormalExit && parExitCode == 0)
      {
        signalWriteSuccess("command finished successfully\n");
      }
      else
      {
        writeFromStandardErr("command failed: '" + currentCommand_->getDescription() +
                             "', exitCode: " + QString::number(parExitCode) + "\n");
        // skip other commands
        while (commands_.size())
          delete commands_.takeFirst();

        running_ = false;
        internalAborted_ = true;
        emit signalFinished(failed);
        return;
      }
    }

    // delete the old command
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


/*---------------------------------------------*/
