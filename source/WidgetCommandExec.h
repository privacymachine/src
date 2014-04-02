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

#pragma once

#include <QWidget>
#include <QProcess>
#include <QTimer>

namespace Ui {
  class WidgetCommandExec;
}

enum commandResult{
  success=0,
  aborted=1,
  failed=2
};


class PMCommand;

class WidgetCommandExec : public QWidget
{
    Q_OBJECT
    
  public:
    explicit WidgetCommandExec(QWidget *parParent = 0);
    ~WidgetCommandExec();
    bool init(QList<PMCommand*>& parAllCommands);
    void start();
    void abort();
    void reset();

  signals:
    void signalFinished(commandResult result);
    void signalUpdateProgress(QString title);


  private:
    Ui::WidgetCommandExec *ui_;

    void writeFromStandardOut(QString parOut);
    void writeFromStandardErr(QString parOut);
    QString removeLastLF(QString msg);
    void writeSuccess(QString parOut);
    void updateProgress();

    QProcess proc_;
    QString allStdout_;
    QString allStderr_;
    bool userAborted_;
    QString lastCommandStdOut_;
    QString lastCommandStdErr_;
    int lastCommandExitCode_;
    QList<PMCommand*> allCommands_;
    int costsAll_;
    int costsFinished_;

  private slots:

    void slotStartNextCommand();
    void slotCommandFinished();
    void btnAbort_clicked();
    void btnDetails_clicked();

    // get the results from QProcess:
    void slotReadyReadStandardOutput();
    void slotProcessReadyReadStandardError();
    void slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus);

};

