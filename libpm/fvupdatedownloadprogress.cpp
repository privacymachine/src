#include "fvupdatedownloadprogress.h"

FvUpdateDownloadProgress::FvUpdateDownloadProgress(QWidget *parent)
	: QWidget(parent, Qt::SplashScreen)
{
	ui.setupUi(this);

	ui.progress->setValue(0);

}

FvUpdateDownloadProgress::~FvUpdateDownloadProgress()
{

}

void FvUpdateDownloadProgress::downloadProgress ( qint64 bytesReceived, qint64 bytesTotal )
{
  // HACK AL: * 90 in order to stop at 90%, which signals that something is still going on.
  // This "something" is, namely, unzipping the base disk, which takes a couple of minutes.
	ui.progress->setValue( ((float)bytesReceived / (float)bytesTotal) * 90 );
}

void FvUpdateDownloadProgress::close()
{
	this->deleteLater();
	QWidget::close();
}
