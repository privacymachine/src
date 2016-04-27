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

#include "WindowMain.h"
#include "ui_WindowMain.h"
#include "WidgetRdpView.h"
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QDesktopServices>

#include <remotedisplaywidget.h>


WindowMain::WindowMain(QWidget *parParent) :
  QMainWindow(parParent),
  pmManager_(0),
  ui_(new Ui::WindowMain())
{
  ui_->setupUi(this);
  questionBox_ = NULL;
  updateMessage_ = NULL;
  updateWidget_ = NULL;
  tabWidget_ = NULL;
}

bool WindowMain::init(QString pmInstallPath)
{
  // set some window title
  slotUpdateProgress("startup...");

  // load the application logo from images.qrc (works only under windows)
#ifdef PM_WINDOWS
  setWindowIcon(QIcon("://images/privacymachine.svg"));
#else
  setWindowIcon(QIcon());
#endif

  pmManager_ = new PMManager();
  if(!pmManager_->init(pmInstallPath)) return false;

  // Show some Introduction for the Demo-Mode
  //   TODO: remove after GLT16
  QUrl pdfIntroFile = QUrl::fromLocalFile(pmInstallPath + "/Intro_de.pdf");
  QDesktopServices::openUrl(pdfIntroFile);

  // TODO: Check for Updates
  bool updateAvailable = true;

  if(updateAvailable && !pmManager_->vmsExist() )
  {
    updateMessage_ = new QLabel(this);
    if( pmManager_->vmsExist() )
    {
      QString newUpdateLine1 = tr("An update is available!");
      QString newUpdateLine2 = tr("Install now?");
      updateMessage_->setText(newUpdateLine1  + QString("\n") + newUpdateLine2);
    }
    else
    {
      QString newUpdateLine1 = tr("Update required. At least one VM Mask ");
      QString newUpdateLine2 = tr("needs to have its VM created.");
      updateMessage_->setText(newUpdateLine1  + QString("\n") + newUpdateLine2);
    }

    updateMessage_->setFont(QFont("Sans",18));
    updateMessage_->setAlignment(Qt::AlignHCenter);
    ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
    ui_->mainLayout_v->addWidget(updateMessage_);
    questionBox_ = new QDialogButtonBox(this);
    questionBox_->addButton(QDialogButtonBox::Ok);
    if( pmManager_->vmsExist() )
    {
      questionBox_->addButton(tr("&Later"),QDialogButtonBox::RejectRole);
    }
    ui_->mainLayout_v->addWidget(questionBox_);

    connect(questionBox_,
            SIGNAL(accepted()),
            this,
            SLOT(slotUpdateBtnOK()));

    connect(questionBox_,
            SIGNAL(rejected()),
            this,
            SLOT(slotUpdateBtnLater()));
  }
  else
  {
    return setupTabWidget();
  }

  return true;
}

void WindowMain::slotUpdateBtnLater()
{
  ui_->mainLayout_v->removeWidget(updateMessage_);
  ui_->mainLayout_v->removeWidget(questionBox_);
  questionBox_->close();
  updateMessage_->close();
  delete questionBox_;
  questionBox_ = NULL;
  delete updateMessage_;
  updateMessage_ = NULL;

  setupTabWidget();
}

void WindowMain::slotUpdateBtnOK()
{
  ui_->mainLayout_v->removeWidget(updateMessage_);
  ui_->mainLayout_v->removeWidget(questionBox_);
  questionBox_->close();
  updateMessage_->close();
  delete questionBox_;
  questionBox_ = NULL;
  delete updateMessage_;
  updateMessage_ = NULL;

  updateWidget_ = new WidgetUpdate(this);
  if (!updateWidget_->init(pmManager_))
  {
    IERR("failed to initialize updateWidget");
    delete updateWidget_;
    updateWidget_ = NULL;
    return;
  }

  connect(updateWidget_,
          SIGNAL(signalUpdateFinished(commandResult)),
          this,
          SLOT(slotUpdateFinished(commandResult)));

  ui_->mainLayout_v->addWidget(updateWidget_);

  updateWidget_->start();
}

void WindowMain::slotNewVmMaskStarted(int parIndexVmMask)
{
  if (parIndexVmMask < 0) return;

  PMInstance* curPMInstance = pmManager_->getInstances().at(parIndexVmMask);

  // create a new WidgetRdpView as tab

  WidgetRdpView* rdpView = new WidgetRdpView( "localhost", curPMInstance );

  tabWidget_->setUpdatesEnabled(false);
  int indexBeforeLast = tabWidget_->count() - 1;  
  int newTabIndex = tabWidget_->insertTab(indexBeforeLast, rdpView, curPMInstance->getConfig()->fullName);
  tabWidget_->setCurrentIndex(newTabIndex);
  tabWidget_->setUpdatesEnabled(true);
  curPMInstance->getConfig()->vmMaskCreated = true;
  connect( rdpView, 
           SIGNAL( signalResize(QWidget*)),
           this,
           SLOT( slotRdpViewResize(QWidget*) ) );
  
  statusBarUpdate( rdpView );

  return;

}

void WindowMain::slotUpdateProgress(QString parProgress)
{
  QString title = "PrivacyMachine";
  if (updateWidget_ != NULL)
  {
    title += " - Update: ";
    title += parProgress;
  }
  setWindowTitle(title);
}


