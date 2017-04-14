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

#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVariant>


struct PmShellCommand
{
    QString Cmd;
    QStringList Args;
    bool ShowStdErrAsStdOut;
    bool ShowStdOutAsStdErr;
    bool IgnoreErrors;
};

enum ePmCommandType
{
  shellCommand = 1,
  /// Poll commands are repeatedly executed until
  /// a) The command for the current iteration succeeds.
  ///    This means that the command succeeded overall.
  /// b) The maximum poll count is reached.
  ///    This means that the command failed overall.
  /// Use this command type e.g. for attempting to connect to a virtual machine using ssh until it finally listens for
  /// ssh requests.
  pollingShellCommand = 2,
  sleepCommand = 3,
  bootUpDetection = 4,
  removeDirCommand = 5
};

enum ePmCommandResult
{
  success=0,
  aborted=1,
  failed=2
};

/// TODO AL: Extend with signals for command completion, with return code as argument
/// This way, command sequences as well as arbitrarily complex graphs can be modeled:
/// slots handle the return code, and run the next command based on the return code.
/// TODO AL: Provide default success/fail implementations (run command, abort) for slots

/// Convenience class for managing a command line call, including tweaks like swapping
/// StdOut/StdErr, if commands swapped it accidentally.
class PmCommand
{
  public:
    /// Init as (ordinary) ShellCommand.
    PmCommand(QString parCmd , QStringList& parArgs, bool parShowStdErrAsStdOut, bool parShowStdOutAsStdErr, QString parDesc, bool parHideStdOut = false);

    /// Init as BootUpDetection.
    /// Waits until virtual machine booted, then executes command.
    PmCommand(QString parCmd, QStringList& parArgs, QString parPngFileName, QString parDesc);

    /// Init as Sleep
    PmCommand(int parMillisec);

    /// Init as RemoveDirCommand
    PmCommand(QString parFolder);

    virtual ~PmCommand();

    /// execute and wait till the command finishes
    bool executeBlocking(bool parCheckExitCode);

    /// getter/setter

    QString getDescription() { return description_; }
    void setDescription(QString desc) { description_ = desc; }

    bool getIgnoreErrors();
    void setIgnoreErrors( bool ignoreErrors );

    bool getHideStdOut() { return hideStdOut_; }

    /// A regex string associated with the command. This can be used by the caller to validate command output, e.g.
    QString getRegexPattern() { return regexPattern_; }
    void setRegexPattern( QString parRegexPattern ) { regexPattern_ = parRegexPattern; }

    uint getRetries() { return retries_; }
    void setRetries( uint parRetries ) { retries_ = parRetries; }

    uint getTimeoutMilliseconds() { return timeoutMilliseconds_; }
    void setTimeoutMilliseconds( uint parTimeoutMilliseconds ) { timeoutMilliseconds_ = parTimeoutMilliseconds; }

    ePmCommandType getType() { return type_; }
    void setType( ePmCommandType parType ) { type_ = parType; }


    /// Tells the caller if the command wants to be called again.
    bool shouldRetryInOneSecond();
    bool start(QProcess* parProc, QObject* parReceiver);

    bool hasBootUpFinished();

    /// for progressbar
    void setExecutionCosts(int parExecutionCost) { executionCost_ = parExecutionCost; }
    int getAllExecutionCosts() { return executionCost_; }
    int popAllExecutionCosts();
    int popExecutionCosts();

    PmShellCommand* getShellCommand() { return shellCmd_; }

  private:

    void executeRemoveDirCommand();

    int executionCost_; // used for progressbars
    QString description_;
    PmShellCommand* shellCmd_;
    QString pngFileName_;
    QString regexPattern_;
    uint retries_;
    uint timeoutMilliseconds_;
    ePmCommandType type_;
    QString folderName_;
    bool hideStdOut_;
};
