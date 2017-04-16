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

#include <QString>

// Singelton used for Logging
// Singelton-Pattern from http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class PmLog
{
  public:
    static PmLog& getInstance()
    {
      static PmLog instance_; // Guaranteed to be destroyed. Instantiated on first use.
      return instance_;
    }

    PmLog(PmLog const&)           = delete; // not allowed for Singeltons (feature of C++11)
    void operator=(PmLog const&)  = delete; // not allowed for Singeltons (feature of C++11)

    /// this function will be used by qDebug(), qWarning(), qCritical(), qFatal()
    void logMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    /// Cycles between log files so that previous sessions are not overwritten immediately.
    bool initAndRotateLogfiles(QString parUserConfigDir);

    void enableSensitiveLogging() { sensitiveLogging_ = true; }
    bool isSensitiveLoggingEnabled() { return sensitiveLogging_; }

  private:
    PmLog();

    QString logFileName_;
    bool sensitiveLogging_; // used for logging username/passwords (can be enabled per commandline)
};

