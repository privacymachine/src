﻿/*==============================================================================
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

#include "WindowMain.h"
#include "ui_WindowMain.h"
#include "WidgetRdpView.h"
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QDesktopServices>
#include <QWidget>

#ifndef SKIP_FREERDP_CODE
  #include <remotedisplaywidget.h>
#endif

WindowMain::WindowMain(QWidget *parParent) :
  QMainWindow(parParent),
  pmManager_(NULL),
  currentWidget_(NULL),
  questionBox_(NULL),
  updateMessage_(NULL),
  regenerationWidget_(NULL),
  tabWidget_(NULL),
  aboutWidget_(NULL),
  ui_(new Ui::WindowMain())
{
  ui_->setupUi(this);

  windowName_ = QApplication::applicationName()+" "+QApplication::applicationVersion();

  setWindowTitle(windowName_);
  connect(ui_->actionQuit,
          SIGNAL(triggered()),
          this,
          SLOT(close()));
  connect(ui_->actionAbout,
          SIGNAL(triggered()),
          this,
          SLOT(slotShowAbout()));
  connect(ui_->actionManual_force_VM_Mask_rebuild,
          SIGNAL(triggered()),
          this,
          SLOT(slotCleanAllVmMasks()));
}

void WindowMain::slotShowAbout()
{
  if(aboutWidget_ != NULL)
  {
    delete aboutWidget_;
  }
  aboutWidget_ = new WidgetAbout();
  aboutWidget_->setWindowIcon(QIcon(":/resources/privacymachine.svg"));
  QLabel *logoLabel = new QLabel(aboutWidget_);
  aboutWidget_->setWindowTitle("About");
  /// @todo: add logo and valid description
  QPixmap pixmap = QIcon(":/resources/privacymachine.svg").pixmap(this->windowHandle(),QSize(150,150));
  logoLabel->setPixmap(pixmap);
  logoLabel->setMinimumSize(QSize(180,150));
  aboutWidget_->addWidget(logoLabel);

  QLabel *versionLabel = new QLabel(aboutWidget_);
  QString msg = "<html><head/><body><p align=\"center\"><span style=\" font-size:16pt;\"> ";
  msg += constPrivacyMachineVersion;
  msg += "</span><br/></p>";
  msg += "<p>The program is provided AS IS</p><p>with NO WARRANTY OF ANY KIND,</p>";
  msg += "<p>INCLUDING THE WARRANTY OF </p><p>DESIGN, MERCHANTABILITY AND</p>";
  msg += "<p>FITNESS FOR A PARTICULAR PURPOSE.</p></body></html>";
  versionLabel->setText(msg);
  aboutWidget_->addWidget(versionLabel);
  aboutWidget_->show();
}

bool WindowMain::init(QString parPmInstallPath, QString parVboxDefaultMachineFolder)
{
  // after this function the caller can use these functions to check the state:
  // pmManager_->isFirstStart()
  // pmManager_->isConfigValid()
  // pmManager_->isBaseDiskAvailable()

  // set some window title
  slotRegenerationProgress("startup...");

  // load the application logo from images.qrc
  setWindowIcon(QIcon(":/resources/privacymachine.svg"));

  pmManager_ = new PmManager();

  // Initialiase the Configuration (on first start)
  if(!pmManager_->initConfiguration(parPmInstallPath, parVboxDefaultMachineFolder))
    return false;

  // read and validate the user config, also check for the BaseDisk
  pmManager_->readAndValidateConfiguration();

  if (!pmManager_->isBaseDiskAvailable())
  {
    /// @todo olaf: show download

  }

  /// @todo: fill with content for alpha 1 users:
  #ifdef PM_WINDOWS
    QString releaseUrl = "https://update.privacymachine.eu/ReleaseNotes_0.10.0.0_WIN64_EN.html";
  #else
    QString releaseUrl = "https://update.privacymachine.eu/ReleaseNotes_0.10.0.0_LINUX_EN.html";
  #endif

  if (pmManager_->isConfigValid() && pmManager_->isBaseDiskAvailable())
  {
    // we initialize pmManager_ with BaseDisk data
    pmManager_->initAllVmMaskData();
  }

  // regeneration of the VM-Masks is needed when we have a new BaseDisk or when the user changes an important part of the configuration
  if(pmManager_->vmMaskRegenerationNecessary())
  {
    regenerateVmMasks();
    return true;
  }


  /// @todo: bernhard: continue working here: currently empty window
  /*
    if (pmManager_->isConfigValid())
    {
      if(pmManager_->vmMaskRegenerationNecessary())
      {
        regenerateVmMasks();
        return true;
      }
    }
    */

  /// @todo olaf: please check in the new tab for pmManager_->isConfigValid()
  return setupTabWidget();
}

