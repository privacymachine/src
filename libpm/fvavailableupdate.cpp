#include "fvavailableupdate.h"

FvAvailableUpdate::FvAvailableUpdate(QObject *parent) :
	QObject(parent)
{
	// noop
}

QString FvAvailableUpdate::GetTitle()
{
	return m_title;
}

void FvAvailableUpdate::SetTitle(QString title)
{
	m_title = title;
}

QString FvAvailableUpdate::GetReleaseNotesText()
{
	return m_releaseNotesText;
}

void FvAvailableUpdate::SetReleaseNotesText( QString text )
{
	m_releaseNotesText = text;
}

QString FvAvailableUpdate::GetPubDate()
{
	return m_pubDate;
}

void FvAvailableUpdate::SetPubDate(QString pubDate)
{
	m_pubDate = pubDate;
}

QString FvAvailableUpdate::GetEnclosureChecksum()
{
	return m_enclosureChecksum;
}

void FvAvailableUpdate::SetEnclosureChecksum(QString checksum)
{
	m_enclosureChecksum = checksum;
}


QString FvAvailableUpdate::GetCurrentFileName()
{
  return m_currentFileName;
  
}


QString FvAvailableUpdate::GetDeltaFileName()
{
  return m_deltaFileName;
  
}


QString FvAvailableUpdate::GetEnclosureFileName()
{
  return m_enclosureFileName;
  
}


QString FvAvailableUpdate::GetTargetFileName()
{
  return m_targetFileName;
  
}


QUrl FvAvailableUpdate::GetEnclosureUrl()
{
	return m_enclosureUrl;
}

void FvAvailableUpdate::SetEnclosureUrl(QUrl enclosureUrl)
{
	m_enclosureUrl = enclosureUrl;
}

void FvAvailableUpdate::SetEnclosureUrl(QString enclosureUrl)
{
	SetEnclosureUrl(QUrl(enclosureUrl));
}

QString FvAvailableUpdate::GetEnclosureVersion()
{
	return m_enclosureVersion;
}

bool FvAvailableUpdate::SetEnclosureVersion( QString enclosureVersion, QString currentVersion, QString feedUrl )
{
  m_enclosureVersion = enclosureVersion;

  // If in doubt, assume that we cannot perform the update.
  m_enclosureFileName = "<update from " + currentVersion + " to " + enclosureVersion + " not supported>";  

  // Argument parsing
  // Version strings have to have 4 numbers, separated by periods.
  if( m_enclosureVersion.split( "." ).size() != 4
    || currentVersion.split( "." ).size() != 4 )
  {
    return false;
  }

  QString marketingVersion = currentVersion.split(".")[0];
  QString apiVersion = currentVersion.split(".")[1];
  QString currentVersionMajor = currentVersion.split(".")[2];
  QString currentVersionMinor = currentVersion.split(".")[3];
  QString targetVersionMajor = enclosureVersion.split(".")[2];
  QString targetVersionMinor = enclosureVersion.split(".")[3];

  QString* versionLiterals[] = 
  { &apiVersion, &currentVersionMajor, &currentVersionMinor, &targetVersionMajor, &targetVersionMinor };
  bool isInteger = false;
  for( int literalIndex = 0; literalIndex < dim( versionLiterals ); ++literalIndex )
  {
    versionLiterals[ literalIndex ]->toInt( &isInteger );
    if( !isInteger )
    {
      return false;
    }
  }

  m_deltaFileName = "";
  m_isDelta = false;


  if( feedUrl.contains( "PrivacyMachine.xml" ) )
  {
    #ifdef PM_WINDOWS
      m_enclosureFileName = "PrivacyMachine_win64-";

    #else
      m_enclosureFileName = "PrivacyMachine_debian-jessie-";

    #endif

    m_currentFileName = "";
    m_targetFileName = "";

    m_enclosureFileName 
      += marketingVersion + "_"
      + apiVersion + "_"
      + targetVersionMajor + "_"
      + targetVersionMinor + ".zip";

  } // feedUrl.contains( "PrivacyMachine.xml" )
  // assume base disk by default:
  else
  {
    // If we are more than 5 base disks behind, get a new base disk
    if( targetVersionMajor.toInt() > ( currentVersionMajor.toInt() + 5 ) 
      ||  currentVersionMajor.toInt() == 0  &&  currentVersionMinor.toInt() == 0 )
    {
      m_enclosureFileName = "base-disk_" + targetVersionMajor + ".7z";

    }
    // Otherwise, only get the delta, which should consume less bandwidth
    else 
    {
      m_isDelta = true;
      m_enclosureFileName = "base-disk_delta_" + currentVersionMajor + "-" + targetVersionMajor + ".7z";
      m_deltaFileName = "base-disk_delta_" + currentVersionMajor + "-" + targetVersionMajor + ".flat.rdiff";

    }

    m_currentFileName = "base-disk_" + currentVersionMajor + ".flat.vmdk";
    m_targetFileName = "base-disk_" + targetVersionMajor + ".flat.vmdk";

  }

}

QString FvAvailableUpdate::GetEnclosurePlatform()
{
	return m_enclosurePlatform;
}

void FvAvailableUpdate::SetEnclosurePlatform(QString enclosurePlatform)
{
	m_enclosurePlatform = enclosurePlatform;
}

unsigned long FvAvailableUpdate::GetEnclosureLength()
{
	return m_enclosureLength;
}

void FvAvailableUpdate::SetEnclosureLength(unsigned long enclosureLength)
{
	m_enclosureLength = enclosureLength;
}

QString FvAvailableUpdate::GetEnclosureType()
{
	return m_enclosureType;
}

void FvAvailableUpdate::SetEnclosureType(QString enclosureType)
{
	m_enclosureType = enclosureType;
}

bool FvAvailableUpdate::GetIsDelta()
{
  return m_isDelta;
}

