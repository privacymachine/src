#ifndef FVAVAILABLEUPDATE_H
#define FVAVAILABLEUPDATE_H

#include <QApplication>
#include <QObject>
#include <QUrl>
#include <QStringList>

// From http://www.reedbeta.com/blog/2013/07/10/cpp-compile-time-array-size/
template <typename T, int N> char( &dim_helper( T(&)[N] ) )[N];
#define dim(x) (sizeof(dim_helper(x)))

class FvAvailableUpdate : public QObject
{
	Q_OBJECT
public:
	explicit FvAvailableUpdate(QObject *parent = 0);

	QString GetTitle();
	void SetTitle(QString title);

	QString GetReleaseNotesText();
	void SetReleaseNotesText( QString text );

	QString GetPubDate();
	void SetPubDate(QString pubDate);

  QString GetEnclosureChecksum();
  void SetEnclosureChecksum(QString checksum);

  QString GetDeltaFileName();

  QString GetEnclosureFileName();

  QString GetCurrentFileName();
  QString GetTargetFileName();

	QUrl GetEnclosureUrl();
	void SetEnclosureUrl(QUrl enclosureUrl);
	void SetEnclosureUrl(QString enclosureUrl);

	QString GetEnclosureVersion();
  bool SetEnclosureVersion( QString enclosureVersion, QString currentVersion, QString feedUrl );

	QString GetEnclosurePlatform();
	void SetEnclosurePlatform(QString enclosurePlatform);

	unsigned long GetEnclosureLength();
	void SetEnclosureLength(unsigned long enclosureLength);

	QString GetEnclosureType();
	void SetEnclosureType(QString enclosureType);

	bool GetIsDelta();

private:
  bool m_isDelta;
	QString m_currentFileName;
  QString m_deltaFileName;
	QString m_targetFileName;
	QString m_title;
	QString m_releaseNotesText;
	QString m_pubDate;
	QUrl m_enclosureUrl;
	QString m_enclosureChecksum;
  QString m_enclosureFileName;
	QString m_enclosureVersion;
	QString m_enclosurePlatform;
	unsigned long m_enclosureLength;
	QString m_enclosureType;

};

#endif // FVAVAILABLEUPDATE_H
