/*==============================================================================
        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
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

#include "utils.h"
#include "PmData.h"
#include "PmManager.h"
#include "SystemConfig.h"
#include "UserConfig.h"
#include "VmMaskCurrentConfig.h"
#include "VmMaskUserConfig.h"
#include "PmVersion.h"

#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSet>


PmManager::PmManager() :
  configUser_(NULL),
  configSystem_(NULL),
  baseDiskConfigIsValid_(false),
  firstFreeLocalPort_(4242),
  firstStart_(false)
{
}

PmManager::~PmManager()
{
  // Delete all VmMasks
  while (!vmMaskData_.isEmpty())
  {
    VmMaskData* lastVmMask = vmMaskData_.takeLast();

    if (lastVmMask->Instance)
    {
      if (lastVmMask->Instance->getConfig())
        delete lastVmMask->Instance->getConfig();

      lastVmMask->Instance.clear();
    }

    // lastVmMask->UserConfig is deleted in the destructor of configUser_  (some lines below)
    /* if (lastVmMask->UserConfig)
        delete lastVmMask->UserConfig;
    */

    if (lastVmMask->StaticConfig)
    {
      delete lastVmMask->StaticConfig;
    }
    delete lastVmMask;
  }

  // Delete System/User-Config
  if (configUser_)
  {
    delete configUser_;
    configUser_ = NULL;
  }
  if (configSystem_)
  {
    delete configSystem_;
    configSystem_ = NULL;
  }
}

QString PmManager::getBaseDiskDirectoryPath()
{
  return configSystem_->getBaseDiskPath();
}

PmVersion PmManager::getBaseDiskVersion()
{
  return configSystem_->getBaseDiskVersion();
}

QString PmManager::baseDiskWithPath()
{
  return configSystem_->getBaseDiskPath() + "/" + configSystem_->getBaseDiskName();
}

bool PmManager::initConfiguration(const QString& parPmInstallPath, const QString& parVboxDefaultMachineFolder)
{
  pmInstallDir_ = parPmInstallPath;
  vboxDefaultMachineFolder_ = parVboxDefaultMachineFolder;

  // determine the configuration directory (os dependend)
  pmConfigDir_ = getPmConfigQDir();

  QString pmUserConfigFile = pmConfigDir_.path() + "/" + constPmUserConfigFileName;
  firstStart_ = !QFile::exists(pmUserConfigFile);

  if(firstStart_)
  {
    ILOG("Prepare the initial configuration at the location: " + pmConfigDir_.path());
    if (!pmConfigDir_.mkpath("."))
    {
      IERR("Error creating initial config dir: " + pmConfigDir_.path());
      return false;
    }

    // Choose the inital Confiuration based on the locale name (default is 'en')

    QString language_country = QLocale::system().name(); // QLocale::name() has the format: language_country i.e: en_US
    QString language = language_country.split('_').first();
    if (language != "en")
    {
      language = "en"; // TODO: @bernhard: hardcode to english because german tranlation is incomplete
    }
    QString templateConfigFile = pmInstallDir_ + "/conf/" + "PrivacyMachine_Example_" + language + ".ini";

    if ( !QFile::copy(templateConfigFile, pmUserConfigFile) )
    {
      QString systemError = getLastErrorMsg();
      IERR("Error copy config file '" + templateConfigFile + "' to '" + pmUserConfigFile + "': " + systemError);
      return false;
    }

    // create an empty vpn-config so the user has a start point
    QString exampleVpnPath = pmConfigDir_.path() + "/vpn/Autistici";
    QDir exampleVpnDir(exampleVpnPath);
    exampleVpnDir.mkpath("."); // ignore errors here
  }

  // Internal Configuration

  QString pmInternalConfigFile = pmConfigDir_.path() + "/" + constPmInternalConfigFileName;
  configSystem_ = new SystemConfig(pmInternalConfigFile);
  configSystem_->readFromFileOrSetDefaults();
  // always set the Binary Version, Name and Path just to be sure
  configSystem_->setBinaryVersion( QApplication::applicationVersion() );
  configSystem_->setBinaryName( QApplication::applicationName() );
  configSystem_->setBinaryPath(PmData::getInstance().getInstallDirPath());

  if(firstStart_)
  {
    // Create the path for BaseDisk
    QString baseDiskPath = QDir::toNativeSeparators(pmConfigDir_.path() + "/BaseDisk");
    configSystem_->setBaseDiskPath(baseDiskPath);

    QDir baseDiskDir(configSystem_->getBaseDiskPath());
    if (!baseDiskDir.exists())
    {
      if (!baseDiskDir.mkpath("."))
      {
        IERR("error creating the initial BaseDisk directory: " + configSystem_->getBaseDiskPath());
      }
    }
    if (!configSystem_->write())
    {
      IERR("Error saving initial internal configuration: " + pmInternalConfigFile);
      return false;
    }
  }
  return true;
}

