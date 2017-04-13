#include "deprecated_fvupdater.h"
#include "utils.h"

FvUpdater* FvUpdater::m_Instance = 0;

FvUpdater* FvUpdater::sharedUpdater()
{
	static QMutex mutex;
	if (! m_Instance) {
		mutex.lock();

		if (! m_Instance) {
			m_Instance = new FvUpdater;
		}

		mutex.unlock();
	}

	return m_Instance;
}

void FvUpdater::drop()
{
	static QMutex mutex;
	mutex.lock();
	delete m_Instance;
	m_Instance = 0;
	mutex.unlock();
}

FvUpdater::FvUpdater()
  : QObject(0), 
  currentComponentVersion_( ComponentVersion( 0, 0, 0, 0 ) )
{
  downloadPath_ = QCoreApplication::applicationDirPath() + "/";
	m_reply = 0;
#ifdef FV_GUI
	m_updaterWindow = 0;
#endif
	m_proposedUpdate = 0;
	m_requiredSslFingerprint = "";
	htAuthUsername = "";
	htAuthPassword = "";
	skipVersionAllowed = true;
	remindLaterAllowed = true;

	connect(&m_qnam, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),this, SLOT(slotAuthenticationRequired(QNetworkReply*, QAuthenticator*)));

	// Translation mechanism
	installTranslator();

#ifdef FV_DEBUG
	// Unit tests
	FvVersionComparatorTest* test = new FvVersionComparatorTest();
	test->runAll();
	delete test;
#endif

}

FvUpdater::~FvUpdater()
{
	if (m_proposedUpdate) {
		delete m_proposedUpdate;
		m_proposedUpdate = 0;
	}

#ifdef FV_GUI
	hideUpdaterWindow();
#endif
}

void FvUpdater::installTranslator()
{
	QTranslator translator;
	QString locale = QLocale::system().name();
	translator.load(QString("fervor_") + locale);

#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
#endif

	qApp->installTranslator(&translator);
}

#ifdef FV_GUI
void FvUpdater::showUpdaterWindowUpdatedWithCurrentUpdateProposal()
{
	// Destroy window if already exists
	hideUpdaterWindow();

	// Create a new window
	m_updaterWindow = new FvUpdateWindow(NULL, skipVersionAllowed, remindLaterAllowed);
	m_updaterWindow->UpdateWindowWithCurrentProposedUpdate();
	m_updaterWindow->show();
}

void FvUpdater::hideUpdaterWindow()
{
	if (m_updaterWindow) {
		if (! m_updaterWindow->close()) {
			qWarning() << "Update window didn't close, leaking memory from now on";
		}

		// not deleting because of Qt::WA_DeleteOnClose

		m_updaterWindow = 0;
	}
}

void FvUpdater::updaterWindowWasClosed()
{
	// (Re-)nullify a pointer to a destroyed QWidget or you're going to have a bad time.
	m_updaterWindow = 0;
}
#endif

void FvUpdater::SetFeedURL(QUrl feedURL)
{
	m_feedURL = feedURL;
}

void FvUpdater::SetFeedURL(QString feedURL)
{
	SetFeedURL(QUrl(feedURL));
}

void FvUpdater::SetComponentVersion( ComponentVersion version )
{
  currentComponentVersion_ = version;

}

QString FvUpdater::GetFeedURL()
{
	return m_feedURL.toString();
}

QString FvUpdater::GetEnclosureURL()
{
  return m_proposedUpdate->GetEnclosureUrl().toString();

}

FvAvailableUpdate* FvUpdater::GetProposedUpdate()
{
	return m_proposedUpdate;
}


bool FvUpdater::slotTriggerUpdate()
{
  qDebug() << "slotTriggerUpdate() entered.";

  if(m_proposedUpdate==NULL)
	{
		qWarning() << "Abort Update: No update prososed! This should not happen.";
		return false;
	}

	// Prepare download
	QUrl url = m_proposedUpdate->GetEnclosureUrl();

	// Check SSL Fingerprint if required
	if(url.scheme()=="https" && !m_requiredSslFingerprint.isEmpty())
		if( !checkSslFingerPrint(url) )	// check failed
		{	
			qWarning() << "Update aborted.";
			return false;
		}

	// Start Download
  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
	QNetworkReply* reply = m_qnam.get( request );
	connect(reply, SIGNAL(finished()), this, SLOT(slotHttpUpdateDownloadFinished()));

	// Maybe Check request 's return value
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug()<<"Unable to download the update: "<<reply->errorString();
		return false;
	}
	else
		qDebug()<<"Request to download update '" << url << "'' has been sent.";

	// Show download Window
