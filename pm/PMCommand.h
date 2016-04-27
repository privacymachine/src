/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

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
  sleepCommand = 3,
  /// Poll commands are repeatedly executed until 
  /// a) The command for the current iteration succeeds.
  ///    This means that the command succeeded overall.
  /// b) The maximum poll count is reached.
  ///    This means that the command failed overall.
  /// Use this command type e.g. for attempting to connect to a virtual machine using ssh until it finally listens for
  /// ssh requests.
  pollingShellCommand = 4
};

// XXX AL: Extend with signals for command completion, with return code as argument
// This way, command sequences as well as arbitrarily complex graphs can be modeled:
// slots handle the return code, and run the next command based on the return code.
// XXX AL: Provide default success/fail implementations (run command, abort) for slots

/// Convenience class for managing a command line call, including tweaks like swapping
/// StdOut/StdErr, if commands swapped it accidentally.
class PMCommand
{
  public:
    // Init as (ordinary) ShellCommand.
    PMCommand(QString parCmd , QStringList& parArgs, bool parShowStdErrAsStdOut, bool parShowStdOutAsStdErr);

    /// Init as BootUpDetection.
    /// Waits until virtual machine booted, then executes command.
    PMCommand(QString parCmd, QStringList& parArgs, QString parPngFileName);

    // init as Sleep
    PMCommand(int parMillisec);

    virtual ~PMCommand();

    bool executeBlocking(bool parCheckExitCode);

    // getter/setter
    QString getDescription() { return description_; }
    void setDescription(QString desc) { description_ = desc; }

    bool getIgnoreErrors() { return shellCmd_->ignoreErrors; }
    void setIgnoreErrors( bool ignoreErrors ) { shellCmd_->ignoreErrors = ignoreErrors; }

    // A regex string associated with the command. This can be used by the caller to validate command output, e.g.
    QString getRegexPattern() { return regexPattern_; }
    void setRegexPattern( QString parRegexPattern ) { regexPattern_ = parRegexPattern; }

    uint getRetries() { return retries_; }
    void setRetries( uint parRetries ) { retries_ = parRetries; }

    uint getTimeoutMilliseconds() { return timeoutMilliseconds_; }
    void setTimeoutMilliseconds( uint parTimeoutMilliseconds ) { timeoutMilliseconds_ = parTimeoutMilliseconds; }

    eCommandType getType() { return type_; }
    void setType( eCommandType parType ) { type_ = parType; }


    /// Tells the caller if the command wants to be called again.
    bool shouldRetryInOneSecond();
    bool start(QProcess* parProc, QObject* parReceiver);

    // for progressbar
    void setCosts(int parCost) { cost_ = parCost; }
    int getAllCosts() { return cost_; }
    int popAllCosts();
    int popCosts();

    PMShellCommand* getShellCommand() { return shellCmd_; }

  private:
    int cost_; // used for progressbars
    QString description_;
    PMShellCommand* shellCmd_;
    QString pngFileName_;
    QString regexPattern_;
    uint retries_;
    uint timeoutMilliseconds_;
    eCommandType type_;

};
