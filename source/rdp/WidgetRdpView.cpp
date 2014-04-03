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

#include "WidgetRdpView.h"
#include "../utils.h"

WidgetRdpView::WidgetRdpView(QUrl parUrl)
{
  url_ = parUrl;
}

WidgetRdpView::~WidgetRdpView()
{
  // TODO: implement me!
}

void WidgetRdpView::connectViaRdp()
{
  connect(&proc_,
          SIGNAL(readyReadStandardOutput()),
          this,
          SLOT(slotReadyReadStandardOutput()));

  connect(&proc_,
          SIGNAL(readyReadStandardError()),
          this,
          SLOT(slotProcessReadyReadStandardError()));

  connect(&proc_,
          SIGNAL(finished(int,QProcess::ExitStatus)),
          this,
          SLOT(slotProcessFinished(int,QProcess::ExitStatus)));


  // HACK! + memory leak

  installEventFilter(this); // ??

  show();

  QString executable= "./xfreerdp"; // use link to version 1.2 in build directory!
  QStringList arguments;

  arguments << QString("/parent-window:0x") + QString::number((ulong)winId(),16);
  arguments << "/u:pm";
  arguments << "/p:123";
  arguments << QString("/v:") + url_.host() + QString(":") + QString::number(url_.port());
  qDebug() << arguments;
  proc_.start(executable,arguments);
}

void WidgetRdpView::slotReadyReadStandardOutput()
{
  proc_.setReadChannel(QProcess::StandardOutput);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  ILOG(newOutput);
}

void WidgetRdpView::slotProcessReadyReadStandardError()
{
  proc_.setReadChannel(QProcess::StandardError);
  QString newOutput = proc_.read(proc_.bytesAvailable());

  ILOG(newOutput);
}

void WidgetRdpView::slotProcessFinished(int parExitCode, QProcess::ExitStatus parExitStatus)
{
}
