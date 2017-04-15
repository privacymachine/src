#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
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
    /// ErrorCodes
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

    /// returns true if a valid configuration is avaiable
    bool isReady();


    // setter:
    void setUrl(QUrl url) {url_=url;}
    void setUrl(QString url) {url_=QUrl(url);}
    void setSHA(QString sha) {shaSum_ = sha;}
    void setSHA(QByteArray sha) {shaSum_ = QString(sha);}
    void setDownloadTargetDir(QDir downloadTargetDir) {targetDir_ = downloadTargetDir;}
    void setDownloadTargetDir(QString downloadTargetDir) {targetDir_ = QDir(downloadTargetDir);}
    void setHashAlgo(QCryptographicHash::Algorithm hashAlgo) {hashAlgo_ = hashAlgo;}

    // getter:
    VerifiedDownloadError getError() {return error_;}
    QUrl getUrl() {return url_;}
    QDir getTargetDir() {return targetDir_;}
    QString getFilePath() {isReady(); return filePath_;}
    QString getSHA() {return shaSum_;}
    /// true while download and verify in progress
    bool isStarted() {return started_;}

  public slots:
    /// abort download
    void abort();
    /// starts download and verifing, emits signal finished
    bool start();

  signals:
    void downloadProgress(qint64 parSize, qint64 parTotalSize);
    void finished();

  private slots:
    void slotReemitDownloadProgress(qint64 down, qint64 total);
    void slotDownloadFinished();
    void slotFinished();

  private:
    QString shaSum_;
    QUrl url_;
    QString filePath_;
    QDir targetDir_;
    QNetworkAccessManager *ptrNAM_;
    QNetworkReply *ptrNetReply_;
    VerifiedDownloadError error_;
    bool started_;
    QCryptographicHash::Algorithm hashAlgo_;
    qint64 progressBarMax_;
};

#endif // DOWNLOADER_H
