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
  pmManager_(0),
  ui_(new Ui::WindowMain())
{
  ui_->setupUi(this);
  currentWidget_ = NULL;
  questionBox_ = NULL;
  updateMessage_ = NULL;
  regenerationWidget_ = NULL;
  tabWidget_ = NULL;
  aboutWidget_ = NULL;

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
    delete aboutWidget_;
  aboutWidget_ = new WidgetAbout();
  aboutWidget_->setWindowIcon(QIcon(":/resources/privacymachine.svg"));
  QLabel *logoLabel = new QLabel(aboutWidget_);
  aboutWidget_->setWindowTitle("About");
  // TODO: add logo and valid description
  QPixmap pixmap = QIcon(":/resources/privacymachine.svg").pixmap(this->windowHandle(),QSize(150,150));
  logoLabel->setPixmap(pixmap);
  logoLabel->setMinimumSize(QSize(180,150));
  aboutWidget_->addWidget(logoLabel);

  QLabel *versionLabel = new QLabel(aboutWidget_);
  versionLabel->setText(
  "<html><head/><body><p align=\"center\"><span style=\" font-size:16pt;\">0.9.0.0</span><br/></p>"
        "<p>The program is provided AS IS</p><p>with NO WARRANTY OF ANY KIND,</p>"
        "<p>INCLUDING THE WARRANTY OF </p><p>DESIGN, MERCHANTABILITY AND</p>"
        "<p>FITNESS FOR A PARTICULAR PURPOSE.</p></body></html>"
        );
  aboutWidget_->addWidget(versionLabel);
  aboutWidget_->show();
}