bool PmManager::readConfiguration()
{
  QString pmUserConfigFile = pmConfigDir_.path() + "/" + constPmUserConfigFileName;

  configUser_ = new UserConfig(pmUserConfigFile, pmConfigDir_.path(), pmInstallDir_);
  if ( !configUser_->readFromFile() )
  {
    return false;
  }

  return true;
}

bool PmManager::validateConfiguration()
{
  if (isBaseDiskAvailable())
  {
    // read BaseDisk_Z_capabilities.json
    QString json_val;
    QFile json_file;
    json_file.setFileName(configSystem_->getBaseDiskPath() + "/" + configSystem_->getBaseDiskName() + "_capabilities.json");
    if ( !json_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      IERR("Error while opening " + configSystem_->getBaseDiskPath() + "/" + configSystem_->getBaseDiskName() + "_capabilities.json");
      return false;
    }
    json_val = json_file.readAll();
    json_file.close();
    QJsonDocument json_doc = QJsonDocument::fromJson(json_val.toUtf8());
    baseDiskCapabilities_ = json_doc.object();

  /*
  // ################################# example for JSON  useage #######################################
  // JSON types:
  //         Constant          Value       Description
  //      QJsonValue::Null      0x0     A Null value
  //      QJsonValue::Bool      0x1     A boolean value. Use toBool() to convert to a bool.
  //      QJsonValue::Double    0x2     A double. Use toDouble() to convert to a double.
  //      QJsonValue::String    0x3     A string. Use toString() to convert to a QString.
  //      QJsonValue::Array     0x4     An array. Use toArray() to convert to a QJsonArray.
  //      QJsonValue::Object    0x5     An object. Use toObject() to convert to a QJsonObject.
  //      QJsonValue::Undefined	0x80    The value is undefined. This is usually returned as an error condition, when trying to read an out of bounds value in an array or a non existent key in an object.

  QStringList json_keys = baseDiskCapabilities_.keys();
  ILOG("\n\ncapabilities.json contains the following keys and types:");
  for (int i = 0; i < json_keys.size(); ++i)
  {
    switch(baseDiskCapabilities_[json_keys.at(i)].type())
    {
      case QJsonValue::String:
        ILOG(json_keys.at(i)+"   String");
        break;
      case QJsonValue::Double:
        ILOG(json_keys.at(i)+"   Double");
        break;
      case QJsonValue::Array:
        ILOG(json_keys.at(i)+"   Array");
        break;
      case QJsonValue::Object:
        ILOG(json_keys.at(i)+"   Object");
        break;
      default:
        ILOG(json_keys.at(i)+"   something else");
    }
  }
  ILOG("\nExample: ssh public key:\n");
  QJsonObject ssh_keys = baseDiskCapabilities_["ssh_keys"].toObject();
  ILOG("keys of baseDiskCapabilities[\"ssh_keys\"]:");
  for (int i = 0; i < ssh_keys.keys().size(); ++i)
  {
    switch(ssh_keys[ssh_keys.keys().at(i)].type())
    {
      case QJsonValue::String:
        ILOG(ssh_keys.keys().at(i)+"   String");
        break;
      case QJsonValue::Double:
        ILOG(ssh_keys.keys().at(i)+"   Double");
        break;
      case QJsonValue::Array:
        ILOG(ssh_keys.keys().at(i)+"   Array");
        break;
      case QJsonValue::Object:
        ILOG(ssh_keys.keys().at(i)+"   Object");
        break;
      default:
        ILOG(ssh_keys.keys().at(i)+"   something else");
    }
  }
  ILOG("\nBaseDisk_pub:");
  ILOG(ssh_keys["BaseDisk_pub"].toString());
  // also possible: baseDiskCapabilities["ssh_keys"].toObject()["BaseDisk_pub"].toString()
  ILOG("\n\n");
  //##################################################################################################
  */


    // The UserConfig can only be validated if we have the BaseDisk-Capabilities
    if (!configUser_->setDefaultsAndValidateConfiguration(baseDiskCapabilities_))
      return false;
  }
  else
  {
    // we cannot finish the validation without BaseDisk because we need to know i.e. the number of fonts available
    return false;
  }

  // we can now mark the config as valid
  baseDiskConfigIsValid_ = true;

  return true;
}