#ifdef FV_GUI
	FvUpdateDownloadProgress* dlwindow = new FvUpdateDownloadProgress(NULL);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), dlwindow, SLOT(downloadProgress(qint64, qint64) ));
  // HACK AL: move to explicit call off close()
	// connect(&m_qnam, SIGNAL(finished(QNetworkReply*)), dlwindow, SLOT(close()));
	connect( this, SIGNAL(signalUpdateInstalled()), dlwindow, SLOT(close()));
	connect( this, SIGNAL(signalUpdateInstallFailed()), dlwindow, SLOT(close()));
	dlwindow->show();
#endif

	emit (signalUpdateTriggered());

#ifdef FV_GUI
	hideUpdaterWindow();
#endif
  
  return true;
}

void FvUpdater::slotHttpUpdateDownloadFinished()
{
  qDebug() << "slotHttpUpdateDownloadFinished() entered.";
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	if(reply==NULL)
	{
		qWarning()<<"The slot slotHttpUpdateDownloadFinished() should only be invoked by S&S.";
		return;
	}

	if(reply->error() == QNetworkReply::NoError)
	{
		int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();

		// no error received?
		if (reply->error() == QNetworkReply::NoError)
		{
			if (reply->isReadable())
			{
        // XXX AL show release notes also in PM
        
        /// @todo AL
        // base-disk Ordner an Fervor durchreichen, in base-disk Ordner entpacken systemconfig.h - getbasediskpath()

				// Write download into File
				QFileInfo fileInfo=reply->url().path();
				QString downloadFileName = downloadPath_ + fileInfo.fileName();
				//qDebug()<<"Writing downloaded file into "<<downloadFileName;
	
				QFile file(downloadFileName);
				file.open(QIODevice::WriteOnly);
				file.write(reply->readAll());
				file.close();

        bool success = true;
        if( slotVerifyUpdate( downloadFileName ) )
        {
          if( GetFeedURL().contains( "PrivacyMachine.xml" ) )
          {

            QList<QuaZipFileInfo64> updateFiles;
            QuaZip zip( downloadFileName );
            zip.setFileNameCodec("IBM866");
            updateFiles = zip.getFileInfoList64();

            // Rename all current files with available update.
            for (int i=0;i<updateFiles.size();i++)
            {
              QString sourceFilePath = targetPath_ + "\\" + updateFiles[i].name;
              QDir appDir( QCoreApplication::applicationDirPath() );

              QFileInfo file(	sourceFilePath );
              if(file.exists())
              {
                qDebug()<<tr("Moving file %1 to %2").arg(sourceFilePath).arg(sourceFilePath+".oldversion");
                SnX appDir.rename( sourceFilePath, sourceFilePath+".oldversion" );
              }
            }

          }

          // By default, all updates are zipped.
          // For binaries, unzip only after renaming all contained files (see above)
          // For base disks, we have to unzip before patching, if necessary (see below)
          // => the place for unzipping is in between the two.
          if (downloadFileName.endsWith((".7z")))
          {
            SnX slotUnzipUpdate_7z( downloadFileName, targetPath_, "" );

          }
          else
          {
            SnX slotUnzipUpdate( downloadFileName, targetPath_, "" );

          }

          if( !success )
          {
            qWarning() 
              << "Failed to unzip " << downloadFileName << "to " << targetPath_ << "."; 

          }

          // base disk
          if( !GetFeedURL().contains( "PrivacyMachine.xml" ) )
          {
            // If we got a delta, we have to patch it over the current file
            if( success && m_proposedUpdate->GetIsDelta() )
            {
              SnX Patch( m_proposedUpdate->GetCurrentFileName(),
                m_proposedUpdate->GetDeltaFileName(),
                m_proposedUpdate->GetTargetFileName() );

              if( !success )
              {
                qWarning() 
                  << "Failed to patch " << m_proposedUpdate->GetCurrentFileName()
                  << "to " << m_proposedUpdate->GetTargetFileName() 
                  << "using " << m_proposedUpdate->GetDeltaFileName();

              }

              // We try to delete the zip and rdiff in any case, so that things are neat and clean again.
              // In worst case this leads to repeated download of the same delta files, but ensures that if e.g. a delta
              // on the server had to be fixed, the fixed version gets downloaded.
              QFile( m_proposedUpdate->GetDeltaFileName() ).remove();
              QFile( downloadFileName ).remove();

            }
          }

          /// @todo AL: download löschen wenn fertig

          // FIXME AL: What happens with files that should be deleted?
          //   Possible solution: Include an update script in the update that takes care of rearranging file and folder
          //   structure.
  
          if( success )
          {
            emit( signalUpdateInstalled() );
          }
          else
          {
            emit( signalUpdateInstallFailed() );
          }

          // Only restart straightaway on Windows and, further more, if this is an interactive update and we are
          // updating the binary.
          #ifdef PM_WINDOWS
            if( !( m_silentAsMuchAsItCouldGet ) && GetFeedURL().contains( "PrivacyMachine.xml" ) )
            {
              cleanUpRestart( downloadFileName );
            }
          #endif

        }

			}
			else qDebug()<<"Error: QNetworkReply is not readable!";
		}
		else 
		{
			qDebug()<<"Download errors ocurred! HTTP Error Code:"<<httpstatuscode;
		}

		reply->deleteLater();
  }	// If !reply->error END
}	// slotHttpUpdateDownloadFinished END


