#include "CheckUpdate.h"
#include <QNetworkRequest>
#include <QApplication>
#include "PmVersion.h"


CheckUpdate::CheckUpdate(QObject *parent) :
  QObject(parent)
{
  started_ = false;
  ptrNetReply_ = NULL;
  ptrNAM_ = new QNetworkAccessManager(this);
  error_= NoError;
}

bool CheckUpdate::isReady()
{
  bool initialized = true;
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
  return initialized;
}

// start the download of the appcast
bool CheckUpdate::start()
{
  ILOG("CheckUpdate: started");
  if(!isReady())
  {
    IERR("CheckUpdate: not (properly) initialized");
    return false;
  }
  if(started_)
  {
    IERR("CheckUpdate: already running");
    return false;
  }
  started_ = true;
  error_= NoError;
  updateListBaseDisk_.clear();
  updateListBinary_.clear();
  updateListConfig_.clear();

  ILOG("CheckUpdate: Start Download of "+url_.toDisplayString());
  QNetworkRequest request( url_ );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  ptrNetReply_ = ptrNAM_->get(request);

  if( ptrNetReply_->error() != QNetworkReply::NoError)
  {
    IERR("CheckUpdate: Could not download "+url_.toDisplayString()+
         " because of QNetworkReply::NetworkError "+QString::number(ptrNetReply_->error()));
    error_ = NetworkError;
    emit finished();
    return false;
  }
  connect(ptrNetReply_,
          SIGNAL(finished()),
          this,
          SLOT(slotDownloadFinished()));
}

// after downloading appcast check parse xml and compare versions
void CheckUpdate::slotDownloadFinished()
{
  if( ptrNetReply_->error() != QNetworkReply::NoError )
  {
    error_ = NetworkError;
    IERR("CheckUpdate: Could not download "+url_.toDisplayString()+
         " because of QNetworkReply::NetworkError "+QString::number(ptrNetReply_->error()));
    emit finished();
    return;
  }
  if (error_ != NoError)
  {
    if(error_ != Aborted)
      emit finished();
    return;
  }


  // parse XML
  if(!xmlUpdateParser_.parse(ptrNetReply_->readAll()))
  {
    IERR("CheckUpdate: Error at parsing appcast XML.");
    error_=ParsingError;
  }
  if (error_ == NoError) findBinaryUpdates();
  if (error_ == NoError) findBaseDiskUpdates();
  emit finished();
}

void CheckUpdate::findBinaryUpdates()
{

  /// @todo: question @bernhard: is "auto binaryList = xmlUpdateParser_.getBinaryVersionList()" ok?
  QList<XmlUpdateParser::UpdateInfoBinary> binaryList = xmlUpdateParser_.getBinaryVersionList();


  foreach (XmlUpdateParser::UpdateInfoBinary binary, binaryList)
  {
    if( PmVersion::greaterInRespectTo(binary.Version, currentBinaryVersion_, PmVersion::Major ) )
    {
      Update binaryUpdate;
      binaryUpdate.Type = Update::Binary;
      binaryUpdate.Version = binary.Version;
      binaryUpdate.Description = binary.Description;
      binaryUpdate.Title = binary.Title;

      //TODO: @Bernhard im deployment in setings aufnehmen
      QString OS;
      #ifndef PM_WINDOWS
        OS = "jessie64";
      #else
        OS = "win64";
      #endif

      foreach( XmlUpdateParser::CheckSumListBinary entry, binary.CheckSums )
      {
        if (entry.Os == OS)
        {
          binaryUpdate.CheckSum = entry.CheckSum;
          foreach (QChar c, binaryUpdate.CheckSum)
          {
            if(!c.isDigit() && !c.isLetter())
            {
              IERR("CheckUpdate: Hash '"+binaryUpdate.CheckSum+"' is not base64.");
              error_=ParsingError;
              break;
            }
          }

          binaryUpdate.Url = QUrl(entry.Url);
          if( (binaryUpdate.Url.isLocalFile()) || (!binaryUpdate.Url.isValid()) )
          {
            IERR("CheckUpdate: Binary Update URL <"+binaryUpdate.Url.toString()+"> is not a valid URL");
            error_=ParsingError;
            break;
          }
        }
      }
      if (error_ == NoError)
      {
        updateListBinary_.push_back(binaryUpdate);
        /// @todo: olaf remove soon
        emit signalUpdateFound(binaryUpdate);
      }
      else break;
    }
  }
}