bool PmManager::isBaseDiskAvailable()
{
  if (configSystem_ == NULL)
    return false;

  // When ComponentMajor set to zero it's used as marker for 'no basedisk available'
  if (configSystem_->getBaseDiskVersion().getComponentMajor() != 0)
    return true;

  return false;
}

bool PmManager::allVmMasksExist()
{
  ILOG("Check if all VmMasks exists");
  QString allOutput;
  QStringList args;

  foreach( VmMaskData* vmMask, vmMaskData_)
  {
    // set the timeout to 10s because we get into real troubles if we have no correct information of the existance of a VM
    args.clear();
    args.append("showvminfo");    
    args.append(vmMask->UserConfig->getVmName());
    if( !ExecShort( PmData::getInstance().getVBoxCommand(), args, &allOutput, true, 10, false) )
      return false;
  }
  return true;
}

bool PmManager::saveConfiguredVmMasks()
{
  QStringList userConfiguredVmMasks;

  foreach (VmMaskData* vmMask, vmMaskData_)
    userConfiguredVmMasks.append(vmMask->UserConfig->getVmName());

  configSystem_->setConfiguredVmMaskNames(userConfiguredVmMasks);
  return configSystem_->write();
}


bool PmManager::vmMaskRegenerationNecessary()
{
  if(!allVmMasksExist())
  {
    ILOG("Some configured VM-Masks needs to be generated");
    return true;
  }

  // TODO: olaf: when imlemented flash or other Mask breaking configuration check for that here too

  QSet<QString> oldConfiguredVmMasks = QSet<QString>::fromList(configSystem_->getConfiguredVmMaskNames());
  QSet<QString> currentConfiguredVmMasks;

  foreach (VmMaskData* vmMask, vmMaskData_)
    currentConfiguredVmMasks.insert(vmMask->UserConfig->getVmName());

  if( oldConfiguredVmMasks != currentConfiguredVmMasks)
  {
    ILOG("The configuration changed for " + QString::number(currentConfiguredVmMasks.size() - oldConfiguredVmMasks.size()) + " VM-Masks -> regeneration is necessary")
    return true;
  }

  return false;
}