void FvUpdater::cleanUpRestart( const QString & filePath )
{
  /// @todo AL ok to use busy and blocking wait here?
  while( QFile::remove( filePath ) )
  {};

  // Restart ap to clean up and start usual business
  restartApplication();

}



bool FvUpdater::slotUnzipUpdate(const QString & filePath, const QString & extDirPath, const QString & singleFileName )
{

  QuaZip zip(filePath);
  if (!zip.open(QuaZip::mdUnzip)) {
    qWarning()<<tr("Error: Unable to open zip archive %1 for unzipping: %2").arg(filePath).arg(zip.getZipError());
    return false;
  }


  zip.setFileNameCodec("IBM866");

  //qWarning("Update contains %d files\n", zip.getEntriesCount());

  QuaZipFileInfo64 info;
  QuaZipFile file(&zip);
  QFile out;
  QString name;
  QDir appDir(extDirPath);
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
  {
    if (!zip.getCurrentFileInfo(&info)) {
      qWarning()<<tr("Error: Unable to retrieve fileInfo about the file to extract: %2").arg(zip.getZipError());
      return false;
    }

    if (!singleFileName.isEmpty())
      if (!info.name.contains(singleFileName))
        continue;

    if (!file.open(QIODevice::ReadOnly)) {
      qWarning()<<tr("Error: Unable to open file %1 for unzipping: %2").arg(filePath).arg(file.getZipError());
      return false;
    }

    name = QString("%1/%2").arg(extDirPath).arg(file.getActualFileName());

    if (file.getZipError() != UNZ_OK) {
      qWarning()<<tr("Error: Unable to retrieve zipped filename to unzip from %1: %2").arg(filePath).arg(file.getZipError());
      return false;
    }

    QFileInfo fi(name);
    appDir.mkpath(fi.absolutePath() );	// Ensure that subdirectories - if required - exist 
    out.setFileName(name);
    out.open(QIODevice::WriteOnly);
    out.write( file.readAll() );
    out.close();

    if (file.getZipError() != UNZ_OK) {
      qWarning()<<tr("Error: Unable to unzip file %1: %2").arg(name).arg(file.getZipError());
      file.close();
      return false;
    }

    if (!file.atEnd()) {
      qWarning()<<tr("Error: Have read all available bytes, but pointer still does not show EOF: %1").arg(file.getZipError());
      file.close();
      return false;
    }

    file.close();

    if (file.getZipError() != UNZ_OK) {
      qWarning()<<tr("Error: Unable to close zipped file %1: %2").arg(name).arg(file.getZipError());
      return false;
    }
  }

  zip.close();

  if (zip.getZipError() != UNZ_OK) {
    qWarning()<<tr("Error: Unable to close zip archive file %1: %2").arg(filePath).arg(file.getZipError());
    return false;
  }

  return true;
}

bool FvUpdater::slotUnzipUpdate_7z(
  const QString & filePath, const QString & extDirPath, const QString & singleFileName )
{

  QString allOutput;
  QStringList args;
  QString cmd;


  #ifdef PM_WINDOWS
    cmd = "7za.exe";
  #else
    cmd = "7za";
  #endif
  args.clear();
  if (filePath.contains("base-disk_"))
  {
    QDir appDir(extDirPath);
    if (!appDir.exists())
    {
       if (!appDir.mkpath(".")) // creates subpaths also
       {
         IERR("failed to create directory: " + extDirPath + ", which is the target directory we want to unzip to." );
         return false;
       }
    }

    args.append("-o" + extDirPath);

  }
  args.append("e");
  args.append( "\"" + filePath + "\"" );
  bool success = ExecShort( cmd, args, &allOutput, true, -1, false);
  ILOG("Output of extraction: \n" + allOutput);

  return success;
}


