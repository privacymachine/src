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
#include <QObject>
#include <QProcess>
#include <QRegExp>
#include <QTimer>

#include "PMCommand.h"
#include "utils.h"


enum CommandResult{
  success=0,
  aborted=1,
  failed=2
};


/// Executes a given list of commands. To be used for all command line invocations.
class PMCommandExec : public QObject
{
    Q_OBJECT
    
  public:
    explicit PMCommandExec();
    ~PMCommandExec();
    void connectSignalsAndSlots();
    void disconnectSignalsAndSlots();
    int getCostsAll();
    int getCostsFinished();
    /// \return The command that is scheduled to be run next or is currently being run. Might be \c NULL.
    PMCommand* getCurrentCommand();
    bool getRunning();
    QString getLastCommandLastLineStdErr();
    QString getLastCommandLastLineStdOut();
    QString getLastCommandStdErr();
    QString getLastCommandStdOut();
    bool setCommands(QList<PMCommand*>& parAllCommands);
    void start();
    void abort();
    void writeFromStandardOut( QString parOut );
    void writeFromStandardErr( QString parOut );
    QString removeLastLF(QString msg);

  signals:
    void signalStartingNextCommand();
    void signalStartingNextCommandFailed();
    void signalFinished( CommandResult result );
    void signalUpdateProgress();
    void signalWriteSuccess( QString message );
    void signalWriteFromStandardOut( QString parOut );
    void signalWriteFromStandardErr( QString parOut );


  private:
    QString allStderr_;
    QString allStdout_;
    QList<PMCommand*> commands_;
    int costsAll_;
    int costsFinished_;
    PMCommand* currentCommand_;
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