void CheckUpdate::findBaseDiskUpdates()
{
  QList<XmlUpdateParser::UpdateInfoBaseDisk> baseDiskList = xmlUpdateParser_.getBaseDiskVersionList();

  foreach (XmlUpdateParser::UpdateInfoBaseDisk baseDisk, baseDiskList)
  {
    if( PmVersion::greaterInRespectTo(baseDisk.Version, currentBaseDiskVersion_, PmVersion::ComponentMajor) || currentBaseDiskVersion_.isZero() )
    {
      Update baseDiskUpdate;
      baseDiskUpdate.Type = Update::BaseDisk;
      baseDiskUpdate.Version = baseDisk.Version;
      baseDiskUpdate.Description = baseDisk.Description;
      baseDiskUpdate.Title = baseDisk.Title;


      foreach( XmlUpdateParser::CheckSumListBaseDisk entry, baseDisk.CheckSums )
      {
        if( (entry.ComponentMajorUp == currentBaseDiskVersion_.getComponentMajor()) ||
            (entry.ComponentMajorUp == entry.Version.getComponentMajor()) )
        {
          baseDiskUpdate.CheckSum = entry.CheckSum;
          foreach (QChar c, baseDiskUpdate.CheckSum)
          {
            if(!c.isDigit() && !c.isLetter())
            {
              IERR("CheckUpdate: Hash '"+baseDiskUpdate.CheckSum+"' is not base64.");
              error_=ParsingError;
              break;
            }
          }

          baseDiskUpdate.Url = QUrl(entry.Url);
          if( (baseDiskUpdate.Url.isLocalFile()) || (!baseDiskUpdate.Url.isValid()) )
          {
            IERR("CheckUpdate: Binary Update URL <"+baseDiskUpdate.Url.toString()+"> is not a valid URL");
            error_=ParsingError;
            break;
          }
        }

      }
      if (error_ == NoError)
      {
        updateListBaseDisk_.push_back(baseDiskUpdate);
        /// @todo: olaf remove soon
        emit signalUpdateFound(baseDiskUpdate);
      }
      else break;
    }
  }
}

void CheckUpdate::findConfigUpdates()
{
  /// @todo: implement me
}

Update CheckUpdate::getLatestBinaryUpdate()
{
  Update latestUpdate;
  if(updateListBinary_.size() < 1)
  {
    latestUpdate.Type=Update::NoUpdate;
    return latestUpdate;
  }
  else
  {
    foreach (Update update, updateListBinary_)
    {
      if (update.Version > latestUpdate.Version) latestUpdate=update;
    }
  }
  return latestUpdate;
}

Update CheckUpdate::getLatestBaseDiskUpdate()
{
  Update latestUpdate;
  if(updateListBaseDisk_.size() < 1)
  {
    latestUpdate.Type=Update::NoUpdate;
    return latestUpdate;
  }
  else
  {
    foreach (Update update, updateListBaseDisk_)
    {
      if (update.Version > latestUpdate.Version) latestUpdate=update;
    }
  }
  return latestUpdate;
}

Update CheckUpdate::getLatestConfigUpdate()
{
  Update latestUpdate;
  if(updateListConfig_.size() < 1)
  {
    latestUpdate.Type=Update::NoUpdate;
    return latestUpdate;
  }
  else
  {
    foreach (Update update, updateListConfig_)
    {
      if (update.Version > latestUpdate.Version) latestUpdate=update;
    }
  }
  return latestUpdate;
}
