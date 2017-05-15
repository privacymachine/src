#ifndef CHECKUPDATE_H
#define CHECKUPDATE_H

#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "XmlUpdateParser.h"
#include "PmVersion.h"

/// \brief The Update struct
/// \brief: struct that holds all infomation for an Update
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

    /// \brief compare
    /// \brief: compare helper function used by std::sort based on PmVersion::operator >
    /// \param a [in]: Update
    /// \param b [in]: Update
    /// \return true if a.Version > b.Version
    static bool compare (Update a, Update b)
    {
      return a.Version > b.Version;
    }
};


/// \brief The CheckUpdate class
/// \brief downloads a appcast and finds the installabble updates
class CheckUpdate : public QObject
{
    Q_OBJECT
  public:

    /// \brief The CheckUpdateError enum holds the different error states of \class CheckUpdate
    enum CheckUpdateError
    {
      NoError = 0,
      Aborted,
      NetworkError,
      ParsingError
    };

    explicit CheckUpdate(QObject *parent = 0);

    /// \brief isReady
    /// \return true if a valid configuration is avaiable
    bool isReady();

    /// setter:

    /// \brief setUrl
    /// \param feedURL
    void setUrl(QUrl feedURL) {url_ = feedURL;}
    void setUrl(QString feedURL){setUrl(QUrl(feedURL));}

    /// \brief setCurrentBaseDiskVersion
    /// \param currentBaseDiskVersion
    void setCurrentBaseDiskVersion(PmVersion currentBaseDiskVersion) {currentBaseDiskVersion_ = currentBaseDiskVersion;}

    /// \brief setCurrentBinaryVersion
    /// \param currentBinaryVersion
    void setCurrentBinaryVersion(PmVersion currentBinaryVersion) {currentBinaryVersion_ = currentBinaryVersion;}

    /// \brief setCurrentConfigVersion
    /// \param currentConfigVersion
    void setCurrentConfigVersion(PmVersion currentConfigVersion) {currentConfigVersion_ = currentConfigVersion;}

    /// getter:

    /// \brief getUrl
    /// \return QUrl
    QUrl getUrl() {return url_;}

    /// \brief isStarted
    /// \return true if download and checking process started
    bool isStarted() {return started_;}

    /// \brief getError
    /// \return CheckUpdate::CheckUpdateError
    CheckUpdateError getError() {return error_;}

    /// \brief getErrorString
    /// \return a error description
    QString getErrorString() {return errorStr_;}

    /// \brief getavailableBaseDiskUpdates
    /// \return a list of available BaseDisk updates
    QList<Update> getAvailableBaseDiskUpdates() {return updateListBaseDisk_;}

    /// \brief getavailableBinaryUpdates
    /// \return a list of available Binary updates
    QList<Update> getAvailableBinaryUpdates() {return updateListBinary_;}

    /// \brief getavailableConfigUpdates
    /// \return a list of available Config updates
    QList<Update> getAvailableConfigUpdates() {return updateListConfig_;}

    /// \brief start
    /// \brief starts the download of the appcast
    /// \return true if the download of the appcast started successfully
    bool start();

  signals:

    /// \brief finished
    /// \brief signal which is emitted if CheckUpdate is finished
    void finished();

  private slots:

    /// \brief slotDownloadFinished
    /// \brief does the error handling of the download, handles the XmlUpdateParser calls the functions to find the installable updates
    void slotDownloadFinished();


  private:

    /// private variables:

    QUrl url_;
    bool started_;
    CheckUpdateError error_;
    QString errorStr_;
    QNetworkAccessManager *ptrNAM_;
    QNetworkReply *ptrNetReply_;
    XmlUpdateParser xmlUpdateParser_;
    QList<Update> updateListBaseDisk_;
    QList<Update> updateListBinary_;
    QList<Update> updateListConfig_;
    PmVersion currentBaseDiskVersion_;
    PmVersion currentBinaryVersion_;
    PmVersion currentConfigVersion_;


    /// private functions:

    /// \brief findBinaryUpdates
    /// \brief compares the versions of all Binary updates with the current Binary version and finds installable updates
    void findBinaryUpdates();

    /// \brief findBaseDiskUpdates
    /// \brief compares the versions of all BaseDisk updates with the current BaseDisk version and finds installable updates
    void findBaseDiskUpdates();

    /// \brief findConfigUpdates
    /// \brief compares the versions of all Config updates with the current Config version and finds installable updates
    void findConfigUpdates();
};

#endif // CHECKUPDATE_H
