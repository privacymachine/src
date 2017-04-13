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

#include "WidgetCommandExec.h"

#include <QWidget>
#include <QRadioButton>
#include <QList>
#include <QUrl>

namespace Ui {
  class WidgetNewTab;
}

class WidgetCommandExec;
class PmManager;
class VmMaskInstance;

class WidgetNewTab : public QWidget
{
    Q_OBJECT
    
  public:
    explicit WidgetNewTab(QWidget *parParent = NULL);
    bool init(PmManager *parPmManager);
    ~WidgetNewTab();
    
  private:

    void connectSignalsAndSlots();
    void disconnectSignalsAndSlots();
    bool startVmMask();

    Ui::WidgetNewTab *ui_;
    QWidget *m_parent_;
    WidgetCommandExec *commandExec_;
    PmManager *pmManager_;
    QButtonGroup* radioButtons_;
    int currentSelectedVmMaskId_;

  signals:
    void signalNewVmMaskReady(int parVmMaskId);

  private slots:
    void slotBtnStart_clicked();
    void slotRadioBtn_clicked();
    void slotFinished(ePmCommandResult parExitCode);

  public slots:
    void slotVmMaskClosed( int parVmMaskId );
};

