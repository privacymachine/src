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

#pragma once

#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QTimer>

#include "PmCommand.h"
#include "utils.h"


/// Executes a given list of commands. To be used for all command line invocations.
class PmCommandExec : public QObject
{
    Q_OBJECT
    
  public:
    explicit PmCommandExec();
    ~PmCommandExec();
    void connectSignalsAndSlots();
    void disconnectSignalsAndSlots();
    int getCostsAll();
    int getCostsFinished();
    /// \return The command that is scheduled to be run next or is currently being run. Might be \c nullptr.
    PmCommand* getCurrentCommand();
    bool isRunning();
    QString getLastCommandLastLineStdErr();
    QString getLastCommandLastLineStdOut();
    QString getLastCommandStdErr();
    QString getLastCommandStdOut();
    bool setCommands(QList<PmCommand*>& parAllCommands);
    void start();
    void abort();
    void writeFromStandardOut( QString parOut );
    void writeFromStandardErr( QString parOut );
    QString removeLastLF(QString msg);

  signals:
    void signalStartingNextCommand();
    void signalStartingNextCommandFailed();
    void signalFinished( ePmCommandResult parResult );
    void signalUpdateProgress();
    void signalWriteSuccess( QString parMessage );
    void signalWriteFromStandardOut( QString parOut );
    void signalWriteFromStandardErr( QString parOut );

  private:
    QString allStderr_;
    QString allStdout_;
    QList<PmCommand*> commands_;
    int costsAll_;
    int costsFinished_;
    PmCommand* currentCommand_;
    bool internalAborted_;
    int lastCommandExitCode_;
    QString lastCommandStdErr_;
    QString lastCommandLastLineStdErr_;
    QString lastCommandStdOut_;
    QString lastCommandLastLineStdOut_;
    QProcess proc_;
    bool running_;
    bool userAborted_;


  private slots:
    void slotCommandFinished();
    void slotStartNextCommand();

    // get the results from QProcess:
    void slotReadyReadStandardOutput();
    void slotProcessReadyReadStandardError();
    void slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus);
};