void WindowMain::slotUpdateNotFoundBtnOK()
{
    ui_->mainLayout_v->removeWidget(updateMessage_);
    ui_->mainLayout_v->removeWidget(questionBox_);
    questionBox_->close();
    updateMessage_->close();
    delete questionBox_;
    questionBox_ = NULL;
    delete updateMessage_;
    updateMessage_ = NULL;
    exit(0);
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

    // We only update the binary on Windows. On Linux, the user has to take action (they got notified in update dialog).
    #ifdef PM_WINDOWS
      /// @todo: Alex start update process here like
      // UPDATER = new updaterWidget;
      // ui_->mainLayout_v->addWidget(UPDATER);
      // connect(UPDATER,
      //         SIGNAL(finished(int)),
      //         this,
      //         SLOT(slotUpdateFinished()));
      // UPDATER->start()
      connect( FvUpdater::sharedUpdater(), SIGNAL( signalUpdateInstalled() ), this, SLOT( slotUpdateFinished() ) );
      FvUpdater::sharedUpdater()->slotTriggerUpdate();

    #endif
}

void WindowMain::slotUpdateFinished()
{
  /// @todo: Alex: implement error handling etc

  // if(status==BROKE_VMMASKS)
  //   regenerateVMMasks();
  // else
  //   setupTabWidget()
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

  /// @todo: from bernhard to olaf: the user said he want to update later, why start it anyway?
  if(pmManager_->vmMaskRegenerationNecessary() )
    regenerateVmMasks();
  else
    setupTabWidget();
}

void WindowMain::slotCleanAllVmMasks()
{
  QMessageBox msgBox(this);
  msgBox.setText("Do you want to delete all VM-Masks?\nThis will take a while and closes the PrivacyMachine when finished.");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
  {
    cleanVmMasksBlocking();
    this->close();
  }
}

void WindowMain::slotEnableMenueEntryForceCleanup()
{
  ui_->actionManual_force_VM_Mask_rebuild->setEnabled(true);
}

void WindowMain::cleanVmMasksBlocking()
{
  ILOG("remove all VmMasks");
  QList<PmCommand*> commandsList;
  pmManager_->createCommandsToCleanupAllVirtualMachines( commandsList );
  foreach(PmCommand* cmd, commandsList)
  {
    cmd->executeBlocking(false);
  }
}

void WindowMain::regenerateVmMasks()
{

  cleanVmMasksBlocking();

  regenerationWidget_ = new WidgetUpdate(this);
  if (!regenerationWidget_->init(pmManager_))
  {
    IERR("failed to initialize regenerationWidget (updateWidget) ");
    delete regenerationWidget_;
    regenerationWidget_ = NULL;
    return;
  }

  connect(regenerationWidget_,
          SIGNAL(signalUpdateFinished(ePmCommandResult)),
          this,
          SLOT(slotRegenerationFinished(ePmCommandResult)));

  ui_->mainLayout_v->addWidget(regenerationWidget_);

  regenerationWidget_->start();
}

void WindowMain::slotNewVmMaskStarted(int parVmMaskId)
{
  if ( parVmMaskId < 0 || parVmMaskId >= pmManager_->getVmMaskData().count() )
    return;

  VmMaskData* vmMask = pmManager_->getVmMaskData()[parVmMaskId];
  if (vmMask->Instance.isNull())
  {
    IERR("received slotNewVmMaskStarted() but no instance exists");
    return;
  }

  // create a new WidgetRdpView as tab

  WidgetRdpView* rdpView = new WidgetRdpView( "localhost", vmMask->Instance);

  tabWidget_->setUpdatesEnabled(false);
  int indexBeforeLast = tabWidget_->count() - 1;  
  int newTabIndex = tabWidget_->insertTab(indexBeforeLast, rdpView, vmMask->Instance->getConfig()->getFullName());
  tabWidget_->setCurrentIndex(newTabIndex);
  tabWidget_->setUpdatesEnabled(true);
  vmMask->Instance->getInfoIpAddress()->startPollingExternalIp();
  
  connect(&(*vmMask->Instance->getInfoIpAddress()),
          SIGNAL( signalUpdateIpSuccess() ),
          this,
          SLOT( slotRegenerationIpSuccess() ) );

  connect( rdpView, 
           SIGNAL( signalScreenResize(QWidget*) ),
           this,
           SLOT( slotRdpViewScreenResize(QWidget*) ) );
  
  statusBarUpdate();

  return;
}

void WindowMain::slotRegenerationIpSuccess()
{
  // This will cause an update for the currently selected VM-Mask, triggered by all VM-Masks. Might cause extraneous
  // calls, but saves us from passing around widget pointers outside widget code (i.e. from VmInfoIpAddress).
  statusBarUpdate();

}

