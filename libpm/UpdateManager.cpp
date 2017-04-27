#include "UpdateManager.h"
#include "VerifiedDownload.h"
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QDirIterator>
#include <QAbstractButton>

UpdateManager::UpdateManager(QObject *parent) :
  QObject(parent)
{
  ptrCheckUpdate_ = NULL;
  ptrInteractiveUpdateWidget_ = NULL;
  ptrSystemConfig_ = NULL;
  baseDiskUpdateRequired_ = false;
  vmMaskRegenerationNecessary_ = false;
  ptrVerifiedDownload_ = NULL;
  ptrExternalProcess_ = NULL;
}

UpdateManager::~UpdateManager()
{
  if (ptrInteractiveUpdateWidget_ != NULL)
  {
    delete (ptrInteractiveUpdateWidget_);
    ptrInteractiveUpdateWidget_ = NULL;
  }
}

void UpdateManager::slotCheckUpdateFinished()
{
  if (ptrCheckUpdate_ == NULL)
    return;

  if( ptrCheckUpdate_->getError() != CheckUpdate::NoError)
  {
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    QString message = QCoreApplication::translate("CheckUpdate Error msgBox",
                                                  "<h2>Checking for Updates failed</h2> <p>&nbsp;</p>");

    message += QCoreApplication::translate("CheckUpdate Error msgBox",
                                           "<p><b>Please check your network connection or the entry 'PMUpdateUrl' in PrivacyMachine.ini</b></p>");
    message += ptrCheckUpdate_->getErrorString();
    msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
    msgBox.button(QMessageBox::Ignore)->setText("Ignore and Continue Anyway");
    msgBox.button(QMessageBox::Abort)->setText("Quit");

    msgBox.setText(message);
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    int ret = msgBox.exec();
    if (ret == QMessageBox::Abort)
    {
      // TODO: Signal the mainwindow a clean shutdown
    }
    else
    {
      ILOG("User pressed Button 'Ignore and Continue Anyway' at Error of CheckUpdate: " + ptrCheckUpdate_->getErrorString());
    }
  }

  if( (ptrCheckUpdate_->getAvailableBaseDiskUpdates().size() > 0) ||
      (ptrCheckUpdate_->getAvailableBinaryUpdates().size() > 0)   ||
      (ptrCheckUpdate_->getAvailableConfigUpdates().size() > 0) )
  {
    if (ptrCheckUpdate_->getAvailableBinaryUpdates().size() > 0)
    {
      slotShowBinaryUpdate();
      return;
    }
    else if (ptrCheckUpdate_->getAvailableConfigUpdates().size() > 0)
    {
      slotShowConfigUpdate();
      return;
    }
    else if (ptrCheckUpdate_->getAvailableBaseDiskUpdates().size() > 0)
    {
      slotShowBaseDiskUpdate();
      return;
    }
    else
    {
      ILOG("no update needed");
    }
  }

  // the finished signal is needed to proceed
  emit signalFinished();
}

void UpdateManager::slotShowBinaryUpdate()
{
  if (ptrCheckUpdate_ == NULL)
    return;

  // Connect slots to continue the Update
  if (ptrCheckUpdate_->getAvailableConfigUpdates().size() > 0)
  {
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotShowConfigUpdate);

    connect (this,
             &UpdateManager::signalUpdateFinished,
             this,
             &UpdateManager::slotShowConfigUpdate);
  }
  else if (ptrCheckUpdate_->getAvailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotShowBaseDiskUpdate);

    connect (this,
             &UpdateManager::signalUpdateFinished,
             this,
             &UpdateManager::slotShowBaseDiskUpdate);
  }
  else
  {
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotReEmitSignalFinished);

    connect (this,
             &UpdateManager::signalUpdateFinished,
             this,
             &UpdateManager::slotReEmitSignalFinished);
  }

  ptrInteractiveUpdateWidget_->setSkipButtonVisible(true);
  ptrInteractiveUpdateWidget_->setTitle("<h1>PrivacyMachine update available</h1>");
  ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(true);
  ptrInteractiveUpdateWidget_->setUpdateEffectsText("<h3><em>This update requires a restart of the PrivacyMachine.</em></h3>");
  ptrInteractiveUpdateWidget_->setButtonsVisible(true);
  ptrInteractiveUpdateWidget_->setTextEditVisible(true);
  ptrInteractiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteractiveUpdateWidget_->showUpdate(ptrCheckUpdate_->getAvailableBinaryUpdates());

  connect (ptrInteractiveUpdateWidget_,
           &WidgetInteractiveUpdate::signalUpdateRequested,
           this,
           &UpdateManager::slotUpdateRequested);
}

