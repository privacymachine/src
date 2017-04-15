#include "UpdateManager.h"
#include "VerifiedDownload.h"
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QDirIterator>


UpdateManager::UpdateManager(QObject *parent) :
  QObject(parent), checkUpdate_(this)
{
  ptrInteraktiveUpdateWidget_ = NULL;
  ptrSystemConfig_ = NULL;
  baseDiskUpdateRequired_ = false;
  interactive_ = false;
  vmMaskRegenerationNecessary_ = false;
  ptrVerifiedDownload_ = NULL;
  ptrExternalProcess_ = NULL;
}

UpdateManager::~UpdateManager()
{
  if (ptrInteraktiveUpdateWidget_ != NULL) ptrInteraktiveUpdateWidget_->deleteLater();
}

void UpdateManager::slotCheckUpdateFinished()
{
  if( checkUpdate_.getError() != CheckUpdate::NoError)
  {
    ///@todo: implement non interactive error handling
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    QString message = QCoreApplication::translate("CheckUpdate Error msgBox",
                                                  "<h2>Error occured at checking for Updates</h2> <p>&nbsp;</p>");

    message += QCoreApplication::translate("CheckUpdate Error msgBox",
                                           "<p><b>Please check PMUpdateUrl in PrivacyMachine.ini or start the problem reporter.</b></p>");
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
      (checkUpdate_.getavailableBinaryUpdates().size() > 0)   ||
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
    connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotShowConfigUpdate()));
  }
  else if (checkUpdate_.getavailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
    connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
    connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>PrivacyMachine update available</h1>");
  ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateEffectsText("<h3><em>This update requires a restart of the PrivacyMachine.</em></h3>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->showUpdate(checkUpdate_.getavailableBinaryUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotUpdateRequested(Update)));
}

void UpdateManager::slotShowConfigUpdate()
{
  ptrInteraktiveUpdateWidget_->setSkipButtonVisible(true);
  if (checkUpdate_.getavailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotShowBaseDiskUpdate()));
    connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotShowBaseDiskUpdate()));
  }
  else
  {
    connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateSkipped()), this, SLOT(slotEmitSignalFinished()));
    connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotEmitSignalFinished()));
  }
  ptrInteraktiveUpdateWidget_->setTitle("<h1>Configuration update available</h1>");
  ptrInteraktiveUpdateWidget_->setButtonsVisible(true);
  ptrInteraktiveUpdateWidget_->setTextEditVisible(true);
  ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(false);
  ptrInteraktiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteraktiveUpdateWidget_->showUpdate(checkUpdate_.getavailableConfigUpdates());

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotUpdateRequested(Update)));
}

