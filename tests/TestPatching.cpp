#include "TestPatching.h"

const unsigned int HTTP_TIMEOUT = 100U;

bool TestPatching::checksumsMatch( const QString & filePath1, const QString & filePath2 )
{
  bool success = false;
  QFile file1( filePath1 );
  QFile file2( filePath2 );
  
  if( file1.open( QIODevice::ReadOnly ) && file2.open( QIODevice::ReadOnly ) )
  {
      QCryptographicHash hash1( QCryptographicHash::Sha256 );
      QCryptographicHash hash2( QCryptographicHash::Sha256 );
      if( hash1.addData( &file1 ) && hash2.addData( &file2 ) )
      {
        success = hash1.result() == hash2.result();
        if( !success )
        {
          qDebug() << "Check sums of files '" << filePath1 << "' (" << hash1.result().toHex() << ") and '" 
            << filePath2 << "' (" << hash1.result().toHex() << ") differ from each other.";
          
        }
        
      }
      else
      {
        qWarning() << "Error: Failed to read input file(s).";
        
      }
      
  }      
  else
  {
    qWarning() << "Error: Failed to open input file(s) for reading..";

  }
  
  return success;

}


bool TestPatching::download( QString url, QString localFileName, ulong milliseconds )
{
  // from
  // http://stackoverflow.com/questions/13207493/qnetworkreply-and-qnetworkaccessmanager-timeout-in-http-request
  QTimer timer;    
  timer.setSingleShot(true);

  // Start Release Notes Download
  QNetworkAccessManager accessManager;
  QNetworkReply* reply = accessManager.get( QNetworkRequest( url ) );
  QEventLoop loop;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  timer.start(milliseconds); 
  loop.exec();

  if( timer.isActive() )
  {
    if (reply->error() != QNetworkReply::NoError)
    {
      qDebug() << "Unable to download '" + url + "': " << reply->errorString();
      return false;

    }

    QFile file( localFileName );
    if( !file.open(QIODevice::WriteOnly) 
      || file.write(reply->readAll()) == -1 )
    {
      qDebug() << "Failed to access '" + localFileName + "' for writing.";
      file.close();
      return false;

    }

    file.close();

  }
  else
  {
    // timeout
    reply->abort();    
    disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    qDebug()<<"Timeout after " << milliseconds << "ms while downloading release notes: " << reply->errorString();
    return false;

  }

  return true;

}


void TestPatching::testFalsifyObsolesce()
{

}

void TestPatching::testVerifyUpdate_7z()
{
  QApplication::setApplicationName("TestPatching");
  QApplication::setOrganizationName("PrivacyMachine");
  QApplication::setOrganizationDomain("privacymachine.eu");

  unsigned int referenceMajorVersion = 1;
  int componentMajorVersion = 1;
  QString referenceVersionString =
      "1.0." + QString::number( referenceMajorVersion ) + ".0";
  QString componentVersionString =
      "1.0." + QString::number( componentMajorVersion ) + ".0";
  qDebug() << "Checking update from version " << componentVersionString << " to version "
           << referenceVersionString << ".";

  QString referenceBaseDiskFilePrefix =
      "base-disk_" + QString::number( referenceMajorVersion );
  QString referenceDirectoryUrl = "file://localhost/media/pmTests/TestPatching/";

  // download reference version
  QString referenceZipFileUrl = referenceDirectoryUrl + "downloads/" + referenceBaseDiskFilePrefix + ".7z";
  QString referenceZipFileName = referenceBaseDiskFilePrefix + ".reference.7z";
  QString referenceFileName = referenceBaseDiskFilePrefix + ".reference.flat.vmdk";
  QString componentZipFileName = referenceBaseDiskFilePrefix + ".7z";
  QString componentFileName = referenceBaseDiskFilePrefix + ".flat.vmdk";

  qDebug() << "Downloading reference base disk zip file " << componentZipFileName << ".";
  QVERIFY( download( referenceZipFileUrl, referenceZipFileName ) );

  qDebug() << "Extracting and renaiming base disk to obtain " << referenceFileName << ".";
  QVERIFY( FvUpdater::sharedUpdater()->slotUnzipUpdate_7z( referenceZipFileName, "./", "" ) );
  QVERIFY( QFile( componentFileName ).rename( referenceFileName ) );

  // set latest reference version inside app cast
  // set current app version, app cast url
  FvUpdater::sharedUpdater()->SetComponentVersion( ComponentVersion( 1, 0, componentMajorVersion, 0 ) );
  QString referenceAppCastUrl = referenceDirectoryUrl + "appcast_" + referenceBaseDiskFilePrefix + ".xml";
  FvUpdater::sharedUpdater()->SetFeedURL( referenceAppCastUrl );

  if( referenceMajorVersion > componentMajorVersion )
  {
    QVERIFY( findUpdate() );
    QVERIFY( installUpdate() );
    QVERIFY( checksumsMatch( referenceFileName, componentFileName ) );

  }
  // If we attempt to update to a version equal to or lower than the current component version, be sure
  // that we refuse to update.
  else
  {
    QVERIFY( !findUpdate() );
    QVERIFY( !installUpdate() );

  }

  QFile::remove( referenceZipFileName );
  QFile::remove( referenceFileName );
  QFile::remove( componentZipFileName );
  QFile::remove( componentFileName );
  QFile::remove( "*delta*.rdiff");
}