void UpdateManager::slotShowConfigUpdate()
{
  if (ptrCheckUpdate_ == NULL)
    return;

  // TODO: maybe dissconnect old connections

  // Connect slots to continue Update
  if (ptrCheckUpdate_->getAvailableBaseDiskUpdates().size() > 0)
  {
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotShowBaseDiskUpdate);

    connect (this,
             &UpdateManager::signalUpdateFinished,
             this,
             &UpdateManager::slotShowBaseDiskUpdate);
  }
  else
  {
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotReEmitSignalFinished);

    connect (this,
             &UpdateManager::signalUpdateFinished,
             this,
             &UpdateManager::slotReEmitSignalFinished);
  }

  ptrInteractiveUpdateWidget_->setSkipButtonVisible(true);
  ptrInteractiveUpdateWidget_->setTitle("<h1>Configuration update available</h1>");
  ptrInteractiveUpdateWidget_->setButtonsVisible(true);
  ptrInteractiveUpdateWidget_->setTextEditVisible(true);
  ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(false);
  ptrInteractiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteractiveUpdateWidget_->showUpdate(ptrCheckUpdate_->getAvailableConfigUpdates());

  connect (ptrInteractiveUpdateWidget_,
           &WidgetInteractiveUpdate::signalUpdateRequested,
           this,
           &UpdateManager::slotUpdateRequested);
}

void UpdateManager::slotShowBaseDiskUpdate()
{
  // TODO: maybe dissconnect old connections

  // BaseDisk update is the latest possible Update so emit signalFinished() after completion
  connect ( this,
            &UpdateManager::signalUpdateFinished,
            this,
            &UpdateManager::slotReEmitSignalFinished);

  if (baseDiskUpdateRequired_)
  {
    ptrInteractiveUpdateWidget_->setTitle("<h1>Need to download a BaseDisk</h1>");
    ptrInteractiveUpdateWidget_->setSkipButtonVisible(false);
    ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(false);
  }
  else
  {
    ptrInteractiveUpdateWidget_->setTitle("<h1>BaseDisk update available</h1>");
    ptrInteractiveUpdateWidget_->setSkipButtonVisible(true);
    ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(true);
    connect (ptrInteractiveUpdateWidget_,
             &WidgetInteractiveUpdate::signalUpdateSkipped,
             this,
             &UpdateManager::slotReEmitSignalFinished);
  }

  ptrInteractiveUpdateWidget_->setUpdateEffectsText("<h3><em>This update requires the regeneration of all VM-Masks.</em></h3>");
  ptrInteractiveUpdateWidget_->setButtonsVisible(true);
  ptrInteractiveUpdateWidget_->setTextEditVisible(true);
  ptrInteractiveUpdateWidget_->setProgressBarVisible(false);
  ptrInteractiveUpdateWidget_->showUpdate(ptrCheckUpdate_->getAvailableBaseDiskUpdates());

  connect (ptrInteractiveUpdateWidget_,
           &WidgetInteractiveUpdate::signalUpdateRequested,
           this,
           &UpdateManager::slotUpdateRequested);
}

bool UpdateManager::findUpdates()
{
  if (ptrCheckUpdate_ == NULL)
  {
    ptrCheckUpdate_ = new CheckUpdate();
  }

  if(!isReady())
  {
    IERR("UpdateManager: Could not start to check for updates");
    return false;
  }
  ptrCheckUpdate_->setCurrentBaseDiskVersion(ptrSystemConfig_->getBaseDiskVersion());
  ptrCheckUpdate_->setCurrentBinaryVersion(ptrSystemConfig_->getBinaryVersion());
  ptrCheckUpdate_->setCurrentConfigVersion(ptrSystemConfig_->getConfigVersion());
  ptrCheckUpdate_->setUrl(appcastUrl_);

  connect( ptrCheckUpdate_,
           &CheckUpdate::finished,
           this,
           &UpdateManager::slotCheckUpdateFinished);

  if (ptrInteractiveUpdateWidget_ == NULL)
  {
    IERR("Interacive Update started but no WidgetInteractiveUpdate was created!");
    return false;
  }

  ptrInteractiveUpdateWidget_->setTitle("<h1>Checking for Updates</h1>");
  ptrInteractiveUpdateWidget_->setButtonsVisible(false);
  ptrInteractiveUpdateWidget_->setTextEditVisible(false);
  ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(false);
  ptrInteractiveUpdateWidget_->setTextEditTitleVisible(false);
  ptrInteractiveUpdateWidget_->setProgressBarVisible(true);
  ptrInteractiveUpdateWidget_->setProgressBarAbortButtonVisible(false);
  ptrInteractiveUpdateWidget_->setProgressBarRange(0,0); //indicates busy
  ptrInteractiveUpdateWidget_->setProgressBarText("Downloading "+appcastUrl_.toString());

  if (!ptrCheckUpdate_->start())
  {
    IERR("Checking for updates failed!");
    return false;
  }
}

WidgetInteractiveUpdate* UpdateManager::getUpdateWidget()
{
  return ptrInteractiveUpdateWidget_;
}