bool WindowMain::init(QString parPmInstallPath, QString parVboxDefaultMachineFolder)
{
  // set some window title
  slotRegenerationProgress("startup...");

  // load the application logo from images.qrc
  setWindowIcon(QIcon(":/resources/privacymachine.svg"));

  pmManager_ = new PMManager();
  if(!pmManager_->init_1(parPmInstallPath, parVboxDefaultMachineFolder)) return false;

  bool updateAvailable = false;

  // Does the base-disk exists?
  QString currentVMDK = pmManager_->baseDiskWithPath() + ".vmdk";
  // Especially for Windows try to avoid downloading into Program Files folder.
  // TODO AL: + "/download/", ggF. erstellen
  FvUpdater::sharedUpdater()->setDownloadPath( pmManager_->getPmUserConfigDir() + "/" );
  FvUpdater::sharedUpdater()->setTargetPath( pmManager_->getBaseDiskDirectoryPath() );
  if (!QFile::exists(currentVMDK))
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("PrivacyMachine");
    QString message = QCoreApplication::translate("check of software dependencies", "We need to download the initial base disk (650MB).\nThis might take a couple of minutes...\nPress Ok to start.");
    msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ok);
    msgBox.setText(message);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
    {
      ILOG("Start downloading the base-disk");
    }
    else
    {
      IERR("User canceled download of basedisk -> no way to continue without base-disk");
      return false;
    }

    QString appcastUrl = pmManager_->getUpdateConfig().appcastBaseDisk;

    // We have to download the first base-disk?
    QApplication::setOrganizationName("PrivacyMachine");
    QApplication::setOrganizationDomain("privacymachine.eu");
    FvUpdater::sharedUpdater()->SetComponentVersion( ComponentVersion( 1, 0, 0, 0 ) );
    FvUpdater::sharedUpdater()->SetFeedURL( appcastUrl );
    updateAvailable = FvUpdater::sharedUpdater()->CheckForUpdateBlocking( 5, Verbosity::Interactive );

    // The rest of the program depends on having the initial base disk, so, if we cannot get it, the only option is to
    // gracefully shut down.
    // FIXME AL will block if base disk cannot be found.
    if( updateAvailable )
    {
      /*
        QString statusMessage = "Downloading initial base disk. This might take a couple of minutes...";
        ILOG(statusMessage);
        updateMessage_ = new QLabel(this);
        updateMessage_->setText( statusMessage );
        updateMessage_->setFont(QFont("Sans",18));
        updateMessage_->setAlignment(Qt::AlignHCenter);
        ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
        ui_->mainLayout_v->addWidget(updateMessage_);
        */



      FvUpdater::sharedUpdater()->InstallUpdateBlocking( );
    }
    else
    {
      QString errorMessage = 
        "Could not find initial base disk, which is required to run the PrivacyMachine. " 
        "Seems that we could not find '" + appcastUrl + "'.";
      IERR(errorMessage);
      updateMessage_ = new QLabel(this);
      updateMessage_->setText( errorMessage );
      updateMessage_->setFont(QFont("Sans",18));
      updateMessage_->setAlignment(Qt::AlignHCenter);
      ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
      ui_->mainLayout_v->addWidget(updateMessage_);
      questionBox_ = new QDialogButtonBox(this);
      questionBox_->addButton(QDialogButtonBox::Ok);
      ui_->mainLayout_v->addWidget(questionBox_);

      connect(questionBox_,
              SIGNAL(accepted()),
              this,
              SLOT(slotUpdateNotFoundBtnOK()));

      return false;

    }

  }
  else
  {
    FvUpdater::sharedUpdater()->SetComponentVersion( ComponentVersion( 1, 0, 0, 0 ) );
    QString appcastUrl = pmManager_->getUpdateConfig().appcastPM;
    FvUpdater::sharedUpdater()->SetFeedURL( appcastUrl );
    #ifdef PM_WINDOWS
      QString releaseUrl = "https://update.privacymachine.eu/ReleaseNotes_0.10.0.0_WIN64_EN.html";
    #else
      QString releaseUrl = "https://update.privacymachine.eu/ReleaseNotes_0.10.0.0_LINUX_EN.html";
    #endif
    updateAvailable = showReleaseNotes( releaseUrl, 2000 );

//    if(updateAvailable)
//    {
//      #ifdef PM_WINDOWS
//        updateMessage_ = new QLabel(this);
//        updateMessage_->setText("An update is available!\nInstall now?");
//        updateMessage_->setFont(QFont("Sans",18));
//        updateMessage_->setAlignment(Qt::AlignHCenter);
//        ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
//        ui_->mainLayout_v->addWidget(updateMessage_);
//        questionBox_ = new QDialogButtonBox(this);
//        questionBox_->addButton(QDialogButtonBox::Ok);
//        questionBox_->addButton(tr("&Later"),QDialogButtonBox::RejectRole);
//        ui_->mainLayout_v->addWidget(questionBox_);
//
//        connect(questionBox_,
//          SIGNAL(accepted()),
//          this,
//          SLOT(slotUpdateBtnOK()));
//
//        connect(questionBox_,
//          SIGNAL(rejected()),
//          this,
//          SLOT(slotUpdateBtnLater()));
//
//        connect(questionBox_,
//          SIGNAL(destroyed()),
//          this,
//          SLOT(slotEnableMenueEntryForceCleanup()));
//        
//        return true;
//
//      #else
//        updateMessage_ = new QLabel(this);
//        // HACK AL: URL hardcoded for now
//        updateMessage_->setText(
//          "An update is available.\nYou can get it here: https://www.privacymachine.eu/de/download/." );
//        updateMessage_->setFont(QFont("Sans",18));
//        updateMessage_->setAlignment(Qt::AlignHCenter);
//        ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
//        ui_->mainLayout_v->addWidget(updateMessage_);
//        questionBox_ = new QDialogButtonBox(this);
//        questionBox_->addButton(QDialogButtonBox::Ok);
//        ui_->mainLayout_v->addWidget(questionBox_);
//
//        connect(questionBox_,
//          SIGNAL(accepted()),
//          this,
//          SLOT(slotUpdateBtnOK()));
//
//        connect(questionBox_,
//          SIGNAL(destroyed()),
//          this,
//          SLOT(slotEnableMenueEntryForceCleanup()));
//        
//        return true;
//
//      #endif
//
//    }
//    else
//      slotEnableMenueEntryForceCleanup();
  }

  // Now we can be sure to have the initial base disk, can we initialize pmManager_
  if(!pmManager_->init_2()) return false;

  if(pmManager_->vmMaskRegenerationNecessary() )
  {
    regenerateVMMasks();
    return true;
  }

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
      // TODO: Alex start update process here like
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
  // TODO: Alex: implement error handling etc

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
  if(pmManager_->vmMaskRegenerationNecessary() )
    regenerateVMMasks();
  else
    setupTabWidget();
}

