/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

                     Licensed under the EUPL, Version 1.1
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

#include <QApplication>
#include <QString>
#include <QDir>
#include <QLayout>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>

#include <string>
#include <iostream>
#include <algorithm>

#include "../libpm/utils.h"


#include <iostream>
using namespace std;

#ifdef IM_WINDOWS
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
  return 42;
}

/*
QString staticGlobalLogFileName;

// this function will be used by qDebug(), qWarning(), qCritical(), qFatal()
void logMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg)
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
  }

  // Also write to the Console
  fprintf(stderr, "%s\n", msg.toLatin1().data());

  QFile outFile(staticGlobalLogFileName);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
  {
    QTextStream ts(&outFile);
    ts << txt << endl;
  }

  if (type == QtFatalMsg) abort();
}


void checkLogfiles(QString executableAbsDirectory)
{
  staticGlobalLogFileName = "";

  #ifdef PM_WINDOWS
    // Use the directory where the exe is located in Windows-Mode
    staticGlobalLogFileName = executableAbsDirectory + "/";
  #else
    // Live-Mode
    staticGlobalLogFileName = "/tmp/";
  #endif

  staticGlobalLogFileName += "logs/";

  // Create the subdirectory logs
  if (!QDir(staticGlobalLogFileName).exists())
  {
    cout << "create the log directory: " << staticGlobalLogFileName.toUtf8().constData() << endl;
    if(!QDir().mkpath(staticGlobalLogFileName))
    {
      cerr << "Error creating log directory: " << staticGlobalLogFileName.toUtf8().constData() << endl;
      // for savety use a logfile within current dir
      staticGlobalLogFileName = "PrivacyMachine_ProblemReporter.txt";
      return;
    }
  }

  staticGlobalLogFileName += "PrivacyMachine_ProblemReporter.txt";

  // delete old one
  if (QFile::exists(staticGlobalLogFileName))
  {
    QFile::remove(staticGlobalLogFileName);
  }
}

int main(int argc, char *argv[])
{
  int retCode = -1;

  // Hide the console on windows if no args specified
  #ifdef PM_WINDOWS
    if (argc == 1)
      FreeConsole();
  #endif

  QString arg0 = QString(argv[0]);
  QFileInfo runningExecuteable(arg0);
  QString executableAbsDirectory = runningExecuteable.dir().absolutePath();
  checkLogfiles(executableAbsDirectory);

  QApplication app(argc, argv);
  // Get the path to the current running executable
  // (only works before we change the current path inside this application)
  QString pmInstallPath = app.applicationDirPath();

  // Enable Warning messages triggered by qDebug(), qWarning(), qCritical(), qFatal()
  qInstallMessageHandler(logMessages);

  QString startTime = currentTimeStampAsISO();
  ILOG("Starting up at " + startTime + " Version: " + constPrivacyMachineVersion);


  try // catch all exceptions
  {
    // init the translation of qt internal messages
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // init the translation of PrivacyMachine messages
    QTranslator pmTranslator;
    QString currentLanguage = "lang_" + QLocale::system().name();
    // TODO: read from PrivacyMachine.ini
    // Enable for Testing the german translation
    currentLanguage = "lang_de_DE";
    pmTranslator.load(currentLanguage);
    app.installTranslator(&pmTranslator);
    
    QCoreApplication::setApplicationName(QCoreApplication::translate("mainfunc", "ProblemReporter of the PrivacyMachine"));
    QCoreApplication::setApplicationVersion(constPrivacyMachineVersion);


    frmProblemReporter mainWindow(executableAbsDirectory);
    mainWindow.show();
  
    retCode = app.exec();
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
*/