bool FvUpdater::slotVerifyUpdate( const QString & filePath )
{
  bool success = false;
  QFile file( filePath );
  
  if( file.open( QIODevice::ReadOnly ) )
  {
      QCryptographicHash hash( QCryptographicHash::Sha256 );
      if( hash.addData( &file ) )
      {
        success = hash.result().toHex() == m_proposedUpdate->GetEnclosureChecksum();
        if( !success )
        {
          qWarning() << "Error: Failed to verify check sum of file '" << filePath << "': " << hash.result().toHex() 
            << " does not match check sum for enclosure, which is " << m_proposedUpdate->GetEnclosureChecksum() << ".";
          
        }
        
      }
      else
      {
        qWarning() << "Error: Failed to read '" << filePath <<"'.";
        
      }
      
      
  }      
  else
  {
    qWarning() << "Error: Failed to open '" << filePath <<"' for reading.";
  }
  
  return success;

}


void FvUpdater::SkipUpdate()
{
  qDebug() << "Skip update";

  FvAvailableUpdate* proposedUpdate = GetProposedUpdate();
  if (! proposedUpdate) {
    qWarning() << "Proposed update is NULL (shouldn't be at this point)";
    return;
  }

  // Start ignoring this particular version
  FVIgnoredVersions::IgnoreVersion( 
    proposedUpdate->GetEnclosureVersion(), 
    currentComponentVersion_.ToString() );

#ifdef FV_GUI
  hideUpdaterWindow();
#endif
}

void FvUpdater::RemindMeLater()
{
	//qDebug() << "Remind me later";

#ifdef FV_GUI
	hideUpdaterWindow();
#endif
}


bool FvUpdater::CheckForUpdateBlocking( uint timeoutSeconds, Verbosity::Enum verbosity = Verbosity::Silent )
{
  bool updateFound = true;
  QTimer timer;    
  timer.setSingleShot(true);

  QEventLoop loop;
  connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
  connect( FvUpdater::sharedUpdater(), SIGNAL( signalFoundUpdate() ), &loop, SLOT( quit() ) );
  timer.start( timeoutSeconds * 1000 ); 
  updateFound = updateFound && FvUpdater::sharedUpdater()->slotCheckForUpdates( verbosity );
  loop.exec();

  if( !timer.isActive() )
  {
    disconnect( FvUpdater::sharedUpdater(), SIGNAL( signalFoundUpdate() ), &loop, SLOT( quit() ) );
    updateFound = false;

  }

  return updateFound;

}


bool FvUpdater::InstallUpdateBlocking()
{
  bool success = true;
  
  QEventLoop loop;
  connect( FvUpdater::sharedUpdater(), SIGNAL( signalUpdateInstalled() ), &loop, SLOT( quit() ) );
  FvUpdater::sharedUpdater()->slotTriggerUpdate();
  loop.exec();

  return success;

}


bool FvUpdater::slotCheckForUpdates( Verbosity::Enum verbosity )
{
  if (m_feedURL.isEmpty()) {
    qCritical() << "Please set feed URL via setFeedURL() before calling slotCheckForUpdates().";
    return false;
  }

  m_silentAsMuchAsItCouldGet = verbosity == Verbosity::Silent;

  // Check if application's organization name and domain are set, fail otherwise
  // (nowhere to store QSettings to)
  if (QCoreApplication::organizationName().isEmpty()) {
    qCritical() << "QCoreApplication::organizationName is not set. Please do that.";
    return false;
  }
  if (QCoreApplication::organizationDomain().isEmpty()) {
    qCritical() << "QCoreApplication::organizationDomain is not set. Please do that.";
    return false;
  }

  if(QCoreApplication::applicationName().isEmpty()) {
    qCritical() << "QCoreApplication::applicationName is not set. Please do that.";
    return false;
  }

  // Set application version is not set yet
  if (currentComponentVersion_.ToString().isEmpty()) {
    qCritical() << "Component Version is not set. Please do that.";
    return false;
  }

  cancelDownloadFeed();
  m_httpRequestAborted = false;
  startDownloadFeed(m_feedURL);

  return true;
}

void FvUpdater::startDownloadFeed(QUrl url)
{
  m_xml.clear();

  // Check SSL Fingerprint if required
  if(url.scheme()=="https" && !m_requiredSslFingerprint.isEmpty())
    if( !checkSslFingerPrint(url) )	// check failed
    {	
      qWarning() << "Update aborted.";
      return;
    }


  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  m_reply = m_qnam.get( request );

  connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotHttpFeedReadyRead()));
  connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(slotHttpFeedUpdateDataReadProgress(qint64, qint64)));
  connect(m_reply, SIGNAL(finished()), this, SLOT(slotHttpFeedDownloadFinished()));
}

void FvUpdater::cancelDownloadFeed()
{
  if (m_reply) {
    m_httpRequestAborted = true;
    m_reply->abort();
  }
}

void FvUpdater::slotHttpFeedReadyRead()
{
  // this slot gets called every time the QNetworkReply has new data.
  // We read all of its new data and write it into the file.
  // That way we use less RAM than when reading it at the finished()
  // signal of the QNetworkReply
  m_xml.addData(m_reply->readAll());
}