bool PmManager::createCommandsToCreateVmMask( VmMaskData* parVmMask,
                                              QList<PmCommand*>& parCmdList)
{  
  // Take care: 'parVmMask->Instance' is not initialialized here!

  PmCommand* curCmd;
  QStringList args;
  int sshPort = parVmMask->StaticConfig->SshPort;
  QString vmMaskFullName = parVmMask->UserConfig->getFullName();
  QString vboxManageCommand = PmData::getInstance().getVBoxCommand();
  QString vmName = parVmMask->UserConfig->getVmName();
  QString desc;

  desc = "Create a new VM for VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("createvm");
  args.append("--name");
  args.append(vmName);
  args.append("--ostype");
  args.append("Debian_64");
  args.append("--register");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Parametrize general properties of VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("modifyvm");
  args.append(vmName);
  args.append("--memory");
  args.append("1024");
  args.append("--vram");
  args.append("9");
  args.append("--accelerate3d");
  args.append("off");
  args.append("--bioslogofadein");
  args.append("off");
  args.append("--biosbootmenu");
  args.append("disabled");
  #ifndef PM_WINDOWS
    // --audio none|null|oss|alsa|pulse
    args.append("--audio");
    args.append("pulse");
  #endif
  args.append("--audiocontroller");
  args.append("ac97");
  args.append("--clipboard");
  args.append("bidirectional");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Add an harddisk controller for VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("storagectl");
  args.append(vmName);
  args.append("--name");
  args.append("SATA");
  args.append("--add");
  args.append("sata");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Add the BaseDisk for VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("storageattach");
  args.append(vmName);
  args.append("--storagectl");
  args.append("SATA");
  args.append("--port");
  args.append("0");
  args.append("--type");
  args.append("hdd");
  args.append("--medium");
  args.append(configSystem_->getBaseDiskPath() + "/" + configSystem_->getBaseDiskName() + ".vmdk");
  args.append("--mtype");
  args.append("immutable");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Configure Portforwarding for VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("--nologo");
  args.append("modifyvm");
  args.append(vmName);
  args.append("--natpf1");
  QString natRule = "guestSSH,tcp," + QString(constLocalIp) + ",";  // ssh-server is only accessible from local machine
  natRule += QString::number(sshPort);                              // host port
  natRule += ",,22";                                                // guest port (default ssh port)
  args.append(natRule);
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Enable VRDP for VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("modifyvm");
  args.append(vmName);
  args.append("--vrde");
  args.append("on");
  args.append("--vrdeport");
  args.append(QString::number(parVmMask->StaticConfig->RdpPort));
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Start the VM-Mask: '" + vmMaskFullName + "'";
  args.clear();
  args.append("startvm");
  args.append(vmName);
  args.append("--type");
  args.append("headless");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  // wait 1 second because VBoxSRV must start before VBoxManage is possible
  curCmd = new PmCommand(1000);
  curCmd->setDescription("Wait till VBoxSVC ist started");
  curCmd->setExecutionCosts(50);
  parCmdList.append(curCmd);

  // Create a special command for BootUpDetection (regulary check the Size of the Display via a Screenshot)
  desc = "Wait until system-boot is done for VM-Mask '" + vmMaskFullName + "'";
  args.clear();
  args.append("controlvm");
  args.append(vmName);
  args.append("screenshotpng");  
  QString tmpPngFile = pmConfigDir_.path() + "/logs/tmpBootUpScreen.png";
  args.append(tmpPngFile);
  curCmd = new PmCommand( vboxManageCommand, args, tmpPngFile, desc );
  curCmd->setExecutionCosts(configSystem_->getMachineBootUpTime());
  parCmdList.append(curCmd);

  // Poll via a ssh-command until VM listens on SSH
  desc = "Waiting until VM for VM-Mask " + vmMaskFullName + "' is restored.";
  curCmd = genSshCmd( "echo Hello VmMask!", sshPort);
  curCmd->setType( pollingShellCommand );
  curCmd->setTimeoutMilliseconds( 500 );
  curCmd->setRetries( 5 );
  curCmd->setDescription( desc );
  curCmd->setExecutionCosts( 100 );
  parCmdList.append(curCmd);

  desc = "Set password for liveuser in VM-Mask " + vmMaskFullName;
  curCmd = genSshCmd( "usermod -p $(echo " + QString(constRootPwd) + " | openssl passwd -1 -stdin) " + QString(constLiveUser), sshPort);
  curCmd->setType( pollingShellCommand );
  curCmd->setTimeoutMilliseconds( 500 );
  curCmd->setRetries( 5 );
  curCmd->setDescription( desc );
  curCmd->setExecutionCosts( 50 );
  parCmdList.append(curCmd);

/**  // Copy folder pm to VM
//  TODO: At startup: ensure that we are at the location of the executable
//  TODO: On Windows: the executable flag gets lost, and scripts have the wrong lineendings
//  args.clear();
//  curCmd = GetPmCommandForScp2VM("liveuser",constVmIp,QString::number(parCurInstance->getConfig()->sshPort),constRootPw,"../../packaging/BaseDisk/pm_files/pm","/");
//  curCmd->setDescription("Copy obfuscation-scripts to VM-Mask '" + parCurInstance->getConfig()->FullName + "'");
//  curCmd->setCosts(configSystem_->getCopyScriptsPerSshTime());
//  parCmdList.append(curCmd);
*/

  desc = "Copy liveusers home directory to VM-Mask '" + vmMaskFullName + "'";
#ifdef USE_OUTSIDE_FIREFOX_PROFILE
  curCmd = GetPmCommandForScp2VM("liveuser",constVmIp,QString::number(parCurInstance->getConfig()->sshPort),constRootPw,"pm_data/VM_ROOTFS/home/liveuser","/home/");
  curCmd->setDescription( desc );
  curCmd->setCosts(200);
  parCmdList.append(curCmd);
#endif

  desc = "Set X-keybord layout to DE in VM-Mask: '" + vmMaskFullName + "'";
  // TODO: move keyboard-layout to the configuration
  curCmd = genSshCmd( "/pm/scripts/set_keybord_layout_to_de.sh", sshPort);
  curCmd->setDescription( desc );
  curCmd->setExecutionCosts( 50 );
  parCmdList.append(curCmd);

  desc = "Create a Snapshot for VM-Mask '" + vmMaskFullName + "'";
  args.clear();
  args.append("snapshot");
  args.append(vmName);
  args.append("take");
  args.append(constSnapshotName);
  if (!RunningOnWindows()) args.append("--pause");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  curCmd->setDescription( desc );
  parCmdList.append(curCmd);

  // Poll until snapshot has been created.
  // We try to list the snapshot's VM info, which should only succeed once it has been created.
  desc = "Waiting until snapshot for VM-Mask " + vmMaskFullName + "' has been created.";
  args.clear();
  args.append("snapshot");
  args.append(vmName);
  args.append("showvminfo");
  args.append(constSnapshotName);
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc, true);
  curCmd->setType( pollingShellCommand );
  curCmd->setTimeoutMilliseconds( 1000 );
  curCmd->setRetries( 100 );
  curCmd->setExecutionCosts( 100 );
  parCmdList.append(curCmd);

  desc = "Poweroff VM for VM-Mask '" + vmMaskFullName + "'";
  args.clear();
  args.append("controlvm");
  args.append(vmName);
  args.append("poweroff");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Waiting until VM for VmMask " + vmMaskFullName + "' is powered off.";
  // We list the virtualbox-command 'showvminfo' and look for 'VMState="poweroff"'
  args.clear();
  args.append("showvminfo");
  args.append("--machinereadable"); // easier to parse and hopefully more stable
  args.append(vmName);
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc, true);
  curCmd->setRegexPattern("VMState=\"poweroff\"");
  curCmd->setType( pollingShellCommand );
  curCmd->setTimeoutMilliseconds( 1000 );
  curCmd->setRetries( 100 );
  curCmd->setExecutionCosts( 100 );
  parCmdList.append(curCmd);

  desc = "Restore the Snapshot for VmMask '" + vmMaskFullName + "'";
  args.clear();
  args.append("snapshot");
  args.append(vmName);
  args.append("restore");
  args.append("UpAndRunning");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  return true;
}

