#include "VerifiedDownload.h"

#include <QNetworkRequest>


#include "utils.h"

VerifiedDownload::VerifiedDownload(QObject *parent) :
  QObject(parent)
{
  // this means hashAlgo_=0 and we don't want to use either Md4 or Md5 (1)
  hashAlgo_=QCryptographicHash::Md4;
  ptrNetReply_ = NULL;
  ptrNAM_ = new QNetworkAccessManager(this);
  connect(this, SIGNAL(finished()), this, SLOT(slotFinished()));
  filePath_="";
}

VerifiedDownload::~VerifiedDownload()
{
  if(ptrNAM_)
    delete ptrNAM_;
  if(ptrNetReply_)
    delete ptrNAM_;
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

  if(hashAlgo_ < QCryptographicHash::Sha1)
  {
    // neider Md4 nor Md5 should be used anymore!
    IWARN("No (secure) hash algrithm is choosen.");
    initialized = false;
  }
  foreach (QChar c, shaSum_)
  {
    if(!c.isDigit() && !c.isLetter())
    {
      IWARN("Hash '"+shaSum_+"' is not base64.");
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
    ptrNetReply_->deleteLater();
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

  QNetworkRequest request( url_ );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  ptrNetReply_ = ptrNAM_->get(request);

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

  return true;
}

void VerifiedDownload::abort()
{
  if(started_)
  {
    ptrNetReply_->abort();
    error_ = Aborted;
    emit finished();
  }
}

void VerifiedDownload::slotDownloadFinished()
{
  if (error_ != NoError)
  {
    if(error_ != Aborted)
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

  if(error_ == Aborted) return;

  QCryptographicHash hash( hashAlgo_ );
  if( file.open( QIODevice::ReadOnly ) && hash.addData( &file ) )
  {
    if( hash.result().toHex() != shaSum_ )
    {
      IERR("Error: Failed to verify check sum of file '"+filePath_+"': "+hash.result().toHex() +
           " does not match check sum, which is "+shaSum_+".");
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
  emit finished();
}
