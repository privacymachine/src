#include "VerifiedDownload.h"
#include "utils.h"

VerifiedDownload::VerifiedDownload(QObject *parent) :
  QObject(parent)
{
  ptrNam_ = new QNetworkAccessManager(this);

  connect(this,
          &VerifiedDownload::finished,
          this,
          &VerifiedDownload::slotFinished);
}

VerifiedDownload::~VerifiedDownload()
{
  if(ptrNam_ != nullptr)
    delete ptrNam_;

  if(ptrNetReply_ != nullptr)
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
  for (QChar c : checkSum_)
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
    ptrNetReply_ = nullptr;
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
          &QNetworkReply::downloadProgress,
          this,
          &VerifiedDownload::slotReemitDownloadProgress);

  connect(ptrNetReply_,
          &QNetworkReply::finished,
          this,
          &VerifiedDownload::slotDownloadFinished);

  connect(ptrNetReply_,
          static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)> (&QNetworkReply::error),
          this,
          &VerifiedDownload::slotError);

  connect(ptrNetReply_,
          static_cast<void (QNetworkReply::*)(const QList<QSslError>&)> (&QNetworkReply::sslErrors),
          this,
          &VerifiedDownload::slotSslErrors);

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
    errorStr_ = "User canceled download.";
    ILOG(errorStr_);
    error_ = Aborted;
  }
  else
  {
    errorStr_ = ptrNetReply_->errorString();
    IERR("Recived network error: " + QString::number(parErrorCode)+"; "+errorStr_);
    error_ = NetworkError;
  }
  started_ = false;
  emit finished();
}

void VerifiedDownload::slotSslErrors(const QList<QSslError> &parSslErrors)
{
  errorStr_ = "Received ssl-network-errors:";
  for (QSslError err : parSslErrors)
  {
    errorStr_ += "\n  " + err.errorString();
  }
  IERR(errorStr_);
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
    slotError(ptrNetReply_->error());
  }

  ILOG("VerifiedDownload: Start writing data to "+filePath_);
  QFile file( filePath_ );
  if( !file.open(QIODevice::WriteOnly)
    || file.write(ptrNetReply_->readAll()) == -1 )
  {
    file.close();
    errorStr_ = "Failed to access '" + filePath_ + "' for writing.";
    IERR(errorStr_);
    error_ = FileWriteError;
    started_ = false;
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
    file.close();
    if( hash.result().toHex() != checkSum_ )
    {
      errorStr_ = "Error: Failed to verify check sum of file '"+filePath_+"': "+hash.result().toHex() +
           " does not match check sum, which is "+checkSum_+".";
      IERR(errorStr_)
      error_ = IntegrityError;
      started_ = false;
      emit finished();
      return;
    }
  }
  else
  {
    file.close();
    errorStr_ = "Error: Failed to access '" + filePath_ + "' for reading.";
    IERR(errorStr_);
    error_ = FileReadError;
    started_ = false;
    emit finished();
    return;
  }
  emit downloadProgress(progressBarMax_, progressBarMax_);
  emit finished();
}

void VerifiedDownload::slotReemitDownloadProgress(qint64 down, qint64 total)
{
  // set progress max to double to indicate integrity check is not done when download is finished
  progressBarMax_ = 2*total;
  emit downloadProgress(down, progressBarMax_);
}