void PmManager::initAllVmMaskData()
{
  foreach(VmMaskUserConfig* userVmMaskConfig, configUser_->getConfiguredVmMasks())
  {
    VmMaskData* newVmMaskData = new VmMaskData();
    newVmMaskData->UserConfig = userVmMaskConfig;

    // Static Config does not change after the Users closes and recreates a VmMask
    newVmMaskData->StaticConfig = new VmMaskStaticConfig();

    // Assign a unique RDP Port and Hostname
    // For assigning unique ports we need an index of the configured VM-Masks
    newVmMaskData->StaticConfig->Name = newVmMaskData->UserConfig->getName();
    newVmMaskData->StaticConfig->VmName = newVmMaskData->UserConfig->getVmName();
    newVmMaskData->StaticConfig->FullName = newVmMaskData->UserConfig->getFullName();
    newVmMaskData->StaticConfig->RdpPort = firstFreeLocalPort_ + userVmMaskConfig->getVmMaskId() * 2;

    // We take the next local Port for the ssh connection
    newVmMaskData->StaticConfig->SshPort = newVmMaskData->StaticConfig->RdpPort + 1;

    // No Instance created till now
    newVmMaskData->Instance.clear();

    // Store the data
    vmMaskData_.push_back(newVmMaskData);
  }
}

bool PmManager::createCommandsToCreateAllVmMasks(QList<PmCommand*>& parCmdList)
{
  foreach (VmMaskData* vmMask, vmMaskData_)
  {
    if(!createCommandsToCreateVmMask(vmMask, parCmdList))
      return false;
  }

  return true;
}

bool PmManager::createCommandsToCloseVmMask(QString parVmName,
                                            QString parVmMaskFullName,
                                            QList<PmCommand*>& parCmdList)
{
  QStringList args;
  QString desc;
  PmCommand *curCmd;
  QString vboxManageCommand = PmData::getInstance().getVBoxCommand();

  desc = "Poweroff VM-Mask '" + parVmMaskFullName + "'";
  args.clear();
  args.append("--nologo");
  args.append("controlvm");
  args.append(parVmName);
  args.append("poweroff");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  desc = "Restore Snapshot of VM-Mask '" + parVmMaskFullName + "'";
  args.clear();
  args.append("--nologo");
  args.append("snapshot");
  args.append(parVmName);
  args.append("restorecurrent");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  parCmdList.append(curCmd);

  return true;
}

