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
class PMManager;
class PMCommand;

class WidgetUpdate : public QWidget
{
    Q_OBJECT
    
  public:
    explicit WidgetUpdate(QWidget *parent = 0);
    ~WidgetUpdate();

    bool init(PMManager* manager);
    void start();
    void abort();

   signals:
    void signalUpdateFinished(commandResult result); // used for communication with frmMainWindow

  protected:
      // override from QWidget
      void closeEvent(QCloseEvent *event);

  private:
    Ui::WidgetUpdate *ui_;
    WidgetCommandExec* widgetCommandExec_;
    PMManager* manager_;

  private slots:
    void slotCommandsFinished(commandResult result); // used for communication with frmCommandExec
};
