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

#include "WidgetUpdate.h"
#include "WidgetNewTab.h"
#include "PMManager.h"

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>

namespace Ui {
  class WindowMain;
}

class WindowMain : public QMainWindow
{
    Q_OBJECT
    
  public:
    explicit WindowMain(QWidget *parParent = 0);
    void show();
    bool init(QString pmInstallPath);
    ~WindowMain();
    
  private:
    Ui::WindowMain *ui_;

    PMManager *pmManager_;

    bool setupTabWidget();
    QTabWidget *tabWidget_;

    WidgetUpdate *updateWidget_;
    QDialogButtonBox *questionBox_;
    QLabel *updateMessage_;
    
    void statusBarUpdate( QWidget *widget );

  protected:
      // override from QWidget
      void closeEvent(QCloseEvent *event);

  signals:
    void signalVmMaskClosed( int vmMaskId );

  private slots:
    void slotUpdateFinished(commandResult parResult);
    void slotTabCloseRequested(int parIndex);
    void slotTabCurrentChanged(int parIndex);
    void slotUpdateBtnOK();
    void slotUpdateBtnLater();
    void slotNewVmMaskStarted(int parIndexVmMask);
    void slotRdpViewResize( QWidget* );
    void slotUpdateProgress(QString parProgress);
};