bool PmManager::createCommandsToCloseAllVmMasks(QList<PmCommand*>& parCmdList)
{
  foreach( VmMaskData* vmMask, vmMaskData_ )
  {
    // before closing copy VPN logs
    PmCommand* pCurrentCommand = NULL;
    pCurrentCommand = GetPmCommandForScp2Host(constRootUser, constLocalIp, QString::number( vmMask->StaticConfig->SshPort ), constRootPwd,
                                              pmConfigDir_.path() + "/logs/vmMask_" + vmMask->UserConfig->getName() + "_vpnLog.txt",
                                              "/var/log/openvpn.log");
    pCurrentCommand->setDescription("Copy vpn logs of VM-Mask " + vmMask->UserConfig->getName());
    pCurrentCommand->setTimeoutMilliseconds(2000);
    parCmdList.append( pCurrentCommand );

    if( !createCommandsToCloseVmMask(vmMask->StaticConfig->VmName, vmMask->UserConfig->getFullName(), parCmdList) )
    {
      return false;
    }
  }
  return true;
}


bool PmManager::createCommandsToStartVmMask(int parVmMaskId,
                                            QList<PmCommand*>& parCmdList)
{
  if(parVmMaskId >= vmMaskData_.count())
  {
    IERR("parIndexVmMask is out of bounds: " + QString::number(parVmMaskId));
    return false;
  }

  VmMaskData* vmMask = vmMaskData_[parVmMaskId];

  // delete the old Instance
  if (vmMask->Instance)
  {
    if (vmMask->Instance->getConfig())
    {
      delete vmMask->Instance->getConfig();
    }
    vmMask->Instance.clear();
  }

  // generate a new random fingerprint
  VmMaskCurrentConfig* newCurrentConfig = vmMask->UserConfig->diceNewVmMaskConfig(vmMask->StaticConfig);
  if (newCurrentConfig == 0)
  {
    IERR("Error generating a new fingerprint (current config)");
    return false;
  }

  vmMask->Instance = QSharedPointer<VmMaskInstance>(new VmMaskInstance(newCurrentConfig, parVmMaskId));
  QString vmName = vmMask->Instance->getConfig()->getVmName();
  QString vmMaskFullName = vmMask->Instance->getConfig()->getFullName();
  QString vboxManageCommand = PmData::getInstance().getVBoxCommand();
  int sshPort = vmMask->Instance->getConfig()->getSshPort();
  QString desc;

  // We might still have an abandoned VM with the same name running from a previous session.
  // To be on the safe side, we attempt to stop it and to reset it to the Snapshot "UpAndRunning".
  desc = "Attempt to power off abandoned VM-Clone for VM-Mask '" + vmMaskFullName + "'";
  QStringList args;
  PmCommand* curCmd;
  args.clear();
  args.append("controlvm");
  args.append(vmName);
  args.append("poweroff");
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  // If the VM actually is not running (which would give us an error), that is fine for us as well.
  curCmd->setIgnoreErrors( true );
  parCmdList.append(curCmd);

  desc = "Attempt to reset VM for VM-Mask '" + vmMaskFullName + "' to snapshot'" + constSnapshotName + "'";
  args.clear();
  args.append("snapshot");
  args.append(vmName);
  args.append("restore");
  args.append(constSnapshotName);
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  // Ignore this error because on normal exit the snapshot is already restored
  // -> not possible to restore again
  curCmd->setIgnoreErrors( true );
  parCmdList.append(curCmd);

  desc = "Startup VM for VM-Mask '" + vmMaskFullName + "'";
  args.clear();
  args.append("startvm");
  args.append(vmName);
  args.append("--type");
  args.append("headless"); // 'gui' or 'headless'
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  curCmd->setExecutionCosts(configSystem_->getMachineRestoreTime());
  parCmdList.append(curCmd);

  // Poll until VM listens on SSH
  desc = "Waiting until VM for VM-Mask " + vmMaskFullName + "' is restored.";
  curCmd = genSshCmd("echo Hello VM-Mask!", sshPort);
  curCmd->setType( pollingShellCommand );
  curCmd->setTimeoutMilliseconds( 500 );
  curCmd->setRetries( 5 );
  curCmd->setDescription( desc );
  curCmd->setExecutionCosts( 100 );
  parCmdList.append(curCmd);

  // Copy liveusers home directory
#ifdef USE_OUTSIDE_FIREFOX_PROFILE
  curCmd = GetPmCommandForScp2VM( constLiveUser, constVmIp,QString::number(vmMask->Instance->getConfig()->sshPort), constLiveUserPwd,
                                      "pm_data/VM_ROOTFS/home/liveuser",
                                      "/home/");
  curCmd->setDescription("Copy liveusers home directory to VM-Mask '" + vmMask->Instance->getConfig()->FullName + "'");
  curCmd->setCosts(200);
  parCmdList.append(curCmd);
#endif

  // Obfuscate fonts:
  foreach(const QString &font, vmMask->Instance->getConfig()->getFonts())
  {
    desc = "Installing font '" + font + "'";
    curCmd = genSshCmd("mv /pm/fonts/" + font + " /usr/local/share/fonts", sshPort);
    curCmd->setDescription(desc);
    parCmdList.append(curCmd);
  }
  desc = "Finish font obfuscation";
  curCmd = genSshCmd("fc-cache -f", sshPort);
  curCmd->setDescription(desc);
  curCmd->setExecutionCosts(100);
  parCmdList.append(curCmd);

  desc = "Obfuscating the time zone";
  curCmd = genSshCmd("timedatectl set-timezone " + vmMask->Instance->getConfig()->getTimeZone(), sshPort);
  curCmd->setDescription(desc);
  parCmdList.append(curCmd);

  desc = "Obfuscating the locale";
  QString vmLocale = vmMask->Instance->getConfig()->getLocale();
  curCmd = genSshCmd("localectl set-locale LANG=" + vmLocale + " LANGUAGE=" + vmLocale, sshPort);
  curCmd->setDescription(desc);
  parCmdList.append(curCmd);

  // configure the browser: Obfuscation via Plugin-Randomisation
  addCommandsToConfigureBrowser(vmMask->Instance, parCmdList);

  // Obfuscate dns-servers TODO: olaf: find a way for VPN via /etc/resolvconf/resolv.conf.d/tail or other openvpn parameters
  const VpnConfig& vpnConfig = vmMask->Instance->getConfig()->getVpnConfig();
  if( vpnConfig.VpnType != "OpenVPN" &&
      vpnConfig.VpnType != "VPNGate" )
  {
    desc = "Cleanup DNS-Servers";
    curCmd = genSshCmd("rm /etc/resolv.conf && touch /etc/resolv.conf", sshPort);
    curCmd->setDescription(desc);
    parCmdList.append(curCmd);
    foreach(const QString &dns, vmMask->Instance->getConfig()->getDnsServers())
    {
      desc = "Installing DNS-Server " + dns;
      curCmd = genSshCmd("echo nameserver " + dns + " >> /etc/resolv.conf", sshPort);
      curCmd->setDescription(desc);
      parCmdList.append(curCmd);
    }
  }

  if( vpnConfig.VpnType == "OpenVPN" ||
      vpnConfig.VpnType == "VPNGate" )
  {
    PmVpnHelper vpn(vmMask->Instance);

    if (!vpn.addCmdToInitVPN(parCmdList))
      return false;

    // Prepare systemd configuration which depends on the browser
    if (!vpn.addCmdToInitSystemD(parCmdList, vmMask->Instance->getConfig()->getBrowser()))
      return false;
  }
  else // no vpn
  {
    // If there is no OpenVPN connection configured, firefox can be started straightaway without waiting for the
    // connection to be established:

    // start the browser
    addCommandsToStartBrowserService(vmMask->Instance, parCmdList);
  }

  return true;
}

