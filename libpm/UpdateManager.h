#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QUrl>
#include <QProcess>
#include "PmVersion.h"
#include "WidgetInteractiveUpdate.h"
#include "CheckUpdate.h"
#include "SystemConfig.h"
#include "VerifiedDownload.h"

class UpdateManager : public QObject
{
    Q_OBJECT
  public:

    explicit UpdateManager(QObject *parent = 0);

    virtual ~UpdateManager();

    /// \brief getUpdateWidget
    /// \brief returns the UpdateWidget to interact with the user
    /// \return UpdateWidget*
    WidgetInteractiveUpdate* getUpdateWidget();

    /// \brief getUpdateWidget
    /// \brief creates a new UpdateWidget if it does not already exist
    /// \param parParent
    /// \return UpdateWidget*
    WidgetInteractiveUpdate* createUpdateWidgetIfNotExisting(QWidget *parParent);

    /// \brief findUpdates
    /// \brief Download and parse appcast
    /// \return true if download of appcast started successfully
    bool findUpdates();


    /// \brief isReady
    /// \return true if appcastUrl is valid and SystemConfiguration is set
    bool isReady();


    /// \brief vmMaskRegenerationNecessary
    /// \return true if we need to destroy the VmMasks to complete update
    bool vmMaskRegenerationNecessary(){ return vmMaskRegenerationNecessary_; }

    /// setter

    /// \brief setAppcastUrl
    /// \param appcastUrl
    void setAppcastUrl(QUrl appcastUrl) {appcastUrl_ = appcastUrl;}

    /// \brief setSystemConfig
    /// \brief used to determine currend versions and update them
    /// \param ptrSystemConfig
    void setSystemConfig(SystemConfig* ptrSystemConfig) {ptrSystemConfig_ = ptrSystemConfig;}

    /// \brief setBaseDiskUpdateRequired
    /// \brief if set forces the user to download a new BaseDisk
    /// \param baseDiskUpdateRequired
    void setBaseDiskUpdateRequired(bool baseDiskUpdateRequired) {baseDiskUpdateRequired_ = baseDiskUpdateRequired;}

  signals:
    /// \brief signalUpdatesFound
    /// \brief emmited if updates available
    void signalUpdatesFound();

    /// \brief signalFinished
    /// \brief emmited if whole update process finished
    void signalFinished();

    /// \brief signalUpdateFinished
    /// \brief emmited if a update is finished
    void signalUpdateFinished();

  public slots:
    /// \brief slotUpdateRequested
    /// \brief starts the download of an Update with VerifiedDownload
    /// \param update
    void slotUpdateRequested(Update update);


  private slots:

    /// \brief slotCheckUpdateFinished
    /// \brief does error handling after findUpdates() and calles slotShow*Update()
    void slotCheckUpdateFinished();

    /// \brief slotShowBinaryUpdate
    /// \brief displays a Binary update possibility in WidgetInteraciveUpdate
    void slotShowBinaryUpdate();

    /// \brief slotShowConfigUpdate
    /// \brief displays a Config update possibility in WidgetInteraciveUpdate
    void slotShowConfigUpdate();

    /// \brief slotShowBaseDiskUpdate
    /// \brief displays a BaseDisk update possibility in WidgetInteraciveUpdate
    void slotShowBaseDiskUpdate();

    /// \brief slotUpdateDownloadFinished
    /// \brief does the error handling of VerifiedDownload and calls *UpdateInstallRequested
    void slotUpdateDownloadFinished();


    /// \brief slotBaseDiskExtractionFinished
    /// \brief does the error handling of baseDiskUpdateInstallRequested(), removes old BaseDisk and updates SystemConfig
    void slotBaseDiskExtractionFinished();


    /// \brief slotReEmitSignalFinished
    void slotReEmitSignalFinished() {emit signalFinished();}

  private:

    /// \brief binaryUpdateInstallRequested
    /// \brief not implemented jet
    void binaryUpdateInstallRequested();

    /// \brief configUpdateInstallRequested
    /// \brief not implemented jet
        void configUpdateInstallRequested();

    /// \brief baseDiskUpdateInstallRequested
    /// \brief start extraction of new BaseDisk
    void baseDiskUpdateInstallRequested();

    /// private variables:
    WidgetInteractiveUpdate* ptrInteractiveUpdateWidget_;
    CheckUpdate* ptrCheckUpdate_;
    QUrl appcastUrl_;
    PmVersion currentBaseDiskVersion_;
    PmVersion currentBinaryVersion_;
    PmVersion currentConfigVersion_;
    bool baseDiskUpdateRequired_;
    bool vmMaskRegenerationNecessary_;
    bool interactive_;
    SystemConfig* ptrSystemConfig_;
    VerifiedDownload* ptrVerifiedDownload_;
    Update progressedUpdate_;
    QProcess *ptrExternalProcess_;
};


/// helper functions




#endif // UPDATEMANAGER_H
