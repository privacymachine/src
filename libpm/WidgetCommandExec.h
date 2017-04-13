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

#pragma once

#include <QMessageBox>
#include <QProcess>
#include <QRegExp>
#include <QSharedPointer>
#include <QTimer>
#include <QWidget>

#include "PmCommand.h"
#include "PmCommandExec.h"
#include "ui_WidgetCommandExec.h"
#include "utils.h"


namespace Ui {
  class WidgetCommandExec;
}

class PmCommand;

/// UI wrapper for PmCommandExec.
class WidgetCommandExec : public QWidget
{
  Q_OBJECT
    
  public:
    explicit WidgetCommandExec(QWidget *parParent = NULL);
    ~WidgetCommandExec();
    void abort();
    void connectSignalsAndSlots();
    bool isStillExecuting();
    void reset();
    bool setCommands( QList<PmCommand*>& parAllCommands );
    void start();


  signals:
    void signalUpdateProgress(QString title);
    void signalFinished( ePmCommandResult result );


  private:
    void disconnectSignalAndSlots();

    Ui::WidgetCommandExec *ui_;
    /// Handles command and process execution for us.
    QSharedPointer< PmCommandExec > exec_;


  private slots:
    void slotBtnAbort_clicked();
    void slotBtnDetails_clicked();
    void slotFinished( ePmCommandResult result );
    void slotStartingNextCommandFailed();
    void slotStartingNextCommand();
    void slotUpdateProgress();
    void slotWriteSuccess( QString message );
    void slotWriteFromStandardErr( QString parOut );
    void slotWriteFromStandardOut( QString parOut );

};

