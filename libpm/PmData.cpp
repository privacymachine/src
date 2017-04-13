#include "PmData.h"
#include "utils.h"

#include <QFile>
#include <QDir>
#include <QTextStream>

PmData::PmData():
  installDirPath_("")
{
  vboxCommand_ = determineVBoxCommand();
}

QString PmData::determineVBoxCommand()
{
  QString vboxCommand;
  if (RunningOnWindows())
  {
    // We can't use QSettings-Registry-Methods here because we are running under WOW64
    // so we get it from the environment variable 'VBOX_INSTALL_PATH' or 'VBOX_MSI_INSTALL_PATH'
    vboxCommand = QProcessEnvironment::systemEnvironment().value("VBOX_INSTALL_PATH", "C:\\VirtualBoxDir_NOTFOUND\\");
    if (vboxCommand.contains("NOTFOUND"))
    {
      vboxCommand = QProcessEnvironment::systemEnvironment().value("VBOX_MSI_INSTALL_PATH", "C:\\VirtualBoxDir_NOTFOUND\\");
    }
    vboxCommand += "VBoxManage.exe";
  }
  else
  {
    // on Linux it is usually on the path
    vboxCommand = "vboxmanage";
  }

  return vboxCommand;
}