void FvUpdater::slotHttpFeedUpdateDataReadProgress(qint64 bytesRead,
  qint64 totalBytes)
{
  Q_UNUSED(bytesRead);
  Q_UNUSED(totalBytes);

  if (m_httpRequestAborted) {
    return;
  }
}

void FvUpdater::slotHttpFeedDownloadFinished()
{
  if (m_httpRequestAborted) {
    m_reply->deleteLater();
    return;
  }

  QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (m_reply->error()) {

    // Error.
    showErrorDialog(tr("Feed download failed: %1.").arg(m_reply->errorString()), false);

  } else if (! redirectionTarget.isNull()) {
    QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());

    m_feedURL = newUrl;
    m_reply->deleteLater();

    startDownloadFeed(m_feedURL);
    return;

  } else {

    // Done.
    xmlParseFeed();

  }

  m_reply->deleteLater();
  m_reply = 0;
}

bool FvUpdater::xmlParseFeed()
{
  qDebug() << "xmlParseFeed() entered.";
	QString currentTag, currentQualifiedTag;

  bool isDelta = false;
	QString xmlTitle;
  QString xmlLink;
  QString xmlReleaseNotesLink;
  QString xmlPubDate;
  QString xmlEnclosureFileName;
  QString xmlEnclosureFileChecksum = "<checksum not found>";
  QString xmlEnclosurePathUrl;
  QString xmlEnclosureVersion;
  QString xmlEnclosurePlatform;
  QString xmlEnclosureType;
	unsigned long xmlEnclosureLength;

  // Creating m_proposedUpdate already here, as we need it for telling us the enclosure file name within the feed that
  // we are looking for, which is version-dependent. And we only want to get one of all enclosures that are available
  // for a specific version (e.g. just one delta, or the whole new component).
  if( m_proposedUpdate ) 
  {
    delete m_proposedUpdate; 
    m_proposedUpdate = 0;

  }
  m_proposedUpdate = new FvAvailableUpdate();

	// Parse
	while (! m_xml.atEnd()) {

		m_xml.readNext();

		if (m_xml.isStartElement()) {

			currentTag = m_xml.name().toString();
			currentQualifiedTag = m_xml.qualifiedName().toString();

			if (m_xml.name() == "item") {

				xmlTitle.clear();
				xmlLink.clear();
				xmlReleaseNotesLink.clear();
				xmlPubDate.clear();
				xmlEnclosurePathUrl.clear();
				xmlEnclosureVersion.clear();
				xmlEnclosurePlatform.clear();
				xmlEnclosureLength = 0;
				xmlEnclosureType.clear();

			} else if (m_xml.name() == "enclosurepath") {

				QXmlStreamAttributes attribs = m_xml.attributes();

				if (attribs.hasAttribute("fervor:platform"))
				{
					xmlEnclosurePlatform = attribs.value("fervor:platform").toString().trimmed();

					if (FvPlatform::CurrentlyRunningOnPlatform(xmlEnclosurePlatform))
					{
						xmlEnclosurePathUrl = attribs.hasAttribute("url") ? attribs.value("url").toString().trimmed() : "";
				
						xmlEnclosureVersion = "";
						if (attribs.hasAttribute("fervor:version")) 
							xmlEnclosureVersion = attribs.value("fervor:version").toString().trimmed();
						if (attribs.hasAttribute("sparkle:version"))
							xmlEnclosureVersion = attribs.value("sparkle:version").toString().trimmed();
				
            m_proposedUpdate->SetEnclosureVersion( 
              xmlEnclosureVersion, currentComponentVersion_.ToString(), GetFeedURL() );
					  xmlEnclosureFileName = m_proposedUpdate->GetEnclosureFileName();

						xmlEnclosureLength = attribs.hasAttribute("length") ? attribs.value("length").toString().toLong() : 0;
		
						xmlEnclosureType = attribs.hasAttribute("type") ? attribs.value("type").toString().trimmed() : "";
					}

				}	// if hasAttribute flevor:platform END

			} else if (m_xml.name() == "checksum") {

        QXmlStreamAttributes attributes = m_xml.attributes();
        if( attributes.hasAttribute( "filename" ) && attributes.value( "filename" ).toString().trimmed() 
          == xmlEnclosureFileName )        
        {
          xmlEnclosureFileChecksum = attributes.hasAttribute( "sha256" ) 
            ? attributes.value( "sha256" ).toString().trimmed()
            : "<checksum not found>";

        }

      }

		} else if (m_xml.isEndElement()) {

			if (m_xml.name() == "item") {

				// That's it - we have analyzed a single <item> and we'll stop
				// here (because the topmost is the most recent one, and thus
				// the newest version.

				return searchDownloadedFeedForUpdates(xmlTitle,
													  xmlLink,
													  xmlReleaseNotesLink,
													  xmlPubDate,
													  xmlEnclosurePathUrl + xmlEnclosureFileName,
													  xmlEnclosureFileChecksum,
													  xmlEnclosureVersion,
													  xmlEnclosurePlatform,
													  xmlEnclosureLength,
													  xmlEnclosureType);

			}

		} else if (m_xml.isCharacters() && ! m_xml.isWhitespace()) {

			if (currentTag == "title") {
				xmlTitle += m_xml.text().toString().trimmed();

			} else if (currentTag == "link") {
				xmlLink += m_xml.text().toString().trimmed();

			} else if (currentQualifiedTag == "sparkle:releaseNotesLink") {
				xmlReleaseNotesLink += m_xml.text().toString().trimmed();

			} else if (currentTag == "pubDate") {
				xmlPubDate += m_xml.text().toString().trimmed();

			}

		}

		if (m_xml.error() && m_xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {

			showErrorDialog(tr("Feed parsing failed: %1 %2.").arg(QString::number(m_xml.lineNumber()), m_xml.errorString()), false);
      if( m_proposedUpdate ) 
      {
        delete m_proposedUpdate; 
        m_proposedUpdate = 0;
    
      }
			return false;

		}
	}

  if( m_proposedUpdate ) 
  {
    delete m_proposedUpdate; 
    m_proposedUpdate = 0;

  }
	return false;
}


bool FvUpdater::searchDownloadedFeedForUpdates(QString xmlTitle,
											   QString xmlLink,
											   QString xmlReleaseNotesLink,
											   QString xmlPubDate,
											   QString xmlEnclosureFilePath,
											   QString xmlEnclosureFileChecksum,
											   QString xmlEnclosureVersion,
											   QString xmlEnclosurePlatform,
											   unsigned long xmlEnclosureLength,
											   QString xmlEnclosureType)
{
  qDebug() << "searchDownloadedFeedForUpdates() entered.";
	Q_UNUSED(xmlTitle);
	Q_UNUSED(xmlPubDate);
	Q_UNUSED(xmlEnclosureLength);
	Q_UNUSED(xmlEnclosureType);

  bool proposeUpdate = true;
  bool success = true;

	// Validate
	if (xmlReleaseNotesLink.isEmpty()) {
		if (xmlLink.isEmpty()) {
			showErrorDialog(tr("Feed error: \"release notes\" link is empty"), false);
      proposeUpdate = false;
			success = false;
		} else {
			xmlReleaseNotesLink = xmlLink;
		}
	} else {
		xmlLink = xmlReleaseNotesLink;
	}
	if (! (xmlLink.startsWith("http://") || xmlLink.startsWith("https://") || xmlLink.startsWith("file://"))) {
		showErrorDialog(tr("Feed error: invalid \"release notes\" link"), false);
    proposeUpdate = false;
		success = false;
	}
	if (xmlEnclosureFilePath.isEmpty() || xmlEnclosureVersion.isEmpty() || xmlEnclosurePlatform.isEmpty()) {
		showErrorDialog(tr("Feed error: invalid \"enclosure\" with the download link"), false);
    proposeUpdate = false;
		success = false;
	}
  
	// Relevant version?
	if( FVIgnoredVersions::VersionIsIgnored( xmlEnclosureVersion, currentComponentVersion_.ToString() ) ) {
		qDebug() << "Version '" << xmlEnclosureVersion << "' is ignored, too old or something like that.";

		showInformationDialog(tr("No updates were found."), false);
    proposeUpdate = false;
	}


	// Success! At this point, we have found an update that can be proposed
	// to the user.
  if( proposeUpdate )
  {
    downloadReleaseNotes( xmlReleaseNotesLink, 500 );

    m_proposedUpdate->SetTitle(xmlTitle);
    m_proposedUpdate->SetPubDate(xmlPubDate);
    m_proposedUpdate->SetEnclosureChecksum(xmlEnclosureFileChecksum);
    m_proposedUpdate->SetEnclosureUrl(xmlEnclosureFilePath);
    m_proposedUpdate->SetEnclosurePlatform(xmlEnclosurePlatform);
    m_proposedUpdate->SetEnclosureLength(xmlEnclosureLength);
    m_proposedUpdate->SetEnclosureType(xmlEnclosureType);

    emit( signalFoundUpdate() );

    #ifdef FV_GUI
      // Show "look, there's an update" window
      showUpdaterWindowUpdatedWithCurrentUpdateProposal();

    #else
      // Decide ourselves what to do
      decideWhatToDoWithCurrentUpdateProposal();

    #endif

  }
  // If we did not find an update that is to be proposed, drop it again.
  else
  {
    if( m_proposedUpdate ) 
    {
      delete m_proposedUpdate; 
      m_proposedUpdate = 0;

    }

  }

	return success;
}


void FvUpdater::showErrorDialog(QString message, bool showEvenInSilentMode)
{
	if (m_silentAsMuchAsItCouldGet) {
		if (! showEvenInSilentMode) {
			// Don't show errors in the silent mode
			return;
		}
	}

#ifdef FV_GUI
	QMessageBox dlFailedMsgBox;
	dlFailedMsgBox.setIcon(QMessageBox::Critical);
	dlFailedMsgBox.setText(tr("Error"));
	dlFailedMsgBox.setInformativeText(message);
	dlFailedMsgBox.exec();
#else
	qCritical() << message;
#endif
}

void FvUpdater::showInformationDialog(QString message, bool showEvenInSilentMode)
{
	if (m_silentAsMuchAsItCouldGet) {
		if (! showEvenInSilentMode) {
			// Don't show information dialogs in the silent mode
			return;
		}
	}

#ifdef FV_GUI
	QMessageBox dlInformationMsgBox;
	dlInformationMsgBox.setIcon(QMessageBox::Information);
	dlInformationMsgBox.setText(tr("Information"));
	dlInformationMsgBox.setInformativeText(message);
	dlInformationMsgBox.exec();
#else
	qDebug() << message;
#endif
}

void FvUpdater::finishUpdate(QString pathToFinish)
{
	pathToFinish = pathToFinish.isEmpty() ? QCoreApplication::applicationDirPath() : pathToFinish;
	QDir appDir(pathToFinish);
	appDir.setFilter( QDir::Files | QDir::Dirs );

	QFileInfoList dirEntries = appDir.entryInfoList();
	foreach (QFileInfo fi, dirEntries)
	{
		if ( fi.isDir() )
		{
            QString dirname = fi.fileName();
            if ((dirname==".") || (dirname == ".."))
                continue;
			
			// recursive clean up subdirectory
			finishUpdate(fi.filePath());
		}
		else
		{
			if(fi.suffix()=="oldversion")
				if( !appDir.remove( fi.absoluteFilePath() ) )
					qDebug()<<"Error: Unable to clean up file: "<<fi.absoluteFilePath();

		}
	}	// For each dir entry END
}

void FvUpdater::restartApplication()
{
	// Spawn a new instance of myApplication:
    QString app = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    QString wd = QDir::currentPath();
    qDebug() << app << arguments << wd;
    QProcess::startDetached(app, arguments, wd);
    QApplication::exit();
}

void FvUpdater::setRequiredSslFingerPrint(QString md5)
{
	m_requiredSslFingerprint = md5.remove(":");
}

QString FvUpdater::getRequiredSslFingerPrint()
{
	return m_requiredSslFingerprint;
}

bool FvUpdater::checkSslFingerPrint(QUrl urltoCheck)
{
	if(urltoCheck.scheme()!="https")
	{
		qWarning()<<tr("SSL fingerprint check: The url %1 is not a ssl connection!").arg(urltoCheck.toString());
		return false;
	}

	QSslSocket *socket = new QSslSocket(this);
	socket->connectToHostEncrypted(urltoCheck.host(), 443);
	if( !socket->waitForEncrypted(1000))	// waits until ssl emits encrypted(), max 1000msecs
	{
		qWarning()<<"SSL fingerprint check: Unable to connect SSL server: "<<socket->sslErrors();
		return false;
	}

	QSslCertificate cert = socket->peerCertificate();

	if(cert.isNull())
	{
		qWarning()<<"SSL fingerprint check: Unable to retrieve SSL server certificate.";
		return false;
	}

	// COmpare digests
	if(cert.digest().toHex() != m_requiredSslFingerprint)
	{
		qWarning()<<"SSL fingerprint check: FINGERPRINT MISMATCH! Server digest="<<cert.digest().toHex()<<", requiered ssl digest="<<m_requiredSslFingerprint;
		return false;
	}
	
	return true;
}

void FvUpdater::slotAuthenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator )
{
	if(reply==NULL || authenticator==NULL)
		return;

	if(!authenticator->user().isEmpty())	// If there is already a login user set but an authentication is still required: credentials must be wrong -> abort
	{
		reply->abort();
		qWarning()<<"Http authentication: Wrong credentials!";
		return;
	}

	authenticator->setUser(htAuthUsername);
	authenticator->setPassword(htAuthPassword);
}

