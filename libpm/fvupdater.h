#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <stdio.h>
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtNetwork>

#include "fvavailableupdate.h"
#include "fvignoredversions.h"
#include "fvplatform.h"
#include <librsync.h>
#include "quazip.h"
#include "quazipfile.h"

#ifdef Q_WS_MAC
  #include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef FV_GUI
  #include "fvupdatewindow.h"
  #include "fvupdatedownloadprogress.h"
  #include <QMessageBox>
  #include <QDesktopServices>

#else
  // QSettings key for automatic update installation
  #define FV_NEW_VERSION_POLICY_KEY              "FVNewVersionPolicy"

#endif

#ifdef FV_DEBUG
	// Unit tests
  #include "fvversioncomparatortest.h"
#endif

#include "ComponentVersion.h"


// TODO AL trying out alternative for better readability - generalize if it proves useful.
#define SnX success = success &&


class QNetworkReply;
class FvUpdateWindow;
class FvUpdateConfirmDialog;
class FvAvailableUpdate;


class Component
{
public:
  // Just in case we want to address multiple components at the same time, we set orthogonal values so that we can use
  // them as flags.
  enum Enum
  {
    BaseDisk = 1,
    Binary = 2,
    Configuration = 4

  } ComponentEnum;

};


class Verbosity
{
public:
  enum Enum
  {
    Silent,
    Interactive

  };

};


class FvUpdater : public QObject
{
	Q_OBJECT

public:

	// Singleton
	static FvUpdater* sharedUpdater();
	static void drop();

	// Set / get feed URL
	void SetFeedURL(QUrl feedURL);
	void SetFeedURL(QString feedURL);
  void SetComponentVersion( ComponentVersion version );
	QString GetFeedURL();
	QString GetEnclosureURL();
	void finishUpdate(QString pathToFinish = "");
	void setRequiredSslFingerPrint(QString md5);
	QString getRequiredSslFingerPrint();	// returns md5!
	// HTTP Authentuication - for security reasons no getters are provided, only a setter
	void setHtAuthCredentials(QString user, QString pass);
	void setHtAuthUsername(QString user);
	void setHtAuthPassword(QString pass);
	void setSkipVersionAllowed(bool allowed);
	void setRemindLaterAllowed(bool allowed);
	bool getSkipVersionAllowed();
	bool getRemindLaterAllowed();

  /// Downloaded files will go here.
  /// Set to "" to use QCoreApplication::applicationDirPath() + "/" on Linux or
  /// CFBundleCopyBundleURL(CFBundleGetMainBundle()) on Mac.
  bool setDownloadPath( const QString& path );

  /// Contents of downloaded zip files will go here. If set to "" this method behaves like setDownloadPath().
  bool setTargetPath( const QString& path );

  bool CheckForUpdateBlocking( uint timeoutSeconds, Verbosity::Enum verbosity );
  bool InstallUpdateBlocking();
	
public slots:

  /// \return \c true if checking for updates was successful, \c false otherwise. This does not say whether there is an
  /// update available or not.
	bool slotCheckForUpdates( Verbosity::Enum verbosity = Verbosity::Silent );

  /// Connect this to a button or, if one silently checks for updates to have them installed automatically, call this
  /// directly.
  /// \return \c true if triggering the actual update succeeded. This does not necessarily mean that the update was
  /// completed successfully.
  bool slotTriggerUpdate();

  /// \return \c true on success
  bool slotUnzipUpdate( const QString & filePath, const QString & extDirPath, const QString & singleFileName );

  /// \return \c true on success
  bool slotUnzipUpdate_7z( const QString & filePath, const QString & extDirPath, const QString & singleFileName );

  //
	// ---------------------------------------------------
	// ---------------------------------------------------
	// ---------------------------------------------------
	// ---------------------------------------------------
	//

protected:

	friend class FvUpdateWindow;		// Uses GetProposedUpdate() and others
	friend class FvUpdateConfirmDialog;	// Uses GetProposedUpdate() and others
	FvAvailableUpdate* GetProposedUpdate();

  /// The current version of component_.
  ComponentVersion currentComponentVersion_;


protected slots:

	// Update window button slots
	void SkipUpdate();
	void RemindMeLater();

private:

	//
	// Singleton business
	//
	// (we leave just the declarations, so the compiler will warn us if we try
	//  to use those two functions by accident)
	FvUpdater();							// Hide main constructor
	~FvUpdater();							// Hide main destructor
	FvUpdater(const FvUpdater&);			// Hide copy constructor
	FvUpdater& operator=(const FvUpdater&);	// Hide assign op