WidgetInteractiveUpdate* UpdateManager::createUpdateWidgetIfNotExisting(QWidget *parParent)
{
  if (ptrInteractiveUpdateWidget_ == NULL)
  {
    ptrInteractiveUpdateWidget_ = new WidgetInteractiveUpdate(parParent);
  }
  return ptrInteractiveUpdateWidget_;
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
  ptrVerifiedDownload_->setHashAlgorithm(QCryptographicHash::Sha256);

  ptrVerifiedDownload_->setUrl(progressedUpdate_.Url);
  ptrVerifiedDownload_->setCheckSum(progressedUpdate_.CheckSum);

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

  connect( ptrVerifiedDownload_,
           &VerifiedDownload::finished,
           this,
           &UpdateManager::slotUpdateDownloadFinished);

  connect( ptrInteractiveUpdateWidget_,
           &WidgetInteractiveUpdate::signalAbortButtonPressed,
           ptrVerifiedDownload_,
           &VerifiedDownload::abort);

  ptrInteractiveUpdateWidget_->setProgressBarAbortButtonVisible(true);

  connect( ptrVerifiedDownload_,
           &VerifiedDownload::downloadProgress,
           ptrInteractiveUpdateWidget_,
           &WidgetInteractiveUpdate::slotProgressBarUpdate);

  ptrInteractiveUpdateWidget_->setProgressBarText("Downloading "+ptrVerifiedDownload_->getUrl().toString());
  // Indicate busy but as soon as download started the Range will be updated
  ptrInteractiveUpdateWidget_->setProgressBarRange(0,0);
  ptrInteractiveUpdateWidget_->setProgressBarVisible(true);
  ptrInteractiveUpdateWidget_->setProgressBarAbortButtonVisible(true);

  switch (update.Type)
  {
    case Update::BaseDisk:
      ptrInteractiveUpdateWidget_->setTitle("<h1>Downloading BaseDisk<h1>");
      break;
    case Update::Binary:
      ptrInteractiveUpdateWidget_->setTitle("<h1>Downloading PrivacyMachine<h1>");
      break;
    case Update::Config:
      ptrInteractiveUpdateWidget_->setTitle("<h1>Downloading Config<h1>");
  }


  ptrInteractiveUpdateWidget_->setTextEditTitleVisible(false);
  ptrInteractiveUpdateWidget_->setButtonsVisible(false);
  ptrInteractiveUpdateWidget_->setTextEditVisible(false);
  ptrInteractiveUpdateWidget_->setUpdateEffectsVisible(false);

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
  // TODO: implement me
}
void UpdateManager::configUpdateInstallRequested()
{
  // TODO: implement me
}
void UpdateManager::baseDiskUpdateInstallRequested()
{
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

  ptrInteractiveUpdateWidget_->setProgressBarText("Extracting new BaseDisk");
  ptrInteractiveUpdateWidget_->setProgressBarRange(0,0); // indicate busy
  ptrInteractiveUpdateWidget_->setProgressBarAbortButtonVisible(false);

  connect( ptrExternalProcess_,
           static_cast<void (QProcess::*)(int, QProcess::ExitStatus)> (&QProcess::finished),
           this,
           &UpdateManager::slotBaseDiskExtractionFinished);

  ptrExternalProcess_->start(cmd,args);
}

void UpdateManager::slotBaseDiskExtractionFinished()
{
  QString stdErr(ptrExternalProcess_->readAllStandardError());
  QString stdOut(ptrExternalProcess_->readAllStandardOutput());

  // very extended Error checking because we cant trust the exit code of 7za
  if( ptrExternalProcess_->exitStatus() != QProcess::NormalExit || ptrExternalProcess_->exitCode() != 0 ||
      stdErr.toLower().indexOf("error") != -1 || stdOut.toLower().indexOf("error") != -1 )
  {
    IERR("Could not extract " + ptrSystemConfig_->getBaseDiskPath() + "/" + progressedUpdate_.Url.fileName() +
         "\nExit code " + ptrExternalProcess_->exitCode() + "\nOutput:\n" + stdOut + "\nError:\n" + stdErr);
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resources/privacymachine.svg"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(QApplication::applicationName());
    msgBox.setStandardButtons(QMessageBox::Abort);
    QString message = QCoreApplication::translate("Update extraction failed",
                                        "<h2>Exraction of the BaseDisk failed!</h2>");
    message += "\n<p>\nExit code: " + QString(ptrExternalProcess_->exitCode()) + "</p>\n<p>Output:</p>\n<p>" + stdOut + "</p>\n<p>Error:</p>\n<p>" + stdErr+"</p>";
    msgBox.setText(message);
    msgBox.exec();
    emit signalUpdateFinished();
    return;
  }
  ILOG("Extraction successful. Output: \n" + QString(ptrExternalProcess_->readAllStandardOutput()));


  // TODO: if delta update patch here!!

  ptrInteractiveUpdateWidget_->setProgressBarText("Deleting old BaseDisk");
  ptrInteractiveUpdateWidget_->setProgressBarRange(0,0); //indicate busy
  ptrInteractiveUpdateWidget_->setProgressBarAbortButtonVisible(false);

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

  // update SystemConfig

  vmMaskRegenerationNecessary_=true;

  // extract BaseDiskName (BaseDisk_x) from BaseDisk_x.7z
  ptrSystemConfig_->setBaseDiskName( progressedUpdate_.Url.fileName().split(".").at(0) );

  ptrSystemConfig_->setBaseDiskVersion(progressedUpdate_.Version);

  ptrSystemConfig_->write();

  emit signalUpdateFinished();
}
