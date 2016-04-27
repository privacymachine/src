/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

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

#include <QProcess>
#include <QStringList>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <string>
#include <iostream>
#include <algorithm>

#include "utils.h"
#include "getMemorySize.h"
#include "WindowMain.h"
#include "SystemConfig.h"

#include <iostream>
using namespace std;

#ifdef PM_WINDOWS
  #include <Windows.h>
  #include <Winsock2.h>
#endif


QString staticGlobalLogFileName;
void logMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg);
void rotateLogfiles(QString processName);
bool checkSystemInstallation(QString parInstallPath);

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
  // TODO:check that!! QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

  // Initialize random seed (based on current time)
  QTime timeForRandomSeed = QTime::currentTime();
  qsrand((uint)timeForRandomSeed.msec());

  rotateLogfiles(QString(argv[0]));

  QApplication app(argc, argv);
  // Get the path to the current running executable
  // (only works before we change the current path inside this application)
  QString pmInstallPath = app.applicationDirPath();

  // We want the same look&feel on all Platforms
  //app.setStyle(QStyleFactory::create("windows"));


  // Enable Warning messages triggered by qDebug(), qWarning(), qCritical(), qFatal()
  qInstallMessageHandler(logMessages);

  QString startTime = currentTimeStampAsISO();
  ILOG("Starting up at " + startTime + " Version:" + constPrivacyMachineVersion);

#ifdef WIN32
    WSADATA wsaData;
    int err;
    err = WSAStartup(0x101, &wsaData);
    if (err != 0)
    {
        IERR("WSAStartup failed with error: " + QString::number(err));
        return -1;
    }
    // TODO: Needed? freerdp_register_addin_provider(freerdp_channels_load_static_addin_entry, 0);
#endif

  try // catching exceptions
  {  
    // init the translation of qt internal messages
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // init the translation of PrivacyMachine messages
    QTranslator pmTranslator;
    QString currentLanguage = "lang_" + QLocale::system().name();
    // Enable for Testing the german translation: TODO: configure somewhere
    currentLanguage = "lang_de_DE";
    pmTranslator.load(currentLanguage);
    app.installTranslator(&pmTranslator);

    QCoreApplication::setApplicationName(QCoreApplication::translate("mainfunc", "PrivacyMachine"));
    QCoreApplication::setApplicationVersion(constPrivacyMachineVersion);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("mainfunc", "This executeable launches the PrivacyMachine, a browser who protects your Privacy"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption optionLogSensitiveData("logsensitive", QCoreApplication::translate("mainfunc", "Enable logging of sensible data, i.e. passwords"));
    parser.addOption(optionLogSensitiveData);
    parser.process(app);

    // set the global variable to enable logging of sensitive data via ILOG_SENSITIVE
    globalSensitiveLoggingEnabed = parser.isSet(optionLogSensitiveData);

    // TODO: Check system configuration
    // Linux/Windows:
    //  o) vboxmanage is in the PATH and the correct version
    //  o) VirtualBox Extension Pack has to be installed
    //  o) VirtualBox can execute 64Bit Guests (VT-x/AMD-V is enabled in BIOS)
    //  o) enough free RAM and diskspace
    // Linux:
    //  o) installed: ssh-pass, ssh
    // Windows:
    //  o) plink.exe, pscp.exe
    if (!checkSystemInstallation(pmInstallPath))
    {
      return -1; // Messages to User are already presented
    }

    WindowMain mainWindow;
    if (mainWindow.init(pmInstallPath))
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


/// Cycles between log files so that previous sessions are not overwritten immediately.
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
  staticGlobalLogFileName += "logs/PrivacyMachineLog.txt";
#else
  staticGlobalLogFileName += "logs/PrivacyMachineLog.log";
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

bool checkSystemInstallation(QString parInstallPath)
{
  size_t memorySize = getMemorySize( );
  ILOG("Total Physical RAM size: " + QString::number(memorySize) + " Bytes");
  qint64 freeSpace = getFreeDiskSpace(parInstallPath);
  ILOG("Free space on disk containing '" + parInstallPath + "': " + QString::number(freeSpace) + " Bytes");

  QString vboxCommand;
  QString vboxVersion;
  QString vboxDefaultMachineFolder;
  bool vboxExtensionPackInstalled;
  if (!determineVirtualBoxInstallation(vboxCommand, vboxVersion, vboxDefaultMachineFolder, vboxExtensionPackInstalled))
  {
    // TODO: Show a nice message to the user
    return false;
  }

  return true;
}