	static FvUpdater* m_Instance;			// Singleton instance


  QString downloadPath_;
  QString targetPath_;

  bool createDirectoriesIfMissing( const QString& path );

	//
	// Windows / dialogs
	//
#ifdef FV_GUI
	FvUpdateWindow* m_updaterWindow;								// Updater window (NULL if not shown)
	void showUpdaterWindowUpdatedWithCurrentUpdateProposal();		// Show updater window
	void hideUpdaterWindow();										// Hide + destroy m_updaterWindow
	void updaterWindowWasClosed();									// Sent by the updater window when it gets closed
#else
	void decideWhatToDoWithCurrentUpdateProposal();                 // Perform an action which is configured in settings
#endif

	// Available update (NULL if not fetched)
	FvAvailableUpdate* m_proposedUpdate;

	// If true, don't show the error dialogs and the "no updates." dialog
	// (silentAsMuchAsItCouldGet from CheckForUpdates() goes here)
	// Useful for automatic update checking upon application startup.
	bool m_silentAsMuchAsItCouldGet;

	// Dialogs (notifications)
	bool skipVersionAllowed;
	bool remindLaterAllowed;

	void showErrorDialog(QString message, bool showEvenInSilentMode = false);			// Show an error message
	void showInformationDialog(QString message, bool showEvenInSilentMode = false);		// Show an informational message


	//
	// HTTP feed fetcher infrastructure
	//
	QUrl m_feedURL;					// Feed URL that will be fetched
	QNetworkAccessManager m_qnam;
	QNetworkReply* m_reply;
	int m_httpGetId;
	bool m_httpRequestAborted;

	void startDownloadFeed(QUrl url);	// Start downloading feed
	void cancelDownloadFeed();			// Stop downloading the current feed
  
  /// \param milliseconds Timeout, in milliseconds, after which the download is aborted.
  void downloadReleaseNotes( QString url, ulong milliseconds );
  
	//
	// SSL Fingerprint Check infrastructure
	//
	QString m_requiredSslFingerprint;

	bool checkSslFingerPrint(QUrl urltoCheck);	// true=ssl Fingerprint accepted, false= ssl Fingerprint NOT accepted

	//
	// Htauth-Infrastructure
	//
	QString htAuthUsername;
	QString htAuthPassword;


	//
	// XML parser
	//
	QXmlStreamReader m_xml;				// XML data collector and parser
	bool xmlParseFeed();				// Parse feed in m_xml
	bool searchDownloadedFeedForUpdates(
    QString xmlTitle,
    QString xmlLink,
    QString xmlReleaseNotesLink,
    QString xmlPubDate,
    QString xmlEnclosureFilePath,
    QString xmlEnclosureFileChecksum,
    QString xmlEnclosureVersion,
    QString xmlEnclosurePlatform,
    unsigned long xmlEnclosureLength,
    QString xmlEnclosureType);

	//
	// Helpers
	//
	void installTranslator();			// Initialize translation mechanism
	void restartApplication();			// Restarts application after update
  
  QString getEnclosureFileName( QString targetVersion );
  
  // Trims marketing version and API from <marketing version>.<API>.<major>.<minor>
  QString getComponentVersion( QString applicationVersion );

  void cleanUpRestart( const QString & filePath );
  
  /// Convenience method that wraps rsync's rs_patch_file().
  bool Patch( QString source, QString delta, QString target );

private slots:

	void slotAuthenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator );
	void slotHttpFeedReadyRead();
	void slotHttpFeedUpdateDataReadProgress(qint64 bytesRead,
										qint64 totalBytes);
	void slotHttpFeedDownloadFinished();

	//
	// Download and install Update infrastructure
	//
	void slotHttpUpdateDownloadFinished();

  /// \return \c true on success
	bool slotVerifyUpdate( const QString & filePath );	

signals:
  /// Emitted if triggering the actual update succeeded. This does not necessarily mean that the update was completed
  /// successfully.
	void signalUpdateTriggered();

  /// Emitted after the update has been installed. This might still require cleanup and restarting the application.
  /// \see cleanUpRestart().
  void signalUpdateInstalled();

  /// The other option of "finishing" updates.
  void signalUpdateInstallFailed();

  /// Emitted after an update has been found and prepared for download.
  void signalFoundUpdate();

};

#endif // FVUPDATER_H
