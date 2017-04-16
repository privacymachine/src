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

#include "WidgetCommandExec.h"

/*---------------------------------------------*/
/*                PUBLIC:                      */

WidgetCommandExec::WidgetCommandExec(QWidget *parParent) :
  QWidget(parParent),
  ui_(new Ui::WidgetCommandExec)
{
  // Smart pointer might be a slight overkill here, but should increase robustnes nevertheless.
  exec_ = QSharedPointer< PmCommandExec >( new PmCommandExec() );
  ui_->setupUi(this);
  ui_->btnAbort->setEnabled(false);
  ui_->txtDetailMessages->setVisible(false);
  ui_->progressBar->setValue(0);
  ui_->minOutputLabel->setText("");
  // From https://stackoverflow.com/questions/10082299/qvboxlayout-how-to-vertically-align-widgets-to-the-top-instead-of-the-center
  ui_->verticalLayout->setAlignment( Qt::AlignTop );

  connect(ui_->btnDetails,
          SIGNAL(clicked()),
          this,
          SLOT(slotBtnDetails_clicked()));
}


WidgetCommandExec::~WidgetCommandExec()
{
  disconnectSignalAndSlots();
  delete ui_;
}


void WidgetCommandExec::connectSignalsAndSlots()
{
  connect(ui_->btnAbort,
          SIGNAL(clicked()),
          this,
          SLOT(slotBtnAbort_clicked()));

  if( exec_ )
  {
    connect(
      &(*exec_),
      SIGNAL( signalStartingNextCommand() ),
      this,
      SLOT( slotStartingNextCommand() ) );

    connect(
      &(*exec_),
      SIGNAL( signalStartingNextCommandFailed() ),
      this,
      SLOT( slotStartingNextCommandFailed() ) );

    connect(
      &(*exec_),
      SIGNAL( signalFinished( ePmCommandResult ) ),
      this,
      SLOT( slotFinished( ePmCommandResult ) ) );

    connect(
      &(*exec_),
      SIGNAL( signalUpdateProgress() ),
      this,
      SLOT( slotUpdateProgress() ) );

    connect(
      &(*exec_),
      SIGNAL( signalWriteSuccess( QString ) ),
      this,
      SLOT( slotWriteSuccess( QString ) ) );

    connect(
      &(*exec_),
      SIGNAL( signalWriteFromStandardErr( QString ) ),
      this,
      SLOT( slotWriteFromStandardErr( QString ) ) );

    connect(
      &(*exec_),
      SIGNAL( signalWriteFromStandardOut( QString ) ),
      this,
      SLOT( slotWriteFromStandardOut( QString ) ) );

    exec_->connectSignalsAndSlots();

  }
  else
  {
    IERR( "Command executor not defined. Cannot connect signals." );

  }

}


void WidgetCommandExec::disconnectSignalAndSlots()
{ 
  // https://stackoverflow.com/questions/28524925/how-to-disconnect-a-signal-with-a-slot-temporarily-in-qt
  disconnect(ui_->btnAbort,
          SIGNAL(clicked()),
          0,
          0);

  if( exec_ )
  {
    disconnect(
      &(*exec_),
      SIGNAL( signalStartingNextCommand() ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalStartingNextCommandFailed() ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalFinished( ePmCommandResult ) ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalUpdateProgress() ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalWriteSuccess( QString ) ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalWriteFromStandardErr( QString ) ),
      0,
      0 );

    disconnect(
      &(*exec_),
      SIGNAL( signalWriteFromStandardOut( QString ) ),
      0,
      0 );

    exec_->disconnectSignalsAndSlots();

  }
  else
  {
    IERR( "Command executor not defined. Cannot disconnect signals." );

  }

}

bool WidgetCommandExec::isStillExecuting()
{
  return exec_ && exec_->isRunning();
}


bool WidgetCommandExec::setCommands(QList<PmCommand *>& parAllCommands)
{
  if( !exec_ )
  {
    IERR( "Failed to set up commands for execution. Command executor not defined." );
    return false;
  }
  
  ui_->txtDetailMessages->setVisible(false);
  
  bool success = exec_->setCommands( parAllCommands );
  ui_->progressBar->setMinimum(0);
  ui_->progressBar->setMaximum( exec_->getCostsAll() );

  return success;

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
  exec_->start();
}


void WidgetCommandExec::abort()
{
  ui_->btnAbort->setEnabled(false);

  ui_->txtDetailMessages->clear();
  ui_->progressBar->setValue(0);
  ui_->minOutputLabel->setText("");
  if( exec_ )
  {
    exec_->abort();

  }
  // If we cannot expect to get a signalFinished() from exec_, Inform attached slots anyway:
  else
  {
    //emit signalFinished( abort );

  }

}


void WidgetCommandExec::slotWriteFromStandardErr(QString parOut)
{
  IERR(removeLastLF(parOut));

  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::red);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);
  ui_->txtDetailMessages->ensureCursorVisible();
}

void WidgetCommandExec::slotWriteFromStandardOut(QString parOut)
{
  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::black);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);
  ui_->txtDetailMessages->ensureCursorVisible();
}

/*---------------------------------------------*/
/*                SLOTS:                       */

void WidgetCommandExec::slotStartingNextCommand()
{
  ui_->minOutputLabel->setText( exec_->getCurrentCommand()->getDescription() );

}


void WidgetCommandExec::slotStartingNextCommandFailed()
{

  QString msg = "Start of";
  if( exec_ && ( exec_->getCurrentCommand() != NULL ) )
  {
    msg += " '" + exec_->getCurrentCommand()->getDescription() + "'";

  }
  else
  {
    msg += " current command";

  }
  
  msg += " failed";
  IERR(msg);
  QMessageBox::warning(this, "", msg);

}


void WidgetCommandExec::slotBtnAbort_clicked()
{
  abort();

}


void WidgetCommandExec::slotBtnDetails_clicked()
{
  ui_->txtDetailMessages->setVisible(!ui_->txtDetailMessages->isVisible());
}


void WidgetCommandExec::slotFinished( ePmCommandResult parResult )
{
  ui_->btnAbort->setEnabled(false);
  emit signalFinished( parResult );
}


void WidgetCommandExec::slotUpdateProgress()
{
  int costsFinished = 0;
  int costsAll = 0;
  if( exec_ )
  {
    ui_->progressBar->setValue( exec_->getCostsFinished() );
    ui_->progressBar->update();

    // send window title via signal
    if( exec_->getCostsAll() > 0 )
    {
      float percent =  exec_->getCostsFinished() / (float)exec_->getCostsAll() * 100;
      QString title = QString("finished: %1%").arg(QString::number((int)percent));
      emit signalUpdateProgress(title);

    }

  }
  else
  {
    IERR( "Failed to determine current progress. Command executor not defined." );

  }

}


void WidgetCommandExec::slotWriteSuccess(QString parOut)
{
  QTextCursor curCursor = ui_->txtDetailMessages->textCursor();
  curCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  QTextCharFormat curFormat = curCursor.charFormat();
  curFormat.setForeground(Qt::darkGreen);
  curCursor.setCharFormat(curFormat);

  curCursor.insertText(parOut);

  ui_->txtDetailMessages->ensureCursorVisible();
}


/*---------------------------------------------*/
