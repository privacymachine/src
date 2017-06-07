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

#include "WindowMain.h"
#include "ui_WindowMain.h"
#include "WidgetRdpView.h"
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QDesktopServices>
#include <QWidget>
#include <QApplication>

#ifndef SKIP_FREERDP_CODE
  #include <remotedisplaywidget.h>
#endif

WindowMain::WindowMain(QWidget *parParent) :
  QMainWindow(parParent),
  pmManager_(nullptr),
  updateManager_(nullptr),
  currentWidget_(nullptr),
  questionBox_(nullptr),
  updateMessage_(nullptr),
  regenerationWidget_(nullptr),
  tabWidget_(nullptr),
  aboutWidget_(nullptr),
  ui_(new Ui::WindowMain())
{
  ui_->setupUi(this);

  setWindowTitle(QApplication::applicationName());

  connect(ui_->actionQuit,
          &QAction::triggered,
          this,
          &QMainWindow::close);

  connect(ui_->actionAbout,
          &QAction::triggered,
          this,
          &WindowMain::slotShowAbout);

  connect(ui_->actionManual_force_VM_Mask_rebuild,
          &QAction::triggered,
          this,
          &WindowMain::slotCleanAllVmMasks);
}

void WindowMain::slotShowAbout()
{
  if(aboutWidget_ != nullptr)
  {
    delete aboutWidget_;
  }
  aboutWidget_ = new WidgetAbout();
  aboutWidget_->setWindowIcon(QIcon(":/resources/privacymachine.svg"));
  QLabel *logoLabel = new QLabel(aboutWidget_);
  aboutWidget_->setWindowTitle("About");
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
  // pmManager_->isBaseDiskAvailable()

  // set some window title
  slotRegenerationProgress("startup...");

  // load the application logo from images.qrc
  setWindowIcon(QIcon(":/resources/privacymachine.svg"));

  pmManager_ = new PmManager();

  // Initialiase the Configuration (on first start)
  if(!pmManager_->initConfiguration(parPmInstallPath, parVboxDefaultMachineFolder))
    return false;

  // read the user config
  pmManager_->readConfiguration();

  updateManager_ = new UpdateManager(this);

  PmVersion applicationVersion;
  if( !applicationVersion.parse(QApplication::applicationVersion()) )
  {
    IERR("Invalid PrivacyMachine Version "+QApplication::applicationVersion() );
    return false;
  }

  if (!pmManager_->isBaseDiskAvailable())
    updateManager_->setBaseDiskUpdateRequired(true);

  updateManager_->setSystemConfig( pmManager_->getSystemConfig() );
  updateManager_->setAppcastUrl( pmManager_->getAppcastUrl() );

  ui_->mainLayout_v->addWidget(updateManager_->createUpdateWidgetIfNotExisting(this));
  updateManager_->findUpdates();

  connect( updateManager_,
           &UpdateManager::signalFinished,
           this,
           &WindowMain::slotUpdateManagerFinished);

  return true;
}

void WindowMain::slotUpdateManagerFinished()
{
  ILOG("signalFinished received")

  // remove and hide UpdateWidget from mainLayout (deletion is done inside the UpdateManager)
  updateManager_->getUpdateWidget()->hide();
  ui_->mainLayout_v->removeWidget(updateManager_->getUpdateWidget());

  if (pmManager_->isBaseDiskAvailable())
  {
    // we can now validate configuration based on the BaseDisk capabilites
    if (!pmManager_->validateConfiguration())
    {
      QString message = QCoreApplication::translate("mainfunc", "Configuration (based on BaseDisk-data) is invalid: Please check the logfile for Details.");
      QMessageBox::warning(Q_NULLPTR, "PrivacyMachine", message);
      return;
    }

    // we finish the intialisation of the VmMask-Data
    pmManager_->initAllVmMaskData();
  }
  else
  {
    // errors should already shown, just skip if no BaseDisk is available
    return;
  }


  // regeneration of the VM-Masks is needed when we have a new BaseDisk or when the user changes an important part of the
  // configuration: i.e. the name which affects the VM-Instance
  if(pmManager_->vmMaskRegenerationNecessary() || updateManager_->vmMaskRegenerationNecessary())
  {
    // show a the UpdateWidget and execute the commands
    regenerateVmMasks();
  }
  else
  {
    // show the WidgetNewTab
    setupTabWidget();
  }
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
  for(PmCommand* cmd : commandsList)
  {
    cmd->executeBlocking(false);
  }
}

