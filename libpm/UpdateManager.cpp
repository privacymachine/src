#include "UpdateManager.h"
#include "VerifiedDownload.h"
#include <stdexcept>
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QIcon>


UpdateManager::UpdateManager(QObject *parent) :
  QObject(parent), checkUpdate_(this)
{
  ptrInteraktiveUpdateWidget_ = NULL;
  ptrSystemConfig_ = NULL;
  baseDiskUpdateRequired_ = false;
  interactive_ = false;
  vmMaskRegenerationNecessary_ = false;
}

UpdateManager::~UpdateManager()
{
  if (ptrInteraktiveUpdateWidget_ != NULL) ptrInteraktiveUpdateWidget_->deleteLater();
}

void UpdateManager::slotCheckUpdateFinished()
{
  if( checkUpdate_.getError() != CheckUpdate::NoError)
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    QString message = QCoreApplication::translate("CheckUpdate Error msgBox","<h3>Error occured at checking for Updates</h3> \n");
    message += checkUpdate_.getErrorString();
    msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
    ///@todo bernhard: why is this not working?!
//    msgBox.button(QMessageBox::Ignore)->setText("Ignore and Continue Anyway");
//    msgBox.button(QMessageBox::Abort)->setText("Quit");
    msgBox.setText(message);
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    int ret = msgBox.exec();
    if (ret == QMessageBox::Abort)
    {
      exit(0);
    }
    else
    {
      ILOG("User pressed Button 'Ignore and Continue Anyway' at Error of CheckUpdtae: "+checkUpdate_.getErrorString());
    }
  }

  if( (checkUpdate_.getavailableBaseDiskUpdates().size() > 0) ||
      (checkUpdate_.getavailableBinaryUpdates().size() > 0) ||
      (checkUpdate_.getavailableConfigUpdates().size() > 0) )
  {
    emit signalUpdatesFound();
    if (interactive_)
    {
      if (checkUpdate_.getavailableBinaryUpdates().size() > 0)
      {
        slotShowBinaryUpdate();
      }
      else if (checkUpdate_.getavailableConfigUpdates().size() > 0)
      {
        slotShowConfigUpdate();
      }
      else if (checkUpdate_.getavailableBaseDiskUpdates().size() > 0)
      {
        slotShowBaseDiskUpdate();
      }
    }
  }
  else
    emit signalFinished();
}

void UpdateManager::slotShowBinaryUpdate()
{
  ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
  if (checkUpdate_.getavailableConfigUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowConfigUpdate()));
  }
  else if (checkUpdate_.getavailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>PrivacyMachine update available</h1>");
  ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateEffectsText("<h3><em>This update requires a restart of the PrivacyMachine.</em></h3>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->showUpdate(checkUpdate_.getavailableBinaryUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotBinaryUpdateRequested(Update)));
}

void UpdateManager::slotShowConfigUpdate()
{
  ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
  if (checkUpdate_.getavailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>Configuration update available</h1>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(false);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->showUpdate(checkUpdate_.getavailableConfigUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotConfigUpdateRequested(Update)));
}

void UpdateManager::slotShowBaseDiskUpdate()
{
  if (baseDiskUpdateRequired_)
  {
    ptrInteraktiveUpdateWidget_->setTitle("<h1>Need to download a BaseDisk</h1>");
    ptrInteraktiveUpdateWidget_->setSkipButtonVisible(false);
    ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(false);
  }
  else
  {
    ptrInteraktiveUpdateWidget_->setTitle("<h1>BaseDisk update available</h1>");
    ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
    ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(true);
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }

  ptrInteraktiveUpdateWidget_->setUpdateEffectsText("<h3><em>This update requires the regeneration of all VM-Masks.</em></h3>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->showUpdate(checkUpdate_.getavailableBaseDiskUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotBaseDiskUpdateRequested(Update)));
}

bool UpdateManager::findUpdates()
{
  if(!isReady())
  {
    IERR("UpdateManager: Could not start to check for updates");
    return false;
  }
  checkUpdate_.setCurrentBaseDiskVersion(ptrSystemConfig_->getBaseDiskVersion());
  checkUpdate_.setCurrentBinaryVersion(ptrSystemConfig_->getBinaryVersion());
  checkUpdate_.setCurrentConfigVersion(ptrSystemConfig_->getConfigVersion());
  checkUpdate_.setUrl(appcastUrl_);

  connect( &checkUpdate_, SIGNAL(finished()), this, SLOT(slotCheckUpdateFinished()) );
  if (interactive_)
  {
    if (ptrInteraktiveUpdateWidget_ == NULL)
    {
      IERR("Interacive Update started but no WidgetInteraktiveUpdate was created!");
      return false;
    }

    ptrInteraktiveUpdateWidget_->setTitle("<h1>Checking for Updates</h1>");
    ptrInteraktiveUpdateWidget_->setButtonsVisible(false);
    ptrInteraktiveUpdateWidget_->setTextEditVisible(false);
    ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(false);
    ptrInteraktiveUpdateWidget_->setUpdateTitleVisible(false);
    ptrInteraktiveUpdateWidget_->setProgressBarVisible(true);
    ptrInteraktiveUpdateWidget_->setProgressBarRange(0,0); //indicates busy
    ptrInteraktiveUpdateWidget_->setProgressBarText("Downloading "+appcastUrl_.toString());
  }

  if (!checkUpdate_.start())
  {
    IERR("Checking for updates failed!");
    return false;
  }
}

WidgetInteraktiveUpdate* UpdateManager::getUpdateWidget(QWidget *parParent)
{
  interactive_ = true;
  if (ptrInteraktiveUpdateWidget_ == NULL) ptrInteraktiveUpdateWidget_ = new WidgetInteraktiveUpdate(parParent);
  connect( ptrInteraktiveUpdateWidget_, SIGNAL(destroyed()), this, SLOT(slotInteraktiveUpdateWidgetDestroyed()) );
  return ptrInteraktiveUpdateWidget_;
}

bool UpdateManager::isReady()
{
  bool ready = true;

  if( ptrSystemConfig_ == NULL )
  {
    IERR("UpdateManager: no SystemConfig is set");
    ready = false;
  }

  if( !appcastUrl_.isValid() )
  {
    IERR("UpdateManager: no valid appcastUrl is set");
    ready = false;
  }

  return ready;
}

void UpdateManager::slotBinaryUpdateRequested(Update binaryUpdate)
{

}

void UpdateManager::slotBaseDiskUpdateRequested(Update baseDiskUpdate)
{

}

void UpdateManager::slotConfigUpdateRequested(Update configUpdate)
{
  VerifiedDownload downloader(this);
  downloader.setHashAlgo(QCryptographicHash::Sha3_256);
//  downloader.setDownloadTargetDir();
}

