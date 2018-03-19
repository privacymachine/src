#include "PmData.h"
#include "utils.h"

#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>

PmData::PmData()
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

bool PmData::completelyFilled()
{
  if(installDirPath_ == "NO VALID ENTRY" ||
     vboxCommand_ == "NO VALID ENTRY" ||
     pmServerIp_ == "NO VALID ENTRY" ||
     baseDiskRootUser_ == "NO VALID ENTRY" ||
     baseDiskRootUserPassword_ == "NO VALID ENTRY" ||
     baseDiskLiveUser_ == "NO VALID ENTRY" ||
     baseDiskLiveUserPassword_ == "NO VALID ENTRY" ||
     vmMaskPrefix_ == "NO VALID ENTRY" ||
     vmPmMaskPrefix_ == "NO VALID ENTRY" ||
     vpnPrefix_ == "NO VALID ENTRY" ||
     vmSnapshotName_ == "NO VALID ENTRY" ||
     pmUserConfigFilePath_ == "NO VALID ENTRY" ||
     pmInternalConfigFilePath_ == "NO VALID ENTRY" ||
     configDirPath_ == "NO VALID ENTRY" )
    return false;

  else return true;
}

void PmData::log()
{
  ILOG_SENSITIVE("PmData:");
  ILOG_SENSITIVE( "installDirPath_ = " + installDirPath_ );
  ILOG_SENSITIVE( "vboxCommand_ = " + vboxCommand_ );
  ILOG_SENSITIVE( "pmServerIp_ = " + pmServerIp_ );
  ILOG_SENSITIVE( "baseDiskRootUser_ = " + baseDiskRootUser_ );
  ILOG_SENSITIVE( "baseDiskRootUserPassword_ = " + baseDiskRootUserPassword_ );
  ILOG_SENSITIVE( "baseDiskLiveUser_ = " + baseDiskLiveUser_ );
  ILOG_SENSITIVE( "baseDiskLiveUserPassword_ = " + baseDiskLiveUserPassword_ );
  ILOG_SENSITIVE( "vmMaskPrefix_ = " + vmMaskPrefix_ );
  ILOG_SENSITIVE( "vmPmMaskPrefix_ = " + vmPmMaskPrefix_ );
  ILOG_SENSITIVE( "vpnPrefix_ = " + vpnPrefix_ );
  ILOG_SENSITIVE( "vmSnapshotName_ = " + vmSnapshotName_ );
  ILOG_SENSITIVE( "pmUserConfigFilePath_ = " + pmUserConfigFilePath_ );
  ILOG_SENSITIVE( "pmInternalConfigFilePath_ = " + pmInternalConfigFilePath_ );
  ILOG_SENSITIVE( "configDirPath_ = " + configDirPath_ );
}
