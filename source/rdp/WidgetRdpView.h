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

#include <QUrl>
#include <QProcess>
#ifdef PM_WINDOWS
  // a hack because we have no implementation for windows
  #include <QWidget>
  #define QX11EmbedContainer QWidget
#else
#include <QX11EmbedContainer>
#endif

class WidgetRdpView : public QX11EmbedContainer
{
    Q_OBJECT

  public:
    WidgetRdpView(QUrl parUrl);
    virtual ~WidgetRdpView();

    void connectViaRdp();

  private:
    QUrl url_;
    QProcess proc_;

  private slots:

    // get the results from QProcess:
    void slotReadyReadStandardOutput();
    void slotProcessReadyReadStandardError();
    void slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus);

};