void FvUpdater::setHtAuthCredentials(QString user, QString pass)
{
	htAuthUsername = user;
	htAuthPassword = pass;
}

void FvUpdater::setHtAuthUsername(QString user)
{
	htAuthUsername = user;
}

void FvUpdater::setHtAuthPassword(QString pass)
{
	htAuthPassword = pass;
}

void FvUpdater::setSkipVersionAllowed(bool allowed)
{
	skipVersionAllowed = allowed;
}

void FvUpdater::setRemindLaterAllowed(bool allowed)
{
	remindLaterAllowed = allowed;
}

bool FvUpdater::getSkipVersionAllowed()
{
	return skipVersionAllowed;
}

bool FvUpdater::getRemindLaterAllowed()
{
	return remindLaterAllowed;
}

#ifndef FV_GUI

void FvUpdater::decideWhatToDoWithCurrentUpdateProposal()
{
  QString policy = "install"; 
	if(policy == "install")
		slotTriggerUpdate();
	else if(policy == "skip")
		SkipUpdate();
	else
		RemindMeLater();
}

#endif


QString FvUpdater::getComponentVersion( QString applicationVersion )
{
  return applicationVersion.split(".")[2] + "." + applicationVersion.split(".")[3];
  
}


void FvUpdater::downloadReleaseNotes( QString url, ulong milliseconds )
{
  // On success, this will be overwritten with the actual release notes.
  m_proposedUpdate->SetReleaseNotesText( "<Failed to download release notes.>" );

  // from http://stackoverflow.com/questions/13207493/qnetworkreply-and-qnetworkaccessmanager-timeout-in-http-request
  QTimer timer;    
  timer.setSingleShot(true);

  // Start Release Notes Download
  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  QNetworkReply* reply = m_qnam.get( request );
  QEventLoop loop;
  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  timer.start(milliseconds); 
  loop.exec();
  
  if( timer.isActive() )
  {
    if (reply->error() != QNetworkReply::NoError)
    {
      qDebug()<<"Unable to download the release notes: " << reply->errorString();
       
    }
    else if (reply->isReadable() && m_proposedUpdate )
	  { 
      m_proposedUpdate->SetReleaseNotesText( reply->readAll() );
      
    }
    
  }
  else
  {
    // timeout
    reply->abort();    
    disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
     
    qDebug()<<"Timeout after " << milliseconds << "ms while downloading release notes: " << reply->errorString();

  }
  
}


