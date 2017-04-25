#include "VerifiedDownload.h"

#include <QNetworkRequest>


#include "utils.h"

VerifiedDownload::VerifiedDownload(QObject *parent) :
  QObject(parent)
{
  // this means hashAlgo_=0 and we don't want to use either Md4 or Md5 (1)
  hashAlgorithm_=QCryptographicHash::Md4;
  ptrNetReply_ = NULL;
  ptrNam_ = new QNetworkAccessManager(this);
  connect(this, SIGNAL(finished()), this, SLOT(slotFinished()));
  filePath_="";
  error_=NoError;
  started_=false;
}

VerifiedDownload::~VerifiedDownload()
{
  if(ptrNam_ != NULL)
    delete ptrNam_;

  if(ptrNetReply_ != NULL)
    delete ptrNetReply_;
}

bool VerifiedDownload::isReady()
{
  bool initialized = true;
  if(!targetDir_.exists())
  {
    IWARN("Directory '"+targetDir_.absolutePath()+"' does not exist.");
    initialized = false;
  }
  if(!url_.isValid())
  {
    IWARN("URL '"+url_.toString()+" is not valid.'");
    initialized = false;
  }
  if(url_.fileName() == "")
  {
    IWARN("URL '"+url_.toString()+"' must end with a filename, not with '/'.");
    initialized = false;
  }
  // If url and directory are ok set file path
  filePath_ = (initialized) ? targetDir_.absoluteFilePath(url_.fileName()) : "";

  if(hashAlgorithm_ < QCryptographicHash::Sha1)
  {
    // neider Md4 nor Md5 should be used anymore!
    IWARN("No (secure) hash algrithm is choosen.");
    initialized = false;
  }
  foreach (QChar c, checkSum_)
  {
    if(!c.isDigit() && !c.isLetter())
    {
      IWARN("Hash '"+checkSum_+"' is not base64.");
      initialized = false;
      break;
    }
  }
  return initialized;
}

void VerifiedDownload::slotFinished()
{
  started_=false;
  if(ptrNetReply_)
  {
    ptrNetReply_->deleteLater();
    ptrNetReply_ = NULL;
  }
}

bool VerifiedDownload::start()
{
  if(!isReady())
  {
    IERR("VerifiedDownload not (properly) initialized");
    return false;
  }
  if(started_)
  {
    IERR("VerifiedDownload already running");
    return false;
  }
  started_ = true;
  error_=NoError;

  ILOG("VerifiedDownload: Start download of "+url_.toString());
  QNetworkRequest request( url_ );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  ptrNetReply_ = ptrNam_->get(request);

  if( ptrNetReply_->error() != QNetworkReply::NoError)
  {
    IERR("Could not download "+url_.toDisplayString()+
         " because of QNetworkReply::NetworkError "+QString::number(ptrNetReply_->error()));
    error_ = NetworkError;
    emit finished();
    return false;
  }

  connect(ptrNetReply_,
          SIGNAL(downloadProgress(qint64,qint64)),
          this,
          SLOT(slotReemitDownloadProgress(qint64,qint64)));

  connect(ptrNetReply_,
          SIGNAL(finished()),
          this,
          SLOT(slotDownloadFinished()));

  connect(ptrNetReply_,
          SIGNAL(error(QNetworkReply::NetworkError)),
          this,
          SLOT(slotError(QNetworkReply::NetworkError)));

  connect(ptrNetReply_,
          SIGNAL(sslErrors(QList<QSslError>)),
          this,
          SLOT(slotSslErrors(QList<QSslError>)));

  return true;
}

void VerifiedDownload::abort()
{
  if(started_)
  {
    ptrNetReply_->abort();
    error_ = Aborted;
    started_ = false;
    emit finished();
  }
}

void VerifiedDownload::slotError(QNetworkReply::NetworkError parErrorCode)
{
  if (parErrorCode == QNetworkReply::OperationCanceledError)
  {
    ILOG("user canceled download")
  }
  else
  {
    IERR("Received network error: " + QString::number(parErrorCode));
  }
  error_ = Aborted;
  started_ = false;
  emit finished();
}

void VerifiedDownload::slotSslErrors(const QList<QSslError> &parSslErrors)
{
  IERR("Received ssl-network-errors: " );
  foreach (QSslError err, parSslErrors)
  {
    IERR("  " + err.errorString());
  }
  error_ = NetworkError;
  started_ = false;
  emit finished();
}

void VerifiedDownload::slotDownloadFinished()
{
  if (error_ != NoError)
  {
    emit finished();
    return;
  }

  if( ptrNetReply_->error() != QNetworkReply::NoError )
  {
    error_ = NetworkError;
    IERR("Could not download "+url_.toDisplayString()+
         " because of QNetworkReply::NetworkError "+QString::number(ptrNetReply_->error()));
    emit finished();
    return;
  }

  ILOG("VerifiedDownload: Start writing data to "+filePath_);
  QFile file( filePath_ );
  if( !file.open(QIODevice::WriteOnly)
    || file.write(ptrNetReply_->readAll()) == -1 )
  {
    IERR("Failed to access '" + filePath_ + "' for writing.");
    file.close();
    error_ = FileWriteError;
    emit finished();
    return;
  }
  file.close();

  //
  emit downloadProgress(progressBarMax_/2+progressBarMax_/4, progressBarMax_);

  if(error_ == Aborted) return;

  ILOG("VerifiedDownload: Checking the hash sum of "+file.fileName());

  QCryptographicHash hash( hashAlgorithm_ );
  if( file.open( QIODevice::ReadOnly ) && hash.addData( &file ) )
  {
    if( hash.result().toHex() != checkSum_ )
    {
      IERR("Error: Failed to verify check sum of file '"+filePath_+"': "+hash.result().toHex() +
           " does not match check sum, which is "+checkSum_+".");
      file.close();
      error_ = IntegrityError;
      emit finished();
      return;
    }
  }
  else
  {
    IWARN("Error: Failed to access '" + filePath_ + "' for reading.");
    file.close();
    error_ = FileReadError;
    emit finished();
    return;
  }
  file.close();
  emit downloadProgress(progressBarMax_, progressBarMax_);
  emit finished();
}

void VerifiedDownload::slotReemitDownloadProgress(qint64 down, qint64 total)
{
  // set progress max to double to indicate integrity check is not done when download is finished
  progressBarMax_ = 2*total;
  emit downloadProgress(down, progressBarMax_);
}