void WindowMain::regenerateVmMasks()
{

  if (regenerationWidget_ == nullptr)
  {
    regenerationWidget_ = new WidgetUpdate(this);

    connect(regenerationWidget_,
            &WidgetUpdate::signalUpdateFinished,
            this,
            &WindowMain::slotRegenerationFinished);

    ui_->mainLayout_v->addWidget(regenerationWidget_);
  }

  // Create the commands including a cleanup
  if (!regenerationWidget_->init(pmManager_, true))
  {
    IERR("failed to initialize regenerationWidget (updateWidget) ");
    delete regenerationWidget_;
    regenerationWidget_ = nullptr;
    return;
  }

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
          &VmInfoIpAddress::signalUpdateIpSuccess,
          this,
          &WindowMain::slotRegenerationIpSuccess);

  connect( rdpView, 
           &WidgetRdpView::signalScreenResize,
           this,
           &WindowMain::slotRdpViewScreenResize);
  
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
  QString title = QApplication::applicationName();
  if (regenerationWidget_ != nullptr)
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
  if( widgetRdpView != nullptr )
  {    
    VmMaskData* vmMask = pmManager_->getVmMaskData()[widgetRdpView->getVmMaskId()];

    // Disable screen resize messages
    disconnect( widgetRdpView,
                &WidgetRdpView::signalScreenResize,
                this,
                &WindowMain::slotRdpViewScreenResize);

    // In case we still try to obtain the IP address, cancel it.
    vmMask->Instance->getInfoIpAddress()->abort();
    disconnect( vmMask->Instance->getInfoIpAddress().data(),
                &VmInfoIpAddress::signalUpdateIpSuccess,
                this,
                &WindowMain::slotRegenerationIpSuccess);

    // Copy VPN logs before the machine shuts down.
    QList<PmCommand*> commandsList;
    PmCommand* pCurrentCommand = nullptr;
    pCurrentCommand = GetPmCommandForScp2Host(constRootUser, constLocalIp, QString::number( vmMask->Instance->getConfig()->getSshPort()), constRootPwd,
                                              pmManager_->getPmConfigDir().path() + "/logs/vmMask_" + vmMask->Instance->getConfig()->getName() + "_vpnLog.txt",
                                              "/var/log/openvpn.log");
    pCurrentCommand->setDescription("Copy vpn logs of VM-Mask " + vmMask->Instance->getConfig()->getName());
    pCurrentCommand->setTimeoutMilliseconds(2000);
    commandsList.append( pCurrentCommand );

    // additional append the commands to close the machine
    pmManager_->createCommandsToCloseVmMask(vmMask->UserConfig->getVmName(), vmMask->UserConfig->getFullName(), commandsList);
    for(PmCommand* cmd : commandsList)
    {
      cmd->executeBlocking(false);
    }
    // remove commands and free memory
    while (commandsList.size())
      delete commandsList.takeFirst();

    // mark as inactive
    vmMask->Instance->setVmMaskIsActive(false);

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

  // While we want to be able to close VM Mask tabs in general, we would not want to close the NewTab
  // => remove 'Close' button for this tab only.
  tabWidget_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

  connect(tabWidget_,
          &QTabWidget::tabCloseRequested,
          this,
          &WindowMain::slotTabCloseRequested);

  connect(this,
          &WindowMain::signalVmMaskClosed,
          newTab,
          &WidgetNewTab::slotVmMaskClosed);

  connect(newTab,
          &WidgetNewTab::signalNewVmMaskReady,
          this,
          &WindowMain::slotNewVmMaskStarted);
  
  connect( tabWidget_,
           &QTabWidget::currentChanged,
           this,
           &WindowMain::slotTabCurrentChanged);

  return true;
}

void WindowMain::statusBarUpdate()
{
  // If this actually is a RDP Widget, attempt to update the status bar with widget info and VM Mask info, if available
  // For all other widgets, we clear the status bar text.
  WidgetRdpView* widgetRdpView = dynamic_cast<WidgetRdpView*>( currentWidget_ );
  QString statusText = "";
  if( widgetRdpView != nullptr )
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
  if (pmManager_)
  {
    delete pmManager_;
    pmManager_ = nullptr;
  }

  if (aboutWidget_)
  {
    delete aboutWidget_;
    aboutWidget_ = nullptr;
  }

  if (updateManager_)
  {
    delete updateManager_;
    updateManager_ = nullptr;
  }

  if (ui_)
  {
    delete ui_;
    ui_ = nullptr;
  }
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
      ui_->statusbar->showMessage(message);
      break;

    case success:
      message += "Generation of VM-Masks was successful.";
      ui_->statusbar->showMessage(message, 5000);
      break;

    default: // failed
      message += "Generation of VM-Masks failed. Please check the logfile for Details.";
      ui_->statusbar->showMessage(message);
      QMessageBox::warning(this, "PrivacyMachine", message);
  }

  // hide the regeneration widget
  regenerationWidget_->hide();
  ui_->mainLayout_v->removeWidget(regenerationWidget_);

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

  QString messageBoxText = "Are you shure you want to close the PrivacyMachine and all opened VM-Masks?";
  if (pmManager_)
  {
    bool oneVmMaskIsActive = false;
    for (int vmMaskId = 0; vmMaskId < pmManager_->getVmMaskData().count(); vmMaskId++)
    {
      VmMaskData* vmMask = pmManager_->getVmMaskData()[vmMaskId];
      if (vmMask->Instance != nullptr && vmMask->Instance->getVmMaskIsActive())
      {
        oneVmMaskIsActive = true;
       break;
      }
    }
    if (oneVmMaskIsActive)
    {
      QMessageBox msgBox;
      msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
      msgBox.setWindowTitle(QApplication::applicationName());
      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
      msgBox.setText(messageBoxText);
      if (msgBox.exec() == QMessageBox::Abort)
        return;
    }
  }

  if (regenerationWidget_ != nullptr)
  {
    // Stop the running update
    regenerationWidget_->abort();
    ui_->mainLayout_v->removeWidget(regenerationWidget_);
    delete regenerationWidget_;
    regenerationWidget_=nullptr;
  }

  if (aboutWidget_ != nullptr)
  {
    delete aboutWidget_;
    aboutWidget_ = nullptr;
  }


  ILOG("Restoring all VM snapshots...");
  QList<PmCommand*> commandsList;
  pmManager_->createCommandsToCloseAllVmMasks( commandsList );
  //pmManager_->createCommandsCleanupVirtualBoxVms( commandsList );
  for(PmCommand* cmd : commandsList)
  {
    cmd->executeBlocking(false);
  }

  ILOG("Saving internals")
}