bool FvUpdater::Patch( QString source, QString delta, QString target )
{
  bool success = true;
  FILE *handles[3] = { NULL };
  QString names[] = { source, delta, target };
  QString accesses[] = { "rb", "rb", "w+b" };

  for( int fileIndex = 0; fileIndex < 3  &&  success; ++fileIndex )
  {
    // Use binary access, as we also patch binary files.
    handles[ fileIndex ] = fopen( names[ fileIndex ].toStdString().c_str(), accesses[ fileIndex ].toStdString().c_str() );

  }

  success = success && 
    rs_patch_file( handles[ 0 ], handles[ 1 ], handles[ 2 ], NULL ) == RS_DONE;

  for( int fileIndex = 0; fileIndex < 3; ++fileIndex )
  {
    if( handles[ fileIndex ] != NULL )
    {
      fclose( handles[ fileIndex ] );
      handles[ fileIndex ] = NULL;

    }

  }

  return success;

}

bool FvUpdater::setDownloadPath( const QString& path )
{
  if( path != "" )
  {
    downloadPath_ = path;

  }
  else
  {
    #ifdef Q_WS_MAC
      CFURLRef appURLRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(appURLRef, TRUE, (UInt8 *)path, PATH_MAX)) {
        // error!
      }

      CFRelease(appURLRef);
      QString filePath = QString(path);
      downloadPath_ = filePath.left(filePath.lastIndexOf("/"));

    #else
      downloadPath_ = QCoreApplication::applicationDirPath() + "/";

    #endif

  }              

  return createDirectoriesIfMissing( downloadPath_ );

}


bool FvUpdater::setTargetPath( const QString& path )
{
  if( path != "" )
  {
    targetPath_ = path;

  }
  else
  {
    #ifdef Q_WS_MAC
      CFURLRef appURLRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(appURLRef, TRUE, (UInt8 *)path, PATH_MAX)) {
        // error!
      }

      CFRelease(appURLRef);
      QString filePath = QString(path);
      targetPath_ = filePath.left(filePath.lastIndexOf("/"));

    #else
      targetPath_ = QCoreApplication::applicationDirPath() + "/";

    #endif

  }              

  return createDirectoriesIfMissing( targetPath_ );

}


bool FvUpdater::createDirectoriesIfMissing( const QString& path )
{
  QDir directory( path );
  if (!directory.exists())
  {
     if (!directory.mkpath(".")) // creates subpaths also
     {
       IERR( "failed to create directory: " + path );
       return false;
     }
  }

  return true;

}
