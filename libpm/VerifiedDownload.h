#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QString>
#include <QCryptographicHash>

class VerifiedDownload : public QObject
{
    Q_OBJECT
  public:

    /// \brief The VerifiedDownloadError enum holdes the different error states of \class VerifiedDownload
    enum VerifiedDownloadError{
      NoError = 0,
      Aborted,
      NetworkError,
      FileReadError,
      FileWriteError,
      IntegrityError,
    };

    explicit VerifiedDownload(QObject *parent = 0);

    ~VerifiedDownload();

    /// \brief isReady
    /// \return true if a valid configuration is avaiable
    bool isReady();

    /// setter:

    /// \brief setUrl
    /// \param url
    void setUrl(QUrl url) {url_=url;}
    void setUrl(QString url) {url_=QUrl(url);}

    /// \brief setCheckSum
    /// \param checkSum
    void setCheckSum(QString checkSum) {checkSum_ = checkSum;}
    void setCheckSum(QByteArray checkSum) {checkSum_ = QString(checkSum);}

    /// \brief setDownloadTargetDir
    /// \param downloadTargetDir
    void setDownloadTargetDir(QDir downloadTargetDir) {targetDir_ = downloadTargetDir;}
    void setDownloadTargetDir(QString downloadTargetDir) {targetDir_ = QDir(downloadTargetDir);}

    /// \brief setHashAlgorithm
    /// \param hashAlgorithm
    void setHashAlgorithm(QCryptographicHash::Algorithm hashAlgorithm) {hashAlgorithm_ = hashAlgorithm;}

    /// getter:

    /// \brief getError
    /// \return VerifiedDownload::VerifiedDownloadError
    VerifiedDownloadError getError() {return error_;}

    /// \brief getErrorString
    /// \return a human-readable description of the last device error that occurred.
    QString getErrorString() {return errorStr_;}

    /// \brief getUrl
    /// \return QUrl
    QUrl getUrl() {return url_;}

    /// \brief getTargetDir
    /// \return QDir
    QDir getTargetDir() {return targetDir_;}

    /// \brief getFilePath
    /// \return QString
    QString getFilePath() {isReady(); return filePath_;}

    /// \brief getCheckSum
    /// \return QString
    QString getCheckSum() {return checkSum_;}

    /// \brief isStarted
    /// \return true while download and verify in progress
    bool isStarted() {return started_;}


  public slots:

    /// \brief abort
    /// \brief aborts download
    void abort();

    /// \brief start
    /// \brief starts download and verifing, emits signal finished
    /// \return true if download was successfully started
    bool start();

  signals:
    /// \brief downloadProgress
    /// \brief signal that emits the current download and verifying status
    /// \param parSize current amount of work done
    /// \param parTotalSize total amount of work to do
    void downloadProgress(qint64 parSize, qint64 parTotalSize);

    /// \brief finished
    /// \brief signal that is emited when VerifiedDownload is finished
    void finished();

  private slots:
    /// \brief slotReemitDownloadProgress
    /// \brief rememits the download process but doubles the total value to indicate downloading is only the first step
    /// \param down current amount of downloaded data
    /// \param total amount of data to download
    void slotReemitDownloadProgress(qint64 down, qint64 total);

    /// \brief slotDownloadFinished
    /// \brief does error handling of download and does the write down as well as the verifying process
    void slotDownloadFinished();

    /// \brief slotError
    /// \brief called when an error occours
    /// \param parErrorCode contains the error
    void slotError(QNetworkReply::NetworkError parErrorCode);

    /// \brief slotSslError
    /// \brief called when an ssl errors occours
    /// \param parSslErrors list of errors
    void slotSslErrors(const QList<QSslError> &parSslErrors);

    /// \brief slotFinished
    /// \brief does cleanup of ptrNetReply
    void slotFinished();

  private:
    // inititialized in constructor
    QNetworkAccessManager *ptrNam_;

    QString filePath_ = "";
    QString checkSum_ = "_invalid checksum_";
    QString errorStr_ = "";
    QDir targetDir_ = QDir("/not_existing");
    QUrl url_ = QUrl("_invalid URL_", QUrl::StrictMode);
    QNetworkReply *ptrNetReply_ = nullptr;
    VerifiedDownloadError error_ = NoError;
    bool started_ = false;
    qint64 progressBarMax_ = 0;
    // this means hashAlgo_=0 and we don't want to use Md4 nor Md5 (1)
    QCryptographicHash::Algorithm hashAlgorithm_ = QCryptographicHash::Md4;

};

#endif // DOWNLOADER_H
