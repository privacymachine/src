#ifndef CHECKUPDATE_H
#define CHECKUPDATE_H

#define ILOG(message) {  qDebug() << qPrintable(message); }
#define ILOG_SENSITIVE(message) {if (globalSensitiveLoggingEnabed) { qDebug() << qPrintable(message); } }
#define IWARN(message) { qWarning() << qPrintable(message); }
#define IERR(message) { qCritical() << qPrintable(message); }


#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "XmlUpdateParser.h"
#include "PmVersion.h"

// struct that holds all infomation for an Update
struct Update
{
    enum UpdateType
    {
      NoUpdate = -1,
      BaseDisk = 0,
      Binary,
      Config,
    } Type;
    QUrl Url;
    QString CheckSum;
    PmVersion Version;
    QString Title;
    QString Description;
};

class CheckUpdate : public QObject
{
    Q_OBJECT
  public:
    /// ErrorCodes
    enum CheckUpdateError
    {
      NoError = 0,
      Aborted,
      NetworkError,
      ParsingError
    };


    explicit CheckUpdate(QObject *parent = 0);

    /// returns true if a valid configuration is avaiable
    bool isReady();


    // setter:
    void setUrl(QUrl feedURL) {url_ = feedURL;}
    void setUrl(QString feedURL){setUrl(QUrl(feedURL));}

    void setCurrentBaseDiskVersion(PmVersion currentBaseDiskVersion) {currentBaseDiskVersion_ = currentBaseDiskVersion;}
    void setCurrentBinaryVersion(PmVersion currentBinaryVersion) {currentBinaryVersion_ = currentBinaryVersion;}
    void setCurrentConfigVersion(PmVersion currentConfigVersion) {currentConfigVersion_ = currentConfigVersion;}

    // getter:
    QUrl getUrl() {return url_;}
    bool isStarted() {return started_;}
    CheckUpdateError getError() {return error_;}
    QList<Update> getAvaiableBaseDiskUpdates() {return updateListBaseDisk_;}
    QList<Update> getAvaiableBinaryUpdates() {return updateListBinary_;}
    QList<Update> getAvaiableConfigUpdates() {return updateListConfig_;}

    Update getLatestBinaryUpdate();
    Update getLatestBaseDiskUpdate();
    Update getLatestConfigUpdate();
    // public variables for developing only

    //QByteArray raw_output;

  signals:
    void finished();
    void signalUpdateFound(Update avaiableUpdate);


  public slots:
    bool start();

  private slots:
    void slotDownloadFinished();

  private:
    // private variables:
    QUrl url_;
    bool started_;
    CheckUpdateError error_;
    QNetworkAccessManager *ptrNAM_;
    QNetworkReply *ptrNetReply_;
    XmlUpdateParser xmlUpdateParser_;
    QList<Update> updateListBaseDisk_;
    QList<Update> updateListBinary_;
    QList<Update> updateListConfig_;
    PmVersion currentBaseDiskVersion_;
    PmVersion currentBinaryVersion_;
    PmVersion currentConfigVersion_;
    // private functions:

    /// \brief findUpdates
    /// \brief compares the versions in xmlUpdateParser_ and emits signalUpdateFound
    void findBinaryUpdates();
    void findBaseDiskUpdates();
    void findConfigUpdates();
};

#endif // CHECKUPDATE_H
