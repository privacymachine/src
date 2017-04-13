#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QUrl>
#include "PmVersion.h"
#include "WidgetInteraktiveUpdate.h"
#include "CheckUpdate.h"

class UpdateManager : public QObject
{
    Q_OBJECT
  public:

    explicit UpdateManager(QObject *parent = 0);
    virtual ~UpdateManager();

    /// create a Widget to Interact with the user
    WidgetInteraktiveUpdate* getUpdateWidget(QWidget *parParent = NULL);


    /// Download and parse Appcast
    bool findUpdates();

    /// getter and setter
    void setAppcastUrl(QUrl appcastUrl) {appcastUrl_=appcastUrl;}
    void setBaseDiskUpdateRequired(bool required) {baseDiskUpdateRequired_=required;}
    void setCurrentBaseDiskVersion(PmVersion currentBaseDiskVersion) {currentBaseDiskVersion_ = currentBaseDiskVersion;}
    void setCurrentBinaryVersion(PmVersion currentBinaryVersion) {currentBinaryVersion_ = currentBinaryVersion;}
    void setCurrentConfigVersion(PmVersion currentConfigVersion) {currentConfigVersion_ = currentConfigVersion;}


  signals:
    void signalUpdatesFound();
    void signalFinished();

  public slots:
    /// slots to install Updates
    void slotBinaryUpdateRequested(Update binaryUpdate);
    void slotBaseDiskUpdateRequested(Update baseDiskUpdate);
    void slotConfigUpdateRequested(Update configUpdate);

  private slots:

    // show the check for Updates Information
    void slotCheckUpdateFinished();

    // show the Update selector (RadioButtons)
    void slotShowBinaryUpdate();
    void slotShowConfigUpdate();
    void slotShowBaseDiskUpdate();

    void slotEmitSignalFinished() {emit signalFinished();}

  private:
    WidgetInteraktiveUpdate* ptrInteraktiveUpdateWidget_;
    CheckUpdate checkUpdate_;
    QUrl appcastUrl_;
    PmVersion currentBaseDiskVersion_;
    PmVersion currentBinaryVersion_;
    PmVersion currentConfigVersion_;
    bool baseDiskUpdateRequired_;
    bool interactive_;
};

#endif // UPDATEMANAGER_H
