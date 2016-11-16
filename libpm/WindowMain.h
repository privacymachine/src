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

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QMessageBox>
#include <QVector>

#include "fvupdater.h"

#include "WidgetAbout.h"
#include "WidgetUpdate.h"
#include "WidgetNewTab.h"
#include "PMManager.h"
#include "utils.h"
#include "VmInfoIpAddress.h"

namespace Ui {
  class WindowMain;
}


class WindowMain : public QMainWindow
{
    Q_OBJECT
    
  public:
    explicit WindowMain(QWidget *parParent = 0);
    void show();
    bool init(QString parPmInstallPath, QString parVboxDefaultMachineFolder);
    ~WindowMain();

    
  private:
    bool setupTabWidget();
    void statusBarUpdate();
    void regenerateVMMasks();
    void cleanVMMasksBlocking();

    bool showReleaseNotes( QString url, ulong milliseconds );

    QWidget* currentWidget_;
    PMManager *pmManager_;
    QDialogButtonBox *questionBox_;
    Ui::WindowMain *ui_;
    QLabel *updateMessage_;
    WidgetUpdate *regenerationWidget_;
    QTabWidget *tabWidget_;
    WidgetAbout *aboutWidget_;
    QString windowName_;

  protected:
    // override from QWidget
    void closeEvent(QCloseEvent *event);


  signals:
    void signalVmMaskClosed( int vmMaskId );


  private slots:
    void slotNewVmMaskStarted(int parIndexVmMask);
    void slotRdpViewScreenResize( QWidget* );
    void slotTabCloseRequested(int parIndex);
    void slotTabCurrentChanged(int parIndex);
    void slotUpdateBtnLater();
    void slotUpdateBtnOK();
    void slotUpdateNotFoundBtnOK();
    void slotUpdateFinished();
    void slotCleanAllVmMasks();
    void slotRegenerationFinished(CommandResult parResult);
    void slotRegenerationIpSuccess();
    void slotRegenerationProgress(QString parProgress);
    void slotShowAbout();
    void slotEnableMenueEntryForceCleanup();

};