void WindowMain::slotCleanAllVmMasks()
{
  QMessageBox msgBox(this);
  msgBox.setText("Do you want to delete all VM-Masks?\nThis will close the PrivacyMachine.");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
  {
    cleanVMMasksBlocking();
    this->close();
  }

}

void WindowMain::slotEnableMenueEntryForceCleanup()
{
  ui_->actionManual_force_VM_Mask_rebuild->setEnabled(true);
}

void WindowMain::cleanVMMasksBlocking()
{
  ILOG("remove all vmMasks");
  QList<PMCommand*> commandsList;
  pmManager_->createCommandsCleanupVirtualBoxVms( commandsList );
  foreach(PMCommand* cmd, commandsList)
  {
    cmd->executeBlocking(false);
  }
}

void WindowMain::regenerateVMMasks()
{

  cleanVMMasksBlocking();

  regenerationWidget_ = new WidgetUpdate(this);
  if (!regenerationWidget_->init(pmManager_))
  {
    IERR("failed to initialize regenerationWidget (updateWidget) ");
    delete regenerationWidget_;
    regenerationWidget_ = NULL;
    return;
  }

  connect(regenerationWidget_,
          SIGNAL(signalUpdateFinished(CommandResult)),
          this,
          SLOT(slotRegenerationFinished(CommandResult)));

  ui_->mainLayout_v->addWidget(regenerationWidget_);

  regenerationWidget_->start();
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
  curPMInstance->getInfoIpAddress()->startPollingExternalIp();
  
  connect(
    &(*curPMInstance->getInfoIpAddress()),
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


void WindowMain::slotTabCloseRequested(int parIndex)
{
  QWidget* currentWidget_ = tabWidget_->widget(parIndex);
  ILOG(">>>>>>>>>>>>>Tab close request")
  // If we get the property 'instance_index', it is a RdpView-Widget
  QVariant val = currentWidget_->property("instance_index");

  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( currentWidget_ );
  // If this actually is a RDP Widget, disconnect the slot in any case.
  if( widgetRdpView != NULL )
  {
    disconnect( widgetRdpView, 
             SIGNAL( signalScreenResize( QWidget* )),
             0,
             0 );
  }
  
  // If this actually is a RDP Widget, we want to shut down the virtual machine it is associated
  // with, if there is any.
  if( widgetRdpView != NULL  &&  widgetRdpView->GetPmInstance() != NULL )
  {    
    PMInstance* curPMInstance = widgetRdpView->GetPmInstance();
    // Show a 'are you shure you want to close this VM-Mask' MessageBox
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setWindowTitle(QApplication::applicationName()+" "+QApplication::applicationVersion());
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
    msgBox.setText("Are you shure you want to close The VM-Mask "+curPMInstance->getConfig()->vmName);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Abort)
      return;

    QList<PMCommand*> commandsList;
    // before closing copy VPN logs
    PMCommand* pCurrentCommand = NULL;
    QString userConfigDir;
    getAndCreateUserConfigDir(userConfigDir);
    pCurrentCommand = GetPMCommandForScp2Host("root",constVmIp,QString::number( curPMInstance->getConfig()->sshPort ),constRootPw,
                                              userConfigDir+"/logs/vmMask_"+curPMInstance->getConfig()->name+"_vpnLog.txt",
                                              "/var/log/openvpn.log");
    pCurrentCommand->setDescription("Copy vpn logs of VM-Mask "+curPMInstance->getConfig()->name);
    pCurrentCommand->setTimeoutMilliseconds(2000);
    commandsList.append( pCurrentCommand );

    pmManager_->createCommandsClose(curPMInstance, commandsList);
    foreach(PMCommand* cmd, commandsList)
    {
      cmd->executeBlocking(false);
    }
    curPMInstance->getConfig()->vmMaskCreated = false;
    // In case we still try to obtain the IP address, cancel it.
    curPMInstance->getInfoIpAddress()->abort();
    disconnect(
      &(*curPMInstance->getInfoIpAddress()),
      SIGNAL( signalUpdateIpSuccess() ), 
      0,
      0 );
    
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
  delete currentWidget_;
}


void WindowMain::slotTabCurrentChanged(int parIndex)
{
  currentWidget_ = tabWidget_->widget(parIndex);

  // If we get the property 'instance_index', it is a RdpView-Widget
  QVariant val = currentWidget_->property("instance_index");

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


void WindowMain::statusBarUpdate()
{
  // If this actually is a RDP Widget, attempt to update the status bar with widget info and VM Mask info, if available
  // For all other widgets, we clear the status bar text.
  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( currentWidget_ );
  QString statusText = "";
  if( widgetRdpView != NULL )
  {    
    if( widgetRdpView->GetPmInstance() != NULL )
    {
      PMInstance *pPmInstance = widgetRdpView->GetPmInstance();
      statusText += pPmInstance->getConfig()->toString();

      statusText += ", ";
      QString ipStatus = pPmInstance->getInfoIpAddress()->toStatus();
      statusText += ipStatus;
      statusText += ipStatus != "" ? ", " : "";
      
    }
    
    statusText += 
        tr("Screen Size") + ": " + QString::number( widgetRdpView->screenWidth_ ) + "x" 
        + QString::number( widgetRdpView->screenHeight_ );
    
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


void WindowMain::slotRegenerationFinished(CommandResult parResult)
{
  QString message=" ";

  switch (parResult)
  {
    case aborted:
      message+="Generation of VM-Masks aborted.";
      ui_->statusbar->showMessage(message, 5000);
      QMessageBox::information(this, "PrivacyMachine", message);
      break;

    case success:
      message+="Generation of VM-Masks successful.";
      ui_->statusbar->showMessage(message, 5000);
      break;

    default: // failed
      message+="Generation of VM-Masks failed. Please check the logfile for Details.";
      ui_->statusbar->showMessage(message);
      QMessageBox::warning(this, "PrivacyMachine", message);
  }

  setWindowTitle(windowName_ + message);

  ui_->mainLayout_v->removeWidget(regenerationWidget_);
  delete regenerationWidget_;
  regenerationWidget_=NULL;
  // Only proceed if all VMs are available - otherwise a user might e.g. abort the first update, leaving at least some
  // VMs missing
  if( pmManager_->vmsExist()  &&  ( parResult == aborted  ||  parResult == success ) )
  {
    pmManager_->saveConfiguredVMMasks();
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
  QList<PMCommand*> commandsList;
  pmManager_->createCommandsCloseAllVms( commandsList );
  //pmManager_->createCommandsCleanupVirtualBoxVms( commandsList );
  foreach(PMCommand* cmd, commandsList)
  {
    cmd->executeBlocking(false);
  }

  ILOG("Saving internals")


}


bool WindowMain::showReleaseNotes( QString url, ulong milliseconds )
{
  bool success = false;
  // from http://stackoverflow.com/questions/13207493/qnetworkreply-and-qnetworkaccessmanager-timeout-in-http-request
  QTimer timer;    
  timer.setSingleShot(true);

  // Start Release Notes Download
  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
	QNetworkAccessManager nam;
  QNetworkReply* reply = nam.get( request );
  QEventLoop loop;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  timer.start(milliseconds); 
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
     
    qDebug()<<"Timeout after " << milliseconds << "ms while downloading release notes: " << reply->errorString();

  }

  return success;
  
}


