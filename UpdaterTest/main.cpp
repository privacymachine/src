#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

#include <fvupdater.h>

QSettings settings;

int main(int argc, char *argv[])
{

  QApplication a(argc, argv);
  QApplication::setApplicationName("UpdaterTest");
  // base disk version plus prepended marketing and API version
  QApplication::setApplicationVersion("1.0.0.2");
  QApplication::setOrganizationName("PrivacyMachine");
  QApplication::setOrganizationDomain("privacymachine.eu");

  FvUpdater::sharedUpdater()->SetFeedURL("file://localhost/media/samplePM/appcast_base-disk.xml");

  // Check for updates silently -- this will not block the initialization of
  // your application, just start a HTTP request and return immediately.
  FvUpdater::sharedUpdater()->CheckForUpdatesNotSilent();

  MainWindow w;
  w.show();

  return a.exec();
}
