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

#include "utils.h"
#include "WidgetCommandExec.h"

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QCloseEvent>

namespace Ui {
  class WidgetUpdate;
}

// forward declaration
class PmManager;
class PmCommand;

class WidgetUpdate : public QWidget
{
    Q_OBJECT
    
  public:
    explicit WidgetUpdate(QWidget *parent = NULL);
    ~WidgetUpdate();

    bool init(PmManager* manager);
    void start();
    void abort();

   signals:
    void signalUpdateFinished(ePmCommandResult result); // used for communication with frmMainWindow

  protected:
      // override from QWidget
      void closeEvent(QCloseEvent *event);

  private:
    Ui::WidgetUpdate *ui_;
    WidgetCommandExec* widgetCommandExec_;
    PmManager* manager_;

  private slots:
    void slotCommandsFinished(ePmCommandResult result); // used for communication with frmCommandExec
};