void WindowMain::slotRegenerationProgress(QString parProgress)
{
  QString title = windowName_;
  if (regenerationWidget_ != NULL)
  {
    title += " - Update: ";
    title += parProgress;
  }
  setWindowTitle(title);
}

void WindowMain::slotTabCloseRequested(int parTabIndex)
{
  QWidget* currentWidget_ = tabWidget_->widget(parTabIndex);
  ILOG("Tab close request")

  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( currentWidget_ );
  
  // If this actually is a RDP Widget, we want to shut down the virtual machine it is associated
  // with, if there is any.
  if( widgetRdpView != NULL )
  {    
    VmMaskData* vmMask = pmManager_->getVmMaskData()[widgetRdpView->getVmMaskId()];

    // Disable screen resize messages
    disconnect( widgetRdpView,
                SIGNAL( signalScreenResize( QWidget* )),
                0,
                0 );

    // In case we still try to obtain the IP address, cancel it.
    vmMask->Instance->getInfoIpAddress()->abort();
    disconnect( &(*vmMask->Instance->getInfoIpAddress()),
                SIGNAL( signalUpdateIpSuccess() ),
                0,
                0 );


    /// @todo: move to whole WindowMain
    /*
    // Show a 'are you shure you want to close this VM-Mask' MessageBox
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setWindowTitle(QApplication::applicationName()+" "+QApplication::applicationVersion());
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
    msgBox.setText("Are you shure you want to close The VM-Mask " + vmMask->Instance->getConfig()->getVmName());
    int ret = msgBox.exec();
    if (ret == QMessageBox::Abort)
      return;    
    */

    // Copy VPN logs before the machine shuts down.
    QList<PmCommand*> commandsList;
    PmCommand* pCurrentCommand = NULL;
    pCurrentCommand = GetPmCommandForScp2Host(constRootUser, constLocalIp, QString::number( vmMask->Instance->getConfig()->getSshPort()), constRootPwd,
                                              pmManager_->getPmConfigDir().path() + "/logs/vmMask_" + vmMask->Instance->getConfig()->getName() + "_vpnLog.txt",
                                              "/var/log/openvpn.log");
    pCurrentCommand->setDescription("Copy vpn logs of VM-Mask " + vmMask->Instance->getConfig()->getName());
    pCurrentCommand->setTimeoutMilliseconds(2000);
    commandsList.append( pCurrentCommand );

    // additional append the commands to close the machine
    pmManager_->createCommandsToCloseVmMask(vmMask->UserConfig->getVmName(), vmMask->UserConfig->getFullName(), commandsList);
    foreach(PmCommand* cmd, commandsList)
    {
      cmd->executeBlocking(false);
    }
    // remove commands and free memory
    while (commandsList.size())
      delete commandsList.takeFirst();


    // Notify WidgetNewTab that the VmMask has closed
    emit signalVmMaskClosed(vmMask->Instance->getVmMaskId());
  }

  tabWidget_->removeTab(parTabIndex);
  delete currentWidget_;
}


void WindowMain::slotTabCurrentChanged(int parTabIndex)
{
  currentWidget_ = tabWidget_->widget(parTabIndex);

  statusBarUpdate();
}


bool WindowMain::setupTabWidget()
{
  tabWidget_ = new QTabWidget(this);
  tabWidget_->setTabsClosable(true);
  WidgetNewTab *newTab = new WidgetNewTab(this);
  newTab->init(pmManager_);
  tabWidget_->addTab(newTab,QIcon(":/images/ApplicationIcon32.png"),"+");

  ui_->mainLayout_v->addWidget(tabWidget_);
  centralWidget()->setLayout( ui_->mainLayout_v );

  // While we want to be able to close VM Mask tabs in general, we would not want to close the NewTab
  // => remove 'Close' button for this tab only.
  tabWidget_->tabBar()->setTabButton(0, QTabBar::RightSide, NULL);

  connect(tabWidget_,
          SIGNAL(tabCloseRequested(int)),
          this,
          SLOT(slotTabCloseRequested(int)));

  connect(this,
          SIGNAL(signalVmMaskClosed(int)),
          newTab,
          SLOT(slotVmMaskClosed(int)));

  connect(newTab,
          SIGNAL(signalNewVmMaskReady(int)),
          this,
          SLOT(slotNewVmMaskStarted(int)));
  
  connect( tabWidget_,
           SIGNAL(currentChanged(int)),
           this,
           SLOT(slotTabCurrentChanged(int)));

  return true;
}