void TestPatching::testVerifyUpdate()
{
  QApplication::setApplicationName("TestPatching");
  QApplication::setOrganizationName("PrivacyMachine");
  QApplication::setOrganizationDomain("privacymachine.eu");

  /*
  QuaZip zipFile( "quazip_base-disk_1.zip" );
  zipFile.setFileNameCodec("IBM866");
  if( zipFile.open( QuaZip::mdCreate ) )
  {
    addFileToZip(
      &zipFile, 
      "/media/pmTests/TestPatching/downloads/base-disk_1.flat.vmdk", 
      "base-disk_1.vmdk", 
      "");

    zipFile.close();
    if (zipFile.getZipError() != 0)
    {
      IERR("error on zipfile.close(): " + QString::number(zipFile.getZipError()));
    }

  }
*/

  for( unsigned int referenceMajorVersion = 0; referenceMajorVersion <= 9; ++referenceMajorVersion )
  {
    for( unsigned int componentMajorVersion = 0; componentMajorVersion <= 9; ++componentMajorVersion )
    {
      QString referenceVersionString = 
        "1.0." + QString::number( referenceMajorVersion ) + ".0";
      QString componentVersionString = 
        "1.0." + QString::number( componentMajorVersion ) + ".0";
      qDebug() << "Checking update from version " << componentVersionString << " to version "
        << referenceVersionString << ".";

      QString referenceBaseDiskFilePrefix = 
        "base-disk_" + QString::number( referenceMajorVersion );
      
      #ifndef PM_WINDOWS
      QString referenceDirectoryUrl = "file://localhost/media/pmTests/TestPatching/";
      #else
      QString referenceDirectoryUrl = "file:///c:/cygwin64/home/worker/jenkins_root/workspace/PrivacyMachine_PM-Win7/tests/TestPatching/";
      #endif

      QString largeSuffix="";
      if (referenceMajorVersion == 1)
        largeSuffix = "_large";

      // download reference version
      QString referenceZipFileUrl = referenceDirectoryUrl + "downloads/" + referenceBaseDiskFilePrefix + largeSuffix + ".zip";
      QString referenceZipFileName = referenceBaseDiskFilePrefix + ".reference.zip";
      QString referenceFileName = referenceBaseDiskFilePrefix + ".reference.flat.vmdk";
      QString componentZipFileName = referenceBaseDiskFilePrefix + ".zip";
      QString componentFileName = referenceBaseDiskFilePrefix + ".flat.vmdk";
      
      qDebug() << "Downloading reference base disk zip file " << componentZipFileName << ".";
      QVERIFY( download( referenceZipFileUrl, referenceZipFileName ) );
      
      qDebug() << "Extracting and renaiming base disk to obtain " << referenceFileName << ".";
      QVERIFY( FvUpdater::sharedUpdater()->slotUnzipUpdate( referenceZipFileName, "./", "" ) );
      QVERIFY( QFile( componentFileName ).rename( referenceFileName ) );

      // set latest reference version inside app cast
      // set current app version, app cast url
      FvUpdater::sharedUpdater()->SetComponentVersion( ComponentVersion( 1, 0, componentMajorVersion, 0 ) );
      QString referenceAppCastUrl = referenceDirectoryUrl + "appcast_" + referenceBaseDiskFilePrefix + ".xml";
      FvUpdater::sharedUpdater()->SetFeedURL( referenceAppCastUrl );

      if( referenceMajorVersion > componentMajorVersion )
      {
        QVERIFY( findUpdate() );
        QVERIFY( installUpdate() );
        QVERIFY( checksumsMatch( referenceFileName, componentFileName ) );

      }
      // If we attempt to update to a version equal to or lower than the current component version, be sure
      // that we refuse to update.
      else
      {
        QVERIFY( !findUpdate() );
        QVERIFY( !installUpdate() );

      }

      QFile::remove( referenceZipFileName );
      QFile::remove( referenceFileName );
      QFile::remove( componentZipFileName );
      QFile::remove( componentFileName );
      QFile::remove( "*delta*.rdiff");

    }
  }
}