void PmManager::addCommandsToConfigureBrowser(QSharedPointer<VmMaskInstance>& parVmMaskInstance, QList<PmCommand*>& parCmdList)
{
  if( parVmMaskInstance->getConfig()->getBrowser() == "firefox" )
  {
    PmCommand *curCmd;
    QString desc = "Obfuscating firefox addons: remove random addons";
    curCmd = genSshCmd( "/pm/firefox_addons/rmAddons.py  /home/liveuser/.mozilla/firefox/al5g3l76.default/extensions/  "
                            "/pm/firefox_addons/installed_addons/",
                            parVmMaskInstance->getConfig()->getSshPort());
    curCmd->setDescription(desc);
    curCmd->setExecutionCosts(100);
    parCmdList.append(curCmd);
  }
  // TODO:  else if( pVmMaskInstance->getConfig()->browser ==...)
}

void PmManager::addCommandsToStartBrowserService(QSharedPointer<VmMaskInstance>& parVmMaskInstance, QList<PmCommand*>& parCmdList)
{
  PmCommand *pCurrentCommand;
  QString desc = "Start " + parVmMaskInstance->getConfig()->getBrowser();
  pCurrentCommand = genSshCmd("systemctl start " + parVmMaskInstance->getConfig()->getBrowser() + ".service", parVmMaskInstance->getConfig()->getSshPort());
  pCurrentCommand->setDescription(desc);
  pCurrentCommand->setExecutionCosts(10);
  parCmdList.append(pCurrentCommand);
}

