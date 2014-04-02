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
#include "PMCommand.h"
#include "WidgetCommandExec.h"
#include "ui_WidgetCommandExec.h"

#include <QMessageBox>

/*---------------------------------------------*/
/*                PUBLIC:                      */

WidgetCommandExec::WidgetCommandExec(QWidget *parParent) :
  QWidget(parParent),
  ui_(new Ui::WidgetCommandExec)
{
  ui_->setupUi(this);
  ui_->btnAbort->setEnabled(false);
  ui_->txtDetailMessages->setVisible(false);
  ui_->progressBar->setValue(0);
  ui_->minOutputLabel->setText("");

  connect(ui_->btnDetails,
          SIGNAL(clicked()),
          this,
          SLOT(btnDetails_clicked()));
}

WidgetCommandExec::~WidgetCommandExec()
{
  delete ui_;
}

bool WidgetCommandExec::init(QList<PMCommand *>& parAllCommands)
{
  allCommands_ = parAllCommands; // Copy the Pointers, we delete them here when we are finished
  lastCommandStdOut_ = "";
  lastCommandStdErr_ = "";
  lastCommandExitCode_ = 0;
  costsAll_ = 0;
  costsFinished_ = 0;
  userAborted_=false;

  ui_->txtDetailMessages->setVisible(false);

  connect(ui_->btnAbort,
          SIGNAL(clicked()),
          this,
          SLOT(btnAbort_clicked()));

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

  foreach( PMCommand* cmd, allCommands_)
    costsAll_ += cmd->getAllCosts();

  ui_->progressBar->setMinimum(0);
  ui_->progressBar->setMaximum(costsAll_);

  return true;
}

void WidgetCommandExec::reset()
{
  ui_->progressBar->reset();
  ui_->txtDetailMessages->clear();
  ui_->txtDetailMessages->setVisible(false);
  ui_->minOutputLabel->setText("");
}

void WidgetCommandExec::start()
{
  // start executing the first command
  ui_->btnAbort->setEnabled(true);
  slotStartNextCommand();
}

void WidgetCommandExec::abort()
{
  userAborted_=true;
  proc_.kill();
  ui_->btnAbort->setEnabled(false);
  writeFromStandardErr("user aborted process");
  // skip other commands
  while (allCommands_.size())
    delete allCommands_.takeFirst();

}


/*---------------------------------------------*/
/*                PRIVATE:                     */

void WidgetCommandExec::writeFromStandardOut(QString parOut)
{
  ILOG(removeLastLF(parOut));

  allStdout_ += parOut;
  lastCommandStdOut_ += parOut;

  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::black);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);
  ui_->txtDetailMessages->ensureCursorVisible();
}

void WidgetCommandExec::writeFromStandardErr(QString parOut)
{
  IERR(removeLastLF(parOut));

  allStderr_ += parOut;
  lastCommandStdErr_ += parOut;

  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::red);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);
  ui_->txtDetailMessages->ensureCursorVisible();
}

QString WidgetCommandExec::removeLastLF(QString parMsg)
{
  if (parMsg.length() == 0)
    return "";

  if (parMsg[parMsg.length()-1] == '\n')
    return parMsg.left(parMsg.length()-1);
  else
    return parMsg;
}

void WidgetCommandExec::writeSuccess(QString parOut)
{
  ILOG(removeLastLF(parOut));

  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::darkGreen);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);

  ui_->txtDetailMessages->ensureCursorVisible();
}

/*---------------------------------------------*/
/*                SLOTS:                       */

void WidgetCommandExec::slotStartNextCommand()
{
  if (!allCommands_.size()) return;

  PMCommand* curCommand = allCommands_.front();
  ui_->minOutputLabel->setText(curCommand->getDescription());

  writeFromStandardOut(currentTimeStampAsISO() + " Starting: " + curCommand->getDescription() + "\n");
  if (!curCommand->start(&proc_, this))
  {
    QString msg = "Start of '" + curCommand->getDescription() + "' failed";
    IERR(msg);
    QMessageBox::warning(this, "", msg);
  }
}

void WidgetCommandExec::btnAbort_clicked()
{
  abort();
  emit signalFinished(aborted);
}

void WidgetCommandExec::btnDetails_clicked()
{
  ui_->txtDetailMessages->setVisible(!ui_->txtDetailMessages->isVisible());
}

void WidgetCommandExec::slotReadyReadStandardOutput()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  proc_.setReadChannel(QProcess::StandardOutput);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (allCommands_.size())
  {
    if (allCommands_.front()->getShellCommand()->showStdOutAsStdErr)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardErr(newOutput);
  else
    writeFromStandardOut(newOutput);
}

void WidgetCommandExec::slotProcessReadyReadStandardError()
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  proc_.setReadChannel(QProcess::StandardError);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  bool swapOutput = false;
  if (allCommands_.size())
  {
    if (allCommands_.front()->getShellCommand()->showStdErrAsStdOut)
      swapOutput = true;
  }

  if (swapOutput)
    writeFromStandardOut(newOutput);
  else
    writeFromStandardErr(newOutput);
}

void WidgetCommandExec::slotCommandFinished()
{
  // this slot is usualy used by a timer
  slotProcessFinished(0, QProcess::NormalExit);
}

void WidgetCommandExec::updateProgress()
{
  ui_->progressBar->setValue(costsFinished_);
  ui_->progressBar->update();

  // send window title via signal
  if (costsAll_ > 0)
  {
    float percent =  costsFinished_ / (float)costsAll_ * 100;
    QString title = QString("finished: %1%").arg(QString::number((int)percent));
    emit signalUpdateProgress(title);
  }
}

void WidgetCommandExec::slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus)
{
  if (userAborted_) return; // can occour if some messages are in the queue after the user has aborted

  if (allCommands_.size())
  {
    PMCommand* curCommand = allCommands_.front();

    if (curCommand->retryInOneSecond())
    {
      // update the progress bar
      costsFinished_ += curCommand->popCosts();
      updateProgress();

      // start the same command again in a second
      QTimer::singleShot(1000, this, SLOT(slotStartNextCommand()));
      return;
    }

    // update the progress bar
    costsFinished_ += curCommand->popAllCosts();
    updateProgress();

    if (curCommand->getType() == sleepCommand || curCommand->getShellCommand()->ignoreErrors)
    {
      writeSuccess("command finished\n");
    }
    else
    {
      if (parExitStatus == QProcess::NormalExit && parExitCode == 0)
      {
        writeSuccess("command finished successfull\n");
      }
      else
      {
        writeFromStandardErr("command failed\n");
        // skip other commands
        while (allCommands_.size())
          delete allCommands_.takeFirst();

        emit signalFinished(failed);
        return;
      }
    }

    // delete the old command
    delete allCommands_.takeFirst();
  }

  // start the next command
  if (allCommands_.size())
  {
    slotStartNextCommand();
  }
  else
  {
    // all commands executed
    ui_->btnAbort->setEnabled(false);
    emit signalFinished(success); // caller can delete this Widget
  }
}

/*---------------------------------------------*/