void UpdateManager::slotShowBaseDiskUpdate()
{
  connect (this, SIGNAL(signalUpdateFinished()), this, SLOT(slotEmitSignalFinished()));
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

  connect (ptrInteraktiveUpdateWidget_, SIGNAL(signalUpdateRequested(Update)), this, SLOT(slotUpdateRequested(Update)));
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
    ptrInteraktiveUpdateWidget_->setTextEditTitleVisible(false);
    ptrInteraktiveUpdateWidget_->setProgressBarVisible(true);
    ptrInteraktiveUpdateWidget_->setProgressBarAbortButtonVisible(false);
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


void UpdateManager::slotUpdateRequested(Update update)
{
  progressedUpdate_ = update;
  if(ptrVerifiedDownload_ == NULL)
  {
    ptrVerifiedDownload_ = new VerifiedDownload(this);
  }
  else if( ptrVerifiedDownload_->isStarted() )
  {
    ptrVerifiedDownload_->abort();
  }

  // !! QCryptographicHash::Sha3_256 does NOT implement sha3_256 !!
  // https://bugreports.qt.io/browse/QTBUG-59770?jql=text%20~%20%22QCryptographicHash%22
  // So we use sha256 instead till qt5.9 is avaiable in debian and its distributions
  //ptrVerifiedDownload_->setHashAlgo(QCryptographicHash::Sha3_256);
  ptrVerifiedDownload_->setHashAlgo(QCryptographicHash::Sha256);

  ptrVerifiedDownload_->setUrl(progressedUpdate_.Url);
  ptrVerifiedDownload_->setSHA(progressedUpdate_.CheckSum);

  switch (progressedUpdate_.Type)
  {
    case Update::BaseDisk:
      ptrVerifiedDownload_->setDownloadTargetDir(ptrSystemConfig_->getBaseDiskPath());
      break;

    case Update::Config:

    case Update::Binary:
      ILOG("Config and Binary update process not implemented now")
      QMessageBox msgBox;
      msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setWindowTitle(QApplication::applicationName());
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setText("<h2>Config and Binary update process not implemented now</h2>");
      msgBox.exec();
      emit signalUpdateFinished();
      return;
      break;
  }

  connect( ptrVerifiedDownload_, SIGNAL(finished()), this, SLOT(slotUpdateDownloadFinished()) );

  if(interactive_)
  {
    connect( ptrInteraktiveUpdateWidget_, SIGNAL(signalAbortButtonPressed()),
             ptrVerifiedDownload_, SLOT(abort()) );
    ptrInteraktiveUpdateWidget_->setProgressBarAbortButtonVisible(true);

    connect( ptrVerifiedDownload_, SIGNAL(downloadProgress(qint64,qint64)),
             ptrInteraktiveUpdateWidget_, SLOT(slotProgressBarUpdate(qint64,qint64)) );
    ptrInteraktiveUpdateWidget_->setProgressBarText("Downloading "+ptrVerifiedDownload_->getUrl().toString());
    // Indicate busy but as soon as download started the Range will be updated
    ptrInteraktiveUpdateWidget_->setProgressBarRange(0,0);
    ptrInteraktiveUpdateWidget_->setProgressBarVisible(true);
    ptrInteraktiveUpdateWidget_->setProgressBarAbortButtonVisible(true);

    switch (update.Type)
    {
      case Update::BaseDisk:
        ptrInteraktiveUpdateWidget_->setTitle("<h1>Downloading BaseDisk<h1>");
        break;
      case Update::Binary:
        ptrInteraktiveUpdateWidget_->setTitle("<h1>Downloading PrivacyMachine<h1>");
        break;
      case Update::Config:
        ptrInteraktiveUpdateWidget_->setTitle("<h1>Downloading Config<h1>");
    }


    ptrInteraktiveUpdateWidget_->setTextEditTitleVisible(false);
    ptrInteraktiveUpdateWidget_->setButtonsVisible(false);
    ptrInteraktiveUpdateWidget_->setTextEditVisible(false);
    ptrInteraktiveUpdateWidget_->setUpdateEffectsVisible(false);

  }

  if ( !ptrVerifiedDownload_->start() )
  {
    QString errorStr="Could not start download of BaseDisk update.";
    IERR(errorStr);
    // TODO: implement non interactive error handling
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    QString message = "<h3>Error occured at downloading update</h3> \n"+
                      errorStr + "\n<b>Please quit and start the problem reporter.</b>";
    msgBox.setStandardButtons(QMessageBox::Abort);
    // TODO: bernhard: why is this not working?!
    //msgBox.button(QMessageBox::Abort)->setText("Quit");
    msgBox.setText(message);
    msgBox.exec();
    exit(1);
  }
}
void UpdateManager::slotUpdateDownloadFinished()
{
  // error handling
  if( ptrVerifiedDownload_->getError() != VerifiedDownload::NoError )
  {
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    switch (ptrVerifiedDownload_->getError())
    {

      case VerifiedDownload::Aborted:
        ILOG("User aborted the download of an update.")
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(QApplication::applicationName());
        msgBox.setText(QCoreApplication::translate("Download of update aborted",
                                         "<h2>Download aborted</h2>"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        emit signalUpdateFinished();
        return;

      case VerifiedDownload::NetworkError:
        IERR("Download of update Failed.")
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(QApplication::applicationName());
        msgBox.setText(QCoreApplication::translate("Update download failed",
                                        "<h2>Download of update Failed.</h2>"));
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.exec();
        emit signalUpdateFinished();
        return;

      case VerifiedDownload::FileReadError:
        IERR("Failed to read update file.")
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(QApplication::applicationName());
        msgBox.setText(QCoreApplication::translate("Update download failed",
                                         "<h2>Failed to read update file.</h2>"));
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.exec();
        emit signalUpdateFinished();
        return;

      case VerifiedDownload::FileWriteError:
        IERR("Failed to write update to file.")
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(QApplication::applicationName());
        msgBox.setText(QCoreApplication::translate("Update download failed",
                                         "<h2>Failed to write update to file.</h2><p>Is there enough space on the disk?</p>"));
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.exec();
        emit signalUpdateFinished();
        return;

      case VerifiedDownload::IntegrityError:
        IERR("Integrity check of the update failed")
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(QApplication::applicationName());
        msgBox.setText(QCoreApplication::translate("Update download failed",
                                         "<h2>Integrity check failed!</h2><p><b>This could mean sombody is intercepting your connection.</b></p>"));
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.exec();
        emit signalUpdateFinished();
        return;
    }
  }

  switch (progressedUpdate_.Type)
  {
    case Update::BaseDisk:
      baseDiskUpdateInstallRequested();
      break;
    case Update::Config:
      configUpdateInstallRequested();
      break;
    case Update::Binary:
      binaryUpdateInstallRequested();
      break;
  }

}

void UpdateManager::binaryUpdateInstallRequested()
{

}
void UpdateManager::configUpdateInstallRequested()
{

}
void UpdateManager::baseDiskUpdateInstallRequested()
{

  // remove old BaseDisk
  if( interactive_ )
  {
    ptrInteraktiveUpdateWidget_->setProgressBarText("Deleting old BaseDisk");
    ptrInteraktiveUpdateWidget_->setProgressBarRange(0,0); //indicate busy
    ptrInteraktiveUpdateWidget_->setProgressBarAbortButtonVisible(false);
  }

  QDirIterator it(ptrSystemConfig_->getBaseDiskPath(), QDirIterator::NoIteratorFlags);
  while (it.hasNext())
  {
    QString filePath = it.next();
    QFile ff(filePath);
    QFileInfo fileInfo(ff);

    if( fileInfo.fileName().startsWith( ptrSystemConfig_->getBaseDiskName() ) && fileInfo.isFile() )
    {
      // TODO: Is this a sensitive information?
      ILOG( "Removing " + fileInfo.absolutePath() )
      if( !ff.remove())
      {
        IERR( "Could not remove " + fileInfo.absolutePath())
      }
    }
  }


  // extract new BaseDisk



  if( ptrExternalProcess_ != NULL ) ptrExternalProcess_->deleteLater();

  ptrExternalProcess_ = new QProcess(this);

  QStringList args;
  QString cmd;

  #ifdef PM_WINDOWS
    cmd = "7za.exe";
  #else
    cmd = "7za";
  #endif

  args.clear();
  args.append( "-y" );
  args.append( "-o" + ptrSystemConfig_->getBaseDiskPath() );
  args.append( "e" );
  args.append( ptrSystemConfig_->getBaseDiskPath() + "/" + progressedUpdate_.Url.fileName() );

  ILOG("Start extracting " + progressedUpdate_.Url.fileName()+". cmd: "+ cmd +" "+args.join(" "));
      if( interactive_ )
        ptrInteraktiveUpdateWidget_->setProgressBarText("Extracting new BaseDisk");

  connect( ptrExternalProcess_, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotBaseDiskExtractionFinished()) );
  ptrExternalProcess_->start(cmd,args);

}

void UpdateManager::slotBaseDiskExtractionFinished()
{
  if( ptrExternalProcess_->exitStatus() != QProcess::NormalExit )
  {
    IERR("Could not extract " + ptrSystemConfig_->getBaseDiskPath() + "/" + progressedUpdate_.Url.fileName() +
         "\nExit code " + ptrExternalProcess_->exitCode() + "\nError:\n" + QString(ptrExternalProcess_->readAllStandardError()));
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    msgBox.setStandardButtons(QMessageBox::Abort);
    QString message = QCoreApplication::translate("Update extraction failed",
                                        "<h2>Exraction of the BaseDisk failed!</h2>");
    message += "\n<p>Error:</p>\n<p>"+QString(ptrExternalProcess_->readAllStandardError())+"</p>";
    msgBox.setText(message);
    msgBox.exec();
    emit signalUpdateFinished();
    return;
  }
  ILOG("Extraction successful. Output: \n" + QString(ptrExternalProcess_->readAllStandardOutput()));

  // update SystemConfig

  vmMaskRegenerationNecessary_=true;

  // extract BaseDiskName (BaseDisk_x) from BaseDisk_x.7z
  ptrSystemConfig_->setBaseDiskName( progressedUpdate_.Url.fileName().split(".").at(0) );

  ptrSystemConfig_->setBaseDiskVersion(progressedUpdate_.Version);

  ptrSystemConfig_->write();

  emit signalUpdateFinished();
}
