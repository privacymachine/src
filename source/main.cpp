/*==============================================================================
        Copyright (c) 2013-2014 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

 Licensed under the EUPL, Version 1.1 or - as soon they will be approved by the
     European Commission - subsequent versions of the EUPL (the "Licence");
        You may not use this work except in compliance with the Licence.
                  You may obtain a copy of the Licence at:
                        http://ec.europa.eu/idabc/eupl

 Unless required by applicable law or agreed to in writing, software distributed
              under the Licence is distributed on an "AS IS" basis,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
      See the Licence for the specific language governing permissions and
                        limitations under the Licence.
==============================================================================*/

#include <QtGui/QApplication>

#include <QString>
#include <QDir>
#include <QLayout>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>

#include <QProcess>
#include <QStringList>

#include <string>
#include <iostream>
#include <algorithm>

#include "utils.h"

#include "WindowMain.h"
#include "SystemConfig.h"

#include <iostream>
using namespace std;

#ifdef PM_WINDOWS
#include <Windows.h>
#endif


QString staticGlobalLogFileName;
void myMessageOutput(QtMsgType type, const char *msg);
void rotateLogfiles(QString processName);

int main(int argc, char *argv[])
{
  int retCode = -1;

  // Hide the console on windows if no args specified
#ifdef PM_WINDOWS
  if (argc == 1)
    FreeConsole();
#endif

#ifndef PM_WINDOWS
  // On Linux, this is needed for console output and for reading INI-Files
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

  // Initialize random seed (based on current time)
  QTime timeForRandomSeed = QTime::currentTime();
  qsrand((uint)timeForRandomSeed.msec());

  rotateLogfiles(QString(argv[0]));

  QApplication app(argc, argv);

  // Enable Warning messages triggered by qDebug(), qWarning(), qCritical(), qFatal()
  qInstallMsgHandler(myMessageOutput);

  QString startTime = currentTimeStampAsISO();
  ILOG("Starting up at " + startTime + " Version:" + constPrivacyMachineVersion);


  try // catching exceptions
  {
    // init the translation of qt internal messages
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // init the translation of PrivacyMachine messages
    QTranslator pmTranslator;
    pmTranslator.load("lang_" + QLocale::system().name());
    app.installTranslator(&pmTranslator);

    // Test i18n
    //QString msg = QCoreApplication::translate("mainfunc", "This is English!");
    //QMessageBox::warning(0,"Some Text", msg);

    WindowMain mainWindow;
    if (mainWindow.init())
    {
      mainWindow.show();
      retCode = app.exec();
    }
    else
    {
      retCode = -1;
    }
  }
  catch (std::exception& e)
  {
    IERR("error: " + QString::fromStdString(e.what()));
    return -1;
  }

  QString stopTime = currentTimeStampAsISO();
  ILOG("shutting down at " + stopTime);
  return retCode;
}

// this function will be used by qDebug(), qWarning(), qCritical(), qFatal()
void myMessageOutput(QtMsgType type, const char *msg)
{
  QString txt;
  switch (type)
  {
    case QtDebugMsg:
      txt = QString("%1").arg(msg);
      break;
    case QtWarningMsg:
      txt = QString("Warning: %1").arg(msg);
      break;
    case QtCriticalMsg:
      txt = QString("Critical: %1").arg(msg);
      break;
    case QtFatalMsg:
      txt = QString("Fatal: %1").arg(msg);
      abort();
  }

  // Also write to the Console
  fprintf(stderr, "%s\n", msg);

  QFile outFile(staticGlobalLogFileName);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Append))
  {
    QTextStream ts(&outFile);
    ts << txt << endl;
  }
}


void rotateLogfiles(QString processName)
{
  staticGlobalLogFileName = "";

  QFileInfo runningExe(processName);

  QString logMainDir = "";
  if (!RunningOnWindows() && processName.startsWith("/usr/"))
  {
    logMainDir = "/tmp/";
  }
  else if (runningExe.exists())
  {
    logMainDir = runningExe.absolutePath() + "/";
  }


  // Create subdir "logs"
  QDir logDir(logMainDir);
  if (logDir.exists())
  {
    logDir.mkdir(staticGlobalLogFileName + "logs");
  }

  staticGlobalLogFileName = logMainDir;
#ifdef PM_WINDOWS
  staticGlobalLogFileName += "logs/PrivacyMachine_Windows.log";
#else
  staticGlobalLogFileName += "logs/PrivacyMachine_Linux.log";
#endif

  // logrotate
  if (QFile::exists(staticGlobalLogFileName))
  {
    for (int i = 5; i >= 1; i--)
    {
      if (QFile::exists(staticGlobalLogFileName + "." + QString::number(i)))
      {
        // delete .5
        if (i == 5)
        {
          QFile::remove(staticGlobalLogFileName + "." + QString::number(i));
        }
        else
        {
          // rename .4 -> .5  ...
          for (int j = i; j >=1; j--)
            QFile::rename(staticGlobalLogFileName + "." + QString::number(j), staticGlobalLogFileName + "." + QString::number(j+1));

          break;
        }
      }
    }
    // rename to ".1"
    QFile::rename(staticGlobalLogFileName, staticGlobalLogFileName + ".1");
  }

}