bool TestPatching::findUpdate()
{
  bool success = true;
  QTimer timer;    
  timer.setSingleShot(true);

  QEventLoop loop;
  connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
  connect( FvUpdater::sharedUpdater(), SIGNAL( signalFoundUpdate() ), &loop, SLOT( quit() ) );
  timer.start( HTTP_TIMEOUT ); 
  success = success && FvUpdater::sharedUpdater()->slotCheckForUpdates( Verbosity::Silent );
  loop.exec();

  if( !timer.isActive() )
  {
    disconnect( FvUpdater::sharedUpdater(), SIGNAL( signalFoundUpdate() ), &loop, SLOT( quit() ) );
    success = false;

  }

  return success;

}


bool TestPatching::installUpdate()
{
  bool success = true;
  updateInstalled_ = false;
  QTimer timer;    
  timer.setSingleShot(true);
  
  QEventLoop loop;
  connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
  connect( FvUpdater::sharedUpdater(), SIGNAL( signalUpdateInstalled() ), &loop, SLOT( quit() ) );
  connect( FvUpdater::sharedUpdater(), SIGNAL( signalUpdateInstalled() ), this, SLOT( slotUpdateInstalled() ) );
  timer.start( HTTP_TIMEOUT ); 
  FvUpdater::sharedUpdater()->slotTriggerUpdate();
  loop.exec();

  // updateInstalled might have been emitted before calling loop.exec(), so we also check updateInstalled_.
  if( !timer.isActive() && !updateInstalled_ )
  {
    disconnect( FvUpdater::sharedUpdater(), SIGNAL( signalUpdateInstalled() ), &loop, SLOT( quit() ) );
    success = false;

  }

  return success;

}

bool TestPatching::addFileToZip(
  QuaZip* zipFile, QString inputFilePath, QString resultingFileName, QString resultingRelativeAddtitionalPath )
{
  QFileInfo fileInfo(inputFilePath);
  if (!fileInfo.isFile())
  {
    // It's ok to add non-existing files to this function (because it makes the caller easier readable)
    ILOG("file " + inputFilePath + " does not exist.");
    return true;
  }

  QFile inFile;
  QuaZipFile outFile(zipFile);

  inFile.setFileName(fileInfo.filePath());
  if (!inFile.open(QIODevice::ReadOnly))
  {
    IERR("error from inFile.open(): " + inFile.errorString());
    return false;
  }

  if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(resultingRelativeAddtitionalPath + resultingFileName, inputFilePath)))
  {
    IERR("error adding file: " + inputFilePath + ": " + outFile.getZipError());
    return false;
  }

  // copy the content
  char c;
  while (inFile.getChar(&c) && outFile.putChar(c));

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error calling putChar(): " + outFile.getZipError());
    return false;
  }

  outFile.close();

  if (outFile.getZipError() != UNZ_OK)
  {
    IERR("error from close(): " + outFile.getZipError());
    return false;
  }

  return true;
}

QTEST_MAIN(TestPatching)