void WindowMain::slotTabCloseRequested(int parIndex)
{
  QWidget* curWidget = tabWidget_->widget(parIndex);
  ILOG(">>>>>>>>>>>>>Tab close request")
  // If we get the property 'instance_index', it is a RdpView-Widget
  QVariant val = curWidget->property("instance_index");

  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( curWidget );
  // If this actually is a RDP Widget, disconnect the slot in any case.
  if( widgetRdpView != NULL )
  {
    disconnect( widgetRdpView, 
             SIGNAL( signalResize( QWidget* )),
             0,
             0 );
  }
  
  // If this actually is a RDP Widget, we want to shut down the virtual machine it is associated
  // with, if there is any.
  if( widgetRdpView != NULL  &&  widgetRdpView->GetPmInstance() != NULL )
  {    
    PMInstance* curPMInstance = widgetRdpView->GetPmInstance();
    // The poweroff command is executed wihtout showing any window
    QList<PMCommand*> commandsList;
    pmManager_->createCommandsClose(curPMInstance, commandsList);
    foreach(PMCommand* cmd, commandsList)
    {
      cmd->executeBlocking(false);
    }
    curPMInstance->getConfig()->vmMaskCreated = false;
    emit signalVmMaskClosed( curPMInstance->getConfig()->vmMaskId );

    // TODO: free memory
    /*
    while commandsList.size()
    {
      commandsList.last();
    }
    */
  }
  tabWidget_->removeTab(parIndex);
  delete curWidget;
}

void WindowMain::slotTabCurrentChanged(int parIndex)
{
  QWidget* curWidget = tabWidget_->widget(parIndex);

  // If we get the property 'instance_index', it is a RdpView-Widget
  QVariant val = curWidget->property("instance_index");

  statusBarUpdate( curWidget );

}

bool WindowMain::setupTabWidget()
{
  tabWidget_ = new QTabWidget(this);
  tabWidget_->setTabsClosable(true);
  WidgetNewTab *newTab = new WidgetNewTab(this);
  newTab->init(pmManager_);
  ui_->mainLayout_v->addWidget(tabWidget_);
  tabWidget_->addTab(newTab,QIcon(":/images/ApplicationIcon32.png"),"+");

  // While we want to be able to close VM Mask tabs in general, we would not want to close the NewTab
  // => remove 'Close' button for this tab only.
  tabWidget_->tabBar()->setTabButton( 0, QTabBar::RightSide, NULL );

  connect(tabWidget_,
          SIGNAL(tabCloseRequested(int)),
          this,
          SLOT(slotTabCloseRequested(int)));

  connect(this,
          SIGNAL(signalVmMaskClosed(int)),
          newTab,
          SLOT(slotVmMaskClosed(int)));

  connect(newTab,
          SIGNAL(newVmMaskReady(int)),
          this,
          SLOT(slotNewVmMaskStarted(int)));
  
  connect( tabWidget_,
           SIGNAL(currentChanged(int)),
           this,
           SLOT(slotTabCurrentChanged(int)));

  return true;
}


void WindowMain::statusBarUpdate( QWidget *widget )
{
  // If this actually is a RDP Widget, attempt to update the status bar with widget info and VM Mask info, if available
  // For all other widgets, we clear the status bar text.
  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( widget );
  QString statusText = "";
  if( widgetRdpView != NULL )
  {    
    if( widgetRdpView->GetPmInstance() != NULL )
    {
      statusText += widgetRdpView->GetPmInstance()->getConfig()->toString();
      
    }
    
    statusText += 
        ", " + tr("Screen Size") + ": " + QString::number( widgetRdpView->screenWidth_ ) + "x" 
        + QString::number( widgetRdpView->screenHeight_ );
    
  }

  statusBar()->showMessage( statusText );
  
}


WindowMain::~WindowMain()
{
  if (pmManager_) delete pmManager_;
  delete ui_;
}

void WindowMain::show()
{
  QMainWindow::show();

}


void WindowMain::slotRdpViewResize( QWidget *widget )
{
  statusBarUpdate( widget );
}


void WindowMain::slotUpdateFinished(commandResult parResult)
{
  QString message="";

  switch (parResult)
  {
    case aborted:
      message+="Update aborted.";
      ui_->statusbar->showMessage(message, 5000);
      QMessageBox::information(this, "PrivacyMachine", message);
      break;

    case success:
      message+="Update successful.";
      ui_->statusbar->showMessage(message, 5000);
      break;

    default: // failed
      message+="Update failed. Please check the logfile for Details.";
      ui_->statusbar->showMessage(message);
      QMessageBox::warning(this, "PrivacyMachine", message);
  }

  setWindowTitle("PrivacyMachine " + message);

  ui_->mainLayout_v->removeWidget(updateWidget_);
  delete updateWidget_;
  updateWidget_=NULL;
  // Only proceed if all VMs are available - otherwise a user might e.g. abort the first update, leaving at least some
  // VMs missing
  if( pmManager_->vmsExist()  &&  ( parResult == aborted  ||  parResult == success ) )
  {
    setupTabWidget();
  }
}

void WindowMain::closeEvent(QCloseEvent * parEvent)
{
  if (updateWidget_ != NULL)
  {
    // Stop the running update
    updateWidget_->abort();
    ui_->mainLayout_v->removeWidget(updateWidget_);
    delete updateWidget_;
    updateWidget_=NULL;
  }

  ILOG("Restoring all VM snapshots...")
  QList<PMCommand*> commandsList;
  pmManager_->createCommandsCloseAllVms( commandsList );
  foreach(PMCommand* cmd, commandsList)
  {
    cmd->executeBlocking(false);
  }

}