void WindowMain::statusBarUpdate()
{
  // If this actually is a RDP Widget, attempt to update the status bar with widget info and VM Mask info, if available
  // For all other widgets, we clear the status bar text.
  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( currentWidget_ );
  QString statusText = "";
  if( widgetRdpView != NULL )
  {    
    VmMaskData* vmMask = pmManager_->getVmMaskData()[widgetRdpView->getVmMaskId()];
    statusText += vmMask->Instance->getConfig()->toString();

    statusText += ", ";
    QString ipStatus = vmMask->Instance->getInfoIpAddress()->toStatus();
    statusText += ipStatus;
    statusText += ipStatus != "" ? ", " : "";
    
    statusText += tr("Screen Size") + ": " +
                  QString::number( widgetRdpView->screenWidth_ ) + "x"  +
                  QString::number( widgetRdpView->screenHeight_ );
  }

  statusBar()->showMessage( statusText );  
}

WindowMain::~WindowMain()
{
  if (pmManager_) delete pmManager_;
  if (aboutWidget_) delete aboutWidget_;
  delete ui_;
}

void WindowMain::show()
{
  QMainWindow::show();
}

void WindowMain::slotRdpViewScreenResize( QWidget *widget )
{
  ILOG( QString( "WindowMain::slotRdpViewScreenResize: " ) + QString::number( widget->width() ) + " x " + QString::number( widget->height() ) );
  statusBarUpdate();
}

void WindowMain::slotRegenerationFinished(ePmCommandResult parResult)
{
  QString message = " ";

  switch (parResult)
  {
    case aborted:
      message += "Generation of VM-Masks aborted.";
      ui_->statusbar->showMessage(message, 5000);
      QMessageBox::information(this, "PrivacyMachine", message);
      break;

    case success:
      message += "Generation of VM-Masks successful.";
      ui_->statusbar->showMessage(message, 5000);
      break;

    default: // failed
      message += "Generation of VM-Masks failed. Please check the logfile for Details.";
      ui_->statusbar->showMessage(message);
      QMessageBox::warning(this, "PrivacyMachine", message);
  }

  setWindowTitle(windowName_ + message);

  ui_->mainLayout_v->removeWidget(regenerationWidget_);
  delete regenerationWidget_;
  regenerationWidget_=NULL;

  // Only proceed if all VMs are available - otherwise a user might e.g. abort the first update, leaving at least some
  // VMs missing
  if( pmManager_->allVmMasksExist() && (parResult == aborted || parResult == success) )
  {
    pmManager_->saveConfiguredVmMasks();
    setupTabWidget();
  }
}


void WindowMain::closeEvent(QCloseEvent * parEvent)
{
  if (regenerationWidget_ != NULL)
  {
    // Stop the running update
    regenerationWidget_->abort();
    ui_->mainLayout_v->removeWidget(regenerationWidget_);
    delete regenerationWidget_;
    regenerationWidget_=NULL;
  }

  if (aboutWidget_ != NULL)
  {
    delete aboutWidget_;
    aboutWidget_ = NULL;
  }


  ILOG("Restoring all VM snapshots...");
  QList<PmCommand*> commandsList;
  pmManager_->createCommandsToCloseAllVmMasks( commandsList );
  //pmManager_->createCommandsCleanupVirtualBoxVms( commandsList );
  foreach(PmCommand* cmd, commandsList)
  {
    cmd->executeBlocking(false);
  }

  ILOG("Saving internals")
}


bool WindowMain::showReleaseNotes(QString parUrl, ulong parTimeoutInMilliseconds)
{
  bool success = false;
  // from http://stackoverflow.com/questions/13207493/qnetworkreply-and-qnetworkaccessmanager-timeout-in-http-request
  QTimer timer;    
  timer.setSingleShot(true);

  // Start Release Notes Download
  QNetworkRequest request( parUrl );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
	QNetworkAccessManager nam;
  QNetworkReply* reply = nam.get( request );
  QEventLoop loop;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  timer.start(parTimeoutInMilliseconds);
  loop.exec();
  
  if( timer.isActive() )
  {
    if (reply->error() != QNetworkReply::NoError)
    {
      qDebug()<<"Unable to download the release notes: " << reply->errorString();       
    }
    else if (reply->isReadable() )
	  { 
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setWindowTitle(QApplication::applicationName()+" "+QApplication::applicationVersion());
      msgBox.setTextFormat(Qt::RichText);   // this is what makes the links clickable
      msgBox.setText(reply->readAll());
      msgBox.exec();

      success = true;      
    }    
  }
  else
  {
    // timeout
    reply->abort();    
    disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
     
    qDebug()<<"Timeout after " << parTimeoutInMilliseconds << "ms while downloading release notes: " << reply->errorString();
  }

  return success;
}