bool PmManager::createCommandsToCleanupAllVirtualMachines(QList<PmCommand*>& parCmdList)
{
  QStringList args;
  PmCommand *curCmd;
  QString vboxManageCommand = PmData::getInstance().getVBoxCommand();
  QString desc;

  QDir vboxDir(vboxDefaultMachineFolder_);

  if (!vboxDir.exists())
  {
    IERR("The 'Default machine folder' of VirtualBox does not exist: '" + vboxDefaultMachineFolder_ + "'");
    return false;
  }

  vboxDir.setNameFilters( QStringList( QString(constPmVmMaskPrefix) + "*" ) );
  QStringList foundVmMasks = vboxDir.entryList( QDir::Dirs | QDir::Readable | QDir::CaseSensitive );

  // Add folders/VmMask-names to the list which are in the way for new VmMask creation
  foreach (VmMaskData* vmMask, vmMaskData_)
  {
    if (!foundVmMasks.contains(vmMask->UserConfig->getVmName()))
      foundVmMasks.append(vmMask->UserConfig->getVmName());
  }

  foreach (const QString &oldVmMask, foundVmMasks)
  {
    // To be on the safe side, we attempt to stop the VM
    args.clear();
    args.append("controlvm");
    args.append( oldVmMask );
    args.append("poweroff");
    curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
    curCmd->setDescription( "Attempt to power off old VM-Mask '" + oldVmMask + "'" );
    curCmd->setIgnoreErrors( true );
    parCmdList.append(curCmd);

    // Delete the VM
    args.clear();
    args.append("unregistervm");
    args.append( oldVmMask );
    args.append("--delete");
    curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
    curCmd->setDescription( "Attempt to delete the old VM-Mask '" + oldVmMask + "'" );
    curCmd->setIgnoreErrors( true );
    parCmdList.append(curCmd);

    // virtualbox needs some break
    curCmd = new PmCommand(200);
    curCmd->setDescription( "Safety wait for virtualbox after remove of vm");
    parCmdList.append(curCmd);

    // Delete the folder
    curCmd = new PmCommand(vboxDefaultMachineFolder_ + "/" + oldVmMask);
    curCmd->setDescription( "Attempt to delete the old VM-Mask folder '" + vboxDefaultMachineFolder_ + "/" + oldVmMask + "'" );
    curCmd->setIgnoreErrors( true );
    parCmdList.append(curCmd);
  }


  // Remove all BaseDisk's from the MediaManager
  QDir baseDiskDir(configSystem_->getBaseDiskPath());

  if (baseDiskDir.exists())
  {
    baseDiskDir.setNameFilters( QStringList( "BaseDisk_*.vmdk" ) );
    QStringList foundBaseDisks = baseDiskDir.entryList( QDir::Files | QDir::Readable | QDir::CaseSensitive );

    foreach (const QString &oldBaseDisk, foundBaseDisks)
    {
      if (!oldBaseDisk.contains("_flat"))
      {
        args.clear();
        args.append("closemedium");
        args.append( configSystem_->getBaseDiskPath() + oldBaseDisk);
        curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
        curCmd->setDescription( "Attempt to delete the old BaseDisk '" + oldBaseDisk + "'" );
        curCmd->setIgnoreErrors( true );
        parCmdList.append(curCmd);
      }
    }
  }
  
  // All BaseDisks have the same id: remove it
  args.clear();
  args.append("closemedium");
  args.append( "0ac66e6e-2a04-4d75-9357-c5c3bc87dcf2" );
  curCmd = new PmCommand( vboxManageCommand, args, true, false, desc );
  curCmd->setDescription( "Attempt to delete the old BaseDisk by id");
  curCmd->setIgnoreErrors( true );
  parCmdList.append(curCmd);

  // virtualbox needs some break
  curCmd = new PmCommand(3000);
  curCmd->setDescription( "Safety wait for vboxmanage to organize itself");
  parCmdList.append(curCmd);

  return true;
}
