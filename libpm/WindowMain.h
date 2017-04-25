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

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QMessageBox>
#include <QVector>

#include "WidgetAbout.h"
#include "WidgetUpdate.h"
#include "WidgetNewTab.h"
#include "PmManager.h"
#include "UpdateManager.h"
#include "utils.h"
#include "VmInfoIpAddress.h"

namespace Ui {
  class WindowMain;
}


class WindowMain : public QMainWindow
{
    Q_OBJECT
    
  public:
    explicit WindowMain(QWidget *parParent = NULL);
    void show();
    bool init(QString parPmInstallPath, QString parVboxDefaultMachineFolder);
    ~WindowMain();

    
  private:
    bool setupTabWidget();
    void statusBarUpdate();
    void regenerateVmMasks();
    void cleanVmMasksBlocking();

    bool showReleaseNotes(QString parUrl, ulong parTimeoutInMilliseconds);

    PmManager *pmManager_;
    UpdateManager *updateManager_;

    QWidget* currentWidget_;
    QDialogButtonBox *questionBox_;
    Ui::WindowMain *ui_;
    QLabel *updateMessage_;
    WidgetUpdate *regenerationWidget_;
    QTabWidget *tabWidget_;
    WidgetAbout *aboutWidget_;

  protected:
    // override from QWidget
    void closeEvent(QCloseEvent *event);


  signals:
    void signalVmMaskClosed( int parVmMaskId );


  private slots:
    void slotNewVmMaskStarted(int parVmMaskId);
    void slotRdpViewScreenResize( QWidget* );
    void slotTabCloseRequested(int parTabIndex);
    void slotTabCurrentChanged(int parTabIndex);


    void slotCleanAllVmMasks();
    void slotRegenerationFinished(ePmCommandResult parResult);
    void slotRegenerationIpSuccess();
    void slotRegenerationProgress(QString parProgress);
    void slotShowAbout();
    void slotEnableMenueEntryForceCleanup();



    // RENAME:
    void slotUpdateFinished();


};
