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

#include "WidgetCommandExec.h"

#include <QWidget>
#include <QRadioButton>
#include <QList>
#include <QUrl>

namespace Ui {
  class WidgetNewTab;
}

class WidgetCommandExec;
class PMManager;
class PMInstance;

class WidgetNewTab : public QWidget
{
    Q_OBJECT
    
  public:
    explicit WidgetNewTab(QWidget *parParent = 0);
    bool init(PMManager *parPMManager);
    ~WidgetNewTab();
    
  private:

    bool startUsecase();
    void reset();

    Ui::WidgetNewTab *ui_;
    QWidget *m_parent_;
    WidgetCommandExec *commandExec_;
    PMManager *pmManager_;
    QButtonGroup* radioButtons_;

  signals:
    void newUseCaseReady(int indexUseCase);

  private slots:
    void btnStart_clicked();
    void radioBtn_clicked();
    void handleCommandStatus(commandResult exitCode);
};

