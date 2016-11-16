#ifndef FVIGNOREDVERSIONS_H
#define FVIGNOREDVERSIONS_H

#include <QObject>

class FVIgnoredVersions : public QObject
{
	Q_OBJECT

public:
	static bool VersionIsIgnored( QString targetVersion, QString applicationVersion );
	static void IgnoreVersion( QString targetVersion, QString applicationVersion );
	
private:
	explicit FVIgnoredVersions(QObject *parent = 0);
	
};

#endif // FVIGNOREDVERSIONS_H
