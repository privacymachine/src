#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QUrl>
#include "PmVersion.h"
#include "WidgetInteraktiveUpdate.h"
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
    /// \brief creates a UpdateWidget to interact with the user
    /// \param parParent
    /// \return UpdateWidget*
    WidgetInteraktiveUpdate* getUpdateWidget(QWidget *parParent = NULL);

    /// \brief findUpdates
    /// \brief Download and parse appcast
    /// \return true if download of appcast started successfully
    bool findUpdates();


    /// \brief isReady
    /// \return true if appcastUrl is valid and ScstemConfiguration is set
    bool isReady();


    /// \brief vmMaskRegenerationNecessary
    /// \return true if we need to destroy the VmMasks to complete update
    bool vmMaskRegenerationNecessary(){ return vmMaskRegenerationNecessary_; }

    /// setter

    /// \brief setAppcastUrl
    /// \param appcastUrl
    void setAppcastUrl(QUrl appcastUrl) {appcastUrl_=appcastUrl;}

    /// \brief setInteractiveUpdate
    /// \brief activate to use the interactive frontend WidgetInteractiveUpdate
    /// \param interactive
    /// \@todo: non interactive update not implemented jet
    void setInteractiveUpdate(bool interactive) {interactive_=interactive;}

    /// \brief setSystemConfig
    /// \brief used to determine currend versions and update them
    /// \param ptrSystemConfig
    void setSystemConfig(SystemConfig* ptrSystemConfig) {ptrSystemConfig_ = ptrSystemConfig;}

    /// \brief setBaseDiskUpdateRequired
    /// \brief if set forces the user to download a new BaseDisk
    /// \param baseDiskUpdateRequired
    void setBaseDiskUpdateRequired(bool baseDiskUpdateRequired) {baseDiskUpdateRequired_=baseDiskUpdateRequired;}

  signals:
    /// \brief signalUpdatesFound
    /// \brief emmited if updates available
    void signalUpdatesFound();

    /// \brief signalFinished
    /// \brief emmited if update process finished
    void signalFinished();

  public slots:
    /// slots to install Updates
    void slotBinaryUpdateRequested(Update binaryUpdate);
    void slotBaseDiskUpdateRequested(Update baseDiskUpdate);
    void slotConfigUpdateRequested(Update configUpdate);

  private slots:

    // after the InteraktiveUpdateWidget is destroyed set the pointer to NULL
    void slotInteraktiveUpdateWidgetDestroyed(){ ptrInteraktiveUpdateWidget_=NULL; }

    // show the check for Updates Information
    void slotCheckUpdateFinished();

    // show the Updates
    void slotShowBinaryUpdate();
    void slotShowConfigUpdate();
    void slotShowBaseDiskUpdate();

    // Updates downloaded
    void slotBinaryUpdateDownloadFinished();
    void slotConfigUpdateDownloadFinished();
    void slotBaseDiskUpdateDownloadFinished();

    void slotEmitSignalFinished() {emit signalFinished();}

  private:
    WidgetInteraktiveUpdate* ptrInteraktiveUpdateWidget_;
    CheckUpdate checkUpdate_;
    QUrl appcastUrl_;
    PmVersion currentBaseDiskVersion_;
    PmVersion currentBinaryVersion_;
    PmVersion currentConfigVersion_;
    bool baseDiskUpdateRequired_;
    bool vmMaskRegenerationNecessary_;
    bool interactive_;
    SystemConfig* ptrSystemConfig_;
    VerifiedDownload* ptrVerifiedDownload_;
};


/// helper functions




#endif // UPDATEMANAGER_H
