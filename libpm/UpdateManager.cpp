#include "UpdateManager.h"

UpdateManager::UpdateManager(QObject *parent) :
  QObject(parent), checkUpdate_(this)
{
  ptrInteraktiveUpdateWidget_ = NULL;
  baseDiskUpdateRequired_ = false;
  interactive_ = false;
}

UpdateManager::~UpdateManager()
{
  if (ptrInteraktiveUpdateWidget_ != NULL) ptrInteraktiveUpdateWidget_->deleteLater();
}

void UpdateManager::slotCheckUpdateFinished()
{
  if( checkUpdate_.getError() != CheckUpdate::NoError)
  {
    /// @todo: implement error handling
    return;
  }

  if( (checkUpdate_.getAvaiableBaseDiskUpdates().size() > 0) ||
      (checkUpdate_.getAvaiableBinaryUpdates().size() > 0) ||
      (checkUpdate_.getAvaiableConfigUpdates().size() > 0) )
  {
    emit signalUpdatesFound();
    if (interactive_)
    {
      if (checkUpdate_.getAvaiableBinaryUpdates().size() > 0)
      {
        slotShowBinaryUpdate();
      }
      else if (checkUpdate_.getAvaiableConfigUpdates().size() > 0)
      {
        slotShowConfigUpdate();
      }
      else if (checkUpdate_.getAvaiableBaseDiskUpdates().size() > 0)
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
  if (checkUpdate_.getAvaiableConfigUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowConfigUpdate()));
  }
  else if (checkUpdate_.getAvaiableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>PrivacyMachine update avaiable</h1>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateSelectorVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->setupRadioButtons(checkUpdate_.getAvaiableBinaryUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotBinaryUpdateRequested(Update)));
}

void UpdateManager::slotShowConfigUpdate()
{
  ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
  if (checkUpdate_.getAvaiableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>PrivacyMachine-Configuration update avaiable</h1>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateSelectorVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->setupRadioButtons(checkUpdate_.getAvaiableConfigUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotConfigUpdateRequested(Update)));
}

void UpdateManager::slotShowBaseDiskUpdate()
{
  if (baseDiskUpdateRequired_)
  {
    ptrInteraktiveUpdateWidget_->setTitle("<h1>Need to download a BaseDisk</h1>");
    ptrInteraktiveUpdateWidget_->setSkipButtonVisible(false);
  }
  else
  {
    ptrInteraktiveUpdateWidget_->setTitle("<h1>BaseDisk update avaiable</h1>");
    ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
  }

  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateSelectorVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->setupRadioButtons(checkUpdate_.getAvaiableBaseDiskUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotBaseDiskUpdateRequested(Update)));
}

bool UpdateManager::findUpdates()
{
  checkUpdate_.setCurrentBaseDiskVersion(currentBaseDiskVersion_);
  checkUpdate_.setCurrentBinaryVersion(currentBinaryVersion_);
  checkUpdate_.setCurrentConfigVersion(currentConfigVersion_);
  checkUpdate_.setUrl(appcastUrl_);

  connect( &checkUpdate_, SIGNAL(finished()), this, SLOT(slotCheckUpdateFinished()) );
  if (interactive_)
  {
    if (ptrInteraktiveUpdateWidget_ == NULL)
    {
      /// todo: error handling
    }

    ptrInteraktiveUpdateWidget_->setTitle("<h1>Checking for Updates</h1>");
    ptrInteraktiveUpdateWidget_->setButtonsVisible(false);
    ptrInteraktiveUpdateWidget_->setUpdateSelectorVisible(false);
    ptrInteraktiveUpdateWidget_->setTextEditVisible(false);
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
  return ptrInteraktiveUpdateWidget_;
}

void UpdateManager::slotBinaryUpdateRequested(Update binaryUpdate)
{

}

void UpdateManager::slotBaseDiskUpdateRequested(Update baseDiskUpdate)
{

}

void UpdateManager::slotConfigUpdateRequested(Update configUpdate)
{

}
