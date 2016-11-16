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

#include <algorithm>
#include <ctime>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QSharedPointer>
#include <QTimer>
#include <vector>

#include "PMCommandExec.h"
#include "PMInstance.h"
#include "utils.h"

using std::vector;
using std::string;

class PMInstance;
class PMCommandExec;

/// Provides the IP address for a specific VM
/// This is the first class that asynchronously gets information out of a VM (beware security leaks!). 
/// It does so using a polling timer (triggered by startPollingExternalIp()), which in turn triggers PMCommands. When
/// finished, these PMCommands cause state updates. Once all state updates have been performed, we signal it to the
/// outside world, using signalUpdateIpSuccess() in this case.
class VmInfoIpAddress: public QObject
{
  Q_OBJECT

  public:
    /// \param configVpn At the moment, the only ConfigVPN::vpnType supported is OpenVPN
    VmInfoIpAddress( PMInstance *pPmInstance );

    virtual ~VmInfoIpAddress();

    QString getIpAddress();

    /// Initializations that go beyond trivial member initialization as performed in the constructor. Non-trivial are
    /// e.g. connecting signals and slots.
    void initialize();

    /// Returns a decorated string representation for use in a status bar, e.g.
    QString toStatus();

    /// Starts polling for the external IP address within the VM. Polling stops after the IP address could be determined
    /// successfully, which will only happen after the browser inside the VM has been started.
    void startPollingExternalIp();

    void abort();

  signals:
    /// Signalled after both the browser inside the VM has been started and the external IP address could be determined
    /// successfully.
    void signalUpdateIpSuccess();


  private:
    void randomizeIpAddressProviders();

    PMCommand *createPmCommandNextIpAddressProvider();

    /// Contains the VM's IP Address, if it could be obtained successfully. \c"", otherwise.
    QString ipAddress_;

    /// \c true if ipAddress_ is supposed to contain an up-to-date value. \c false otherwise.
    bool ipAddressUpdated_;

    unsigned nProvidersTried_;

    QList<unsigned> ipAddressProviderPermutation_;

    /// The VM for which we attempt to obtain the IP Address.
    PMInstance *pPmInstance_;

    /// exec_ takes care of running and evaluating all command-line commands for us.
    QSharedPointer< PMCommandExec > exec_;


  private slots:
    void slotExecFinished( CommandResult result );

};

