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

#include <QString>
#include <QVariant>
#include <QProcess>
#include <QTimer>

struct PMShellCommand
{
    QString cmd;
    QStringList args;
    bool showStdErrAsStdOut;
    bool showStdOutAsStdErr;
    bool ignoreErrors;
};

enum eCommandType
{
  shellCommand = 1,
  bootUpDetection = 2,
  sleepCommand = 3
};

class PMCommand
{
  public:
    // init as ShellCommand
    PMCommand(QString parCmd , QStringList& parArgs, bool parShowStdErrAsStdOut, bool parShowStdOutAsStdErr);

    // init as BootUpDetection
    PMCommand(QString parCmd, QStringList& parArgs, QString parPngFileName);

    // init as Sleep
    PMCommand(int parMillisec);

    virtual ~PMCommand();

    bool executeBlocking(bool parCheckExitCode);

    // getter/setter
    eCommandType getType() { return type_; }
    QString getDescription() { return description_; }
    void setDescription(QString desc) { description_ = desc; }

    bool retryInOneSecond();
    bool start(QProcess* parProc, QObject* parReceiver);

    // for progressbar
    void setCosts(int parCost) { cost_ = parCost; }
    int getAllCosts() { return cost_; }
    int popAllCosts();
    int popCosts();

    PMShellCommand* getShellCommand() { return shellCmd_; }

  private:
    eCommandType type_;
    QString description_;
    PMShellCommand* shellCmd_;
    QString pngFileName_;
    int sleeptime_;
    int cost_; // used for progressbars
};
