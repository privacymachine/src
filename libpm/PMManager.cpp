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

#include "utils.h"
#include "PMManager.h"
#include "SystemConfig.h"
#include "UserConfig.h"

#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSet>


PMManager::PMManager() :
  configUser_(0),
  configSystem_(0),
  firstFreeLocalPort_(4242)
{

  if(!getAndCreateUserConfigDir(pmUserConfigDir_))
  {
    IERR("error getting user config dir");
  }

  QDir pmUserConfigDirectory(pmUserConfigDir_);
  firstStart_ =  !pmUserConfigDirectory.exists();
}

PMManager::~PMManager()
{
  while (!pm_.isEmpty()) delete pm_.takeLast();
  while (!pmConfigs.isEmpty()) delete pmConfigs.takeLast();
  if (configUser_) delete configUser_;
  configUser_ = NULL;
  if (configSystem_) delete configSystem_;
  configSystem_ = NULL;

}

QString PMManager::getBaseDiskDirectoryPath()
{
  return configSystem_->getBaseDiskPath();
  
}

QString PMManager::baseDiskWithPath()
{
  return configSystem_->getBaseDiskPath() + configSystem_->getBaseDiskName();
}

bool PMManager::init_1(QString parPmInstallPath, QString parVboxDefaultMachineFolder)
{
  pmInstallDir_ = parPmInstallPath;
  vboxDefaultMachineFolder_ = parVboxDefaultMachineFolder;

  QString pmUserConfigFile = pmUserConfigDir_ + "/PrivacyMachine.ini";

  bool firstStart = false;
  if( !QFile::exists(pmUserConfigFile) )
  {
    firstStart = true;
    ILOG("Initialize PrivacyMachine.ini");

    QString language_country = QLocale::system().name(); // QLocale::name() has the format: language_country i.e: en_US
    QString language = language_country.split('_').first();
    // TODO: Bernhard: hardcode to english because german tranlation is incomplete
    language = "en";
    QString templateConfigFile = pmInstallDir_ + "/conf/" + "PrivacyMachine_Example_" + language + ".ini";

    if (!QFile::copy(templateConfigFile, pmUserConfigFile))
    {
      QString systemError = getLastErrorMsg();
      IERR("Error copy config file '" + templateConfigFile + "' to '" + pmUserConfigFile + "': " + systemError);
      return false;
    }

    // create an empty vpn-config so the user has a start point
    QString exampleVpnPath = pmUserConfigDir_ + "/vpn/Autistici";
    QDir exampleVpnDir(exampleVpnPath);
    exampleVpnDir.mkpath("."); // ignore errors here
  }

  configUser_ = new UserConfig(pmUserConfigFile, pmUserConfigDir_, pmInstallDir_);
  if (!configUser_->readFromFile())
    return false;

  QString pmInternalConfigFile = pmUserConfigDir_ + "/PrivacyMachineInternals.ini";
  configSystem_ = new SystemConfig(pmInternalConfigFile);
  if (!configSystem_->init())
    return false;

  if( firstStart )
  {
    // TODO: ask alex for a better way:
    configSystem_->setBaseDiskName("base-disk_1");

    QString baseDiskPath = QDir::toNativeSeparators(pmUserConfigDir_ + "/base-disk/");

    configSystem_->setBaseDiskPath(baseDiskPath);
    QDir baseDiskDir(configSystem_->getBaseDiskPath());
    if (!baseDiskDir.exists())
    {
      if (!baseDiskDir.mkpath("."))
      {
        IERR("error creating the initial base-disk directory: " + configSystem_->getBaseDiskPath());
      }
    }

    if (!configSystem_->write())
    {
      QString systemError = getLastErrorMsg();
      IERR("Error writing internal configuration '" + pmInternalConfigFile + "': " + systemError);
      return false;
    }
  }
  else
  {
    // TODO: test if VM-masks configuration changed in PrivacyMachineInternals.ini
  }

  return true;
}

bool PMManager::init_2()
{

  // read base-disk_Z_capabilities.json
  QString json_val;
  QFile json_file;
  json_file.setFileName(configSystem_->getBaseDiskPath() + configSystem_->getBaseDiskName() + "_capabilities.json");
  if (!json_file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    IERR("Error while opening " + configSystem_->getBaseDiskPath() + configSystem_->getBaseDiskName() + "_capabilities.json");
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


  if (!initAllVmMasks())
    return false;

  return true;
}

QList<ConfigVmMask*>& PMManager::getConfiguredVmMasks()
{
  return configUser_->getConfiguredVmMasks();
}

bool PMManager::vmsExist()
{
  QString allOutput;
  QStringList args;

  foreach( ConfigVmMask* vmMask, getConfiguredVmMasks() )
  {
    // set the timeout to 10s because we get into real troubles if we have no correct information of the existance of a VM
    args.clear();
    args.append("showvminfo");
    args.append(vmMask->vmName);
    if( !ExecShort( configSystem_->getVBoxCommand(), args, &allOutput, true, 10, false) )
      return false;
  }
  return true;
}

bool PMManager::vmMaskRegenerationNecessary()
{

  // TODO: when imlemented flash or other Mask breaking configuration check for that here too

  QSet<QString> alreadyConfiguredVMMasks = QSet<QString>::fromList(configSystem_->getConfiguredVMMaskNames());
  QSet<QString> userConfiguredVMMasks;
  foreach (ConfigVmMask* vmMask, configUser_->getConfiguredVmMasks() )
    userConfiguredVMMasks.insert(vmMask->vmName);
  if( alreadyConfiguredVMMasks != userConfiguredVMMasks)
    //ILOG("\n\nvmMaskRegenerationNeccesary:        "+ QString::number(alreadyConfiguredVMMasks.size())+" | "+QString::number(userConfiguredVMMasks.size())+"\n\n")
    return true;

  if(!vmsExist())
    return true;

  return false;
}

bool PMManager::createCommandsCloneStartCreateSnapshot(
  PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList)
{
  PMCommand* curCommand;
  QStringList args;

  // Create a new VM based on the base-disk_Z.vmdk for this VM-Mask
  args.clear();
  args.append("createvm");
  args.append("--name");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--ostype");
  args.append("Debian_64");
  args.append("--register");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Create a VM for VM-Mask: '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  args.clear();
  args.append("modifyvm");
  args.append(parCurInstance->getConfig()->vmName);
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
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Parametrize the VM for VM-Mask: '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);



  args.clear();
  args.append("storagectl");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--name");
  args.append("SATA");
  args.append("--add");
  args.append("sata");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Parametrize the VM for VM-Mask: '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);


  args.clear();
  args.append("storageattach");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--storagectl");
  args.append("SATA");
  args.append("--port");
  args.append("0");
  args.append("--type");
  args.append("hdd");
  args.append("--medium");
  args.append(configSystem_->getBaseDiskPath() + configSystem_->getBaseDiskName() + ".vmdk");
  args.append("--mtype");
  args.append("immutable");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Parametrize the VM for VM-Mask: '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);




  // Configure Portforwarding
  parCommandsList.append(parCurInstance->getCommandToBindNatPort());


  // Enable VRDP
  args.clear();
  args.append("modifyvm");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--vrde");
  args.append("on");
  args.append("--vrdeport");
  args.append(QString::number(parCurInstance->getConfig()->rdpPort));
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Configure VM-Clone: enable VRDP");
  parCommandsList.append(curCommand);

  // Startup
  args.clear();
  args.append("startvm");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--type");
  args.append("headless"); // 'gui' or 'headless'
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Startup VM-Clone for VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);


  // wait 1 second because VBoxSRV must start before VBoxManage is possible
  curCommand = new PMCommand(1000);
  curCommand->setDescription("Wait till VBoxSVC ist started");
  curCommand->setCosts(50);
  parCommandsList.append(curCommand);

  // Create a special command for BootUpDetection (regulary check the Size of the Display via a
  // Screenshot)
  args.clear();
  args.append("controlvm");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("screenshotpng");  
  QString tmpPngFile = pmUserConfigDir_ + "/logs/tmpBootUpScreen.png";
  args.append(tmpPngFile);
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, tmpPngFile);
  curCommand->setDescription(
    "Wait until system-boot is done for VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(configSystem_->getMachineBootUpTime());
  parCommandsList.append(curCommand);


  // Poll until VM listens on SSH
  curCommand = GetPMCommandForSshCmd( 
    "root", constVmIp, QString::number( parCurInstance->getConfig()->sshPort), constRootPw, "echo Hello PM!" );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 500 );
  curCommand->setRetries( 5 );
  curCommand->setDescription( 
    "Waiting until VM for VM-Mask " + parCurInstance->getConfig()->fullName + "' is restored." );
  curCommand->setCosts( 100 );
  parCommandsList.append( curCommand );


  // Set password for liveuser
  curCommand = GetPMCommandForSshCmd(
    "root",constVmIp,QString::number( parCurInstance->getConfig()->sshPort),constRootPw,"usermod -p $(echo " + QString( constRootPw ) + " | openssl passwd -1 -stdin) liveuser" );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 500 );
  curCommand->setRetries( 5 );
  curCommand->setDescription(
    "Set password for liveuser in VM-Mask " + parCurInstance->getConfig()->fullName );
  curCommand->setCosts( 50 );
  parCommandsList.append( curCommand );


//  // Copy folder pm to VM
//  //TODO: At startup: ensure that we are at the location of the executable
//  //TODO: On Windows: the executable flag gets lost, and scripts have the wrong lineendings
//  args.clear();
//  curCommand = GetPMCommandForScp2VM("liveuser",constVmIp,QString::number(parCurInstance->getConfig()->sshPort),constRootPw,"../../packaging/build_base-disk/pm_files/pm","/");
//  curCommand->setDescription("Copy obfuscation-scripts to VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
//  curCommand->setCosts(configSystem_->getCopyScriptsPerSshTime());
//  parCommandsList.append(curCommand);

  // Copy liveusers home directory
#ifdef USE_OUTSIDE_FIREFOX_PROFILE
  curCommand = GetPMCommandForScp2VM("liveuser",constVmIp,QString::number(parCurInstance->getConfig()->sshPort),constRootPw,"pm_data/VM_ROOTFS/home/liveuser","/home/");
  curCommand->setDescription("Copy liveusers home directory to VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(200);
  parCommandsList.append(curCommand);
#endif


  // Set X-keybord layout to DE
  curCommand = GetPMCommandForSshCmd("liveuser",constVmIp,QString::number(parCurInstance->getConfig()->sshPort),constRootPw,"/pm/scripts/set_keybord_layout_to_de.sh");
  curCommand->setDescription(
    "Set X-keybord layout to DE in VM-Mask " + parCurInstance->getConfig()->fullName );
  curCommand->setCosts( 50 );
  parCommandsList.append( curCommand );



  // Create a snapshot
  QString snapshotName = "UpAndRunning";
  args.clear();
  args.append("snapshot");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("take");
  args.append( snapshotName);
  if (!RunningOnWindows()) args.append("--pause");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Create a Snapshot for VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  /*
  // TODO: Bernhard: remove this
  // wait 5 seconds because weird bug on windows
  curCommand = new PMCommand(5000);
  curCommand->setDescription("safety-wait till snapshot has been created");
  curCommand->setCosts(50);
  parCommandsList.append(curCommand);
  */

  // Poll until snapshot has been created.
  // We try to list the snapshot's VM info, which should only succeed once it has been created.
  args.clear();
  args.append("snapshot");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("showvminfo");
  args.append( snapshotName );
  curCommand = new PMCommand( configSystem_->getVBoxCommand(), args, true, false );
  curCommand->setDescription( 
    "Waiting until snapshot for VM-Mask " + parCurInstance->getConfig()->fullName + "' has been created." );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 1000 );
  curCommand->setRetries( 100 );
  curCommand->setCosts( 100 );
  parCommandsList.append( curCommand );


  args.clear();
  args.append("controlvm");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("poweroff");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Poweroff VM for VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  // Poll until VM has been powered off.
  // We list the VM's info and look for "State: powered off"
  args.clear();
  args.append("showvminfo");
  args.append(parCurInstance->getConfig()->vmName);
  curCommand = new PMCommand( configSystem_->getVBoxCommand(), args, true, false );
  curCommand->setDescription( 
    "Waiting until VM for VmMask " + parCurInstance->getConfig()->fullName + "' is powered off." );
  curCommand->setRegexPattern( "State:\\s*powered off" );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 1000 );
  curCommand->setRetries( 100 );
  curCommand->setCosts( 100 );
  parCommandsList.append( curCommand );


  // Restore to the saved snapshot
  args.clear();
  args.append("snapshot");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("restore");
  args.append("UpAndRunning");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription(
    "Restore the Snapshot for VmMask '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  return true;
}

PMInstance* PMManager::createPMInstance(ConfigVmMask *parCurVmMask)
{
  PMInstance                  *newPMInst          = 0;
  PMInstanceConfiguration     *newPMInstConfig    = 0;

  newPMInstConfig     = new PMInstanceConfiguration(parCurVmMask, configUser_, configSystem_, baseDiskCapabilities_);
  newPMInstConfig->init();
  newPMInst           = new PMInstance(newPMInstConfig);

  // Assign a unique RDP Port and Hostname
  // For assigning unique ports we need an index of the configured VM-Masks
  newPMInstConfig->vmMaskId = configUser_->getConfiguredVmMasks().indexOf( parCurVmMask );
  newPMInstConfig->rdpPort = firstFreeLocalPort_ + newPMInstConfig->vmMaskId * 2;

  // We take the next local Port for the ssh connection
  newPMInstConfig->sshPort = newPMInstConfig->rdpPort + 1;

  newPMInstConfig->hostName = "PMInstance_OnPort_" + QString::number(newPMInstConfig->rdpPort);

  pm_.push_back(newPMInst);
  pmConfigs.push_back(newPMInstConfig);

  return newPMInst;
}

bool PMManager::initAllVmMasks()
{
  foreach(ConfigVmMask* curVmMask, configUser_->getConfiguredVmMasks())
  {
    // Create a Instance with a 'unique' Fingerprint
    createPMInstance(curVmMask);
  }

  return true;
}

bool PMManager::createCommandsUpdateAllVmMasks(QList<PMCommand*>& parCommandsList)
{
  foreach (PMInstance* curInstance, pm_)
  {
    if(!createCommandsCloneStartCreateSnapshot(curInstance, parCommandsList))
      return false;
  }

  return true;
}

bool PMManager::createCommandsClose(
  PMInstance* parPmInstance, QList<PMCommand*>& parCommandsList)
{
  QStringList args;

  PMCommand *curCommand;

  args.clear();
  args.append("--nologo");  //suppress the logo
  args.append("controlvm");
  args.append(parPmInstance->getConfig()->vmName);
  args.append("poweroff");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Poweroff VM");
  parCommandsList.append(curCommand);

  args.clear();
  args.append("--nologo");  //suppress the logo
  args.append("snapshot");
  args.append(parPmInstance->getConfig()->vmName);
  args.append("restorecurrent");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Restore Snapshot of VM");
  parCommandsList.append(curCommand);

  return true;
}

bool PMManager::createCommandsCloseAllVms( QList<PMCommand*>& parCommandsList )
{
  foreach( PMInstance* curInstance, pm_ )
  {
    // before closing copy VPN logs
    PMCommand* pCurrentCommand = NULL;
    QString userConfigDir;
    getAndCreateUserConfigDir(userConfigDir);
    pCurrentCommand = GetPMCommandForScp2Host("root",constVmIp,QString::number( curInstance->getConfig()->sshPort ),constRootPw,
                                              userConfigDir+"/logs/vmMask_"+curInstance->getConfig()->name+"_vpnLog.txt",
                                              "/var/log/openvpn.log");
    pCurrentCommand->setDescription("Copy vpn logs of VM-Mask "+curInstance->getConfig()->name);
    pCurrentCommand->setTimeoutMilliseconds(2000);
    parCommandsList.append( pCurrentCommand );


    if( !createCommandsClose( curInstance, parCommandsList ) )
    {
      return false;
    }
  }
  return true;
}

bool PMManager::createCommandsStart(
  QString parInstanceName, QList<PMCommand*>& parCommandsList)
{
  int waittime;

  for(int indexOfVmMask=0; indexOfVmMask < configUser_->getConfiguredVmMasks().size(); indexOfVmMask++)
  {
    if( parInstanceName != configUser_->getConfiguredVmMasks().at( indexOfVmMask )->name )
      continue;

    ConfigVmMask* curVmMask = configUser_->getConfiguredVmMasks().at(indexOfVmMask);
    // XXX AL FIXME - potential SIGSEGV-source. Align VmMasks and PMInstances. We shall not assume to have the same
    // indices in both lists.
    PMInstance* curInstance = pm_.at( indexOfVmMask );
    // Re-Initialize the instance's configuration before every start in order to randomize all settings that are to be obfuscated.
    curInstance->getConfig()->init();    
    
    runningInstances_.push_back(parInstanceName);
    QStringList args;
    PMCommand *curCommand;
    

    // We might still have an abandoned VM with the same name running from a previous session.
    // To be on the safe side, we attempt to stop it and to reset it to UpAndRunning.
    args.clear();
    args.append("controlvm");
    args.append( curInstance->getConfig()->vmName );
    args.append("poweroff");
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription(
      "Attempt to power off abandoned VM-Clone for VM-Mask '"
      + curInstance->getConfig()->fullName + "'");
    // If the VM actually is not running (which would give us an error), that is fine for us as
    // well.
    curCommand->setIgnoreErrors( true );
    parCommandsList.append(curCommand);

    QString snapshotName = "UpAndRunning";
    args.clear();
    args.append("snapshot");
    args.append( curInstance->getConfig()->vmName );
    args.append("restore");
    args.append( snapshotName );
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription(
      "Attempt to reset VM for VM-Mask '" + curInstance->getConfig()->fullName + "' to snapshot'"
      + snapshotName + "'" );
    // Ignore this error because on normal exit the snapshot is already restored
    // -> not possible to restore again
    curCommand->setIgnoreErrors( true );
    parCommandsList.append(curCommand);

    // Start VM
    args.clear();
    args.append("startvm");
    args.append(curInstance->getConfig()->vmName);
    args.append("--type");
    args.append("headless"); // 'gui' or 'headless'
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription(
      "Startup VM-Clone for VM-Mask '" + curInstance->getConfig()->fullName + "'");
    curCommand->setCosts(configSystem_->getMachineRestoreTime());

    parCommandsList.append(curCommand);


    // Poll until VM listens on SSH
    curCommand = GetPMCommandForSshCmd( 
      "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,"echo Hello PM!" );
    curCommand->setType( pollingShellCommand );
    curCommand->setTimeoutMilliseconds( 500 );
    curCommand->setRetries( 5 );
    curCommand->setDescription( 
      "Waiting until VM for VM-Mask " + curInstance->getConfig()->fullName + "' is restored." );
    curCommand->setCosts( 100 );
    parCommandsList.append( curCommand );

    // Copy liveusers home directory
#ifdef USE_OUTSIDE_FIREFOX_PROFILE
    curCommand = GetPMCommandForScp2VM("liveuser",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,"pm_data/VM_ROOTFS/home/liveuser","/home/");
    curCommand->setDescription("Copy liveusers home directory to VM-Mask '" + curInstance->getConfig()->fullName + "'");
    curCommand->setCosts(200);
    parCommandsList.append(curCommand);
#endif

    // obfuscate fonts:
    foreach(const QString &font, curInstance->getConfig()->fonts)
    {
      curCommand = GetPMCommandForSshCmd( "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                       "mv /pm/fonts/" + font + " /usr/local/share/fonts");
      curCommand -> setCosts(10);
      curCommand -> setDescription("Installing font "+font);
      parCommandsList.append(curCommand);
    }
    curCommand = GetPMCommandForSshCmd( "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                     "fc-cache -f");
    curCommand -> setDescription("Finish font obfuscation");
    curCommand -> setCosts(100);
    parCommandsList.append(curCommand);

    ConfigVPN& currentVpnConfig = curInstance->getConfig()->usedVPNConfig;
    if( currentVpnConfig.vpnType != "OpenVPN" &&
        currentVpnConfig.vpnType != "VPNGate" )
    {
      // obfuscate dns-server:
      curCommand = GetPMCommandForSshCmd( "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                     "rm /etc/resolv.conf && touch /etc/resolv.conf");
      curCommand -> setDescription("cleanup DNS-Server");
      curCommand -> setCosts(10);
      parCommandsList.append(curCommand);
      foreach(const QString &dns, curInstance->getConfig()->dnsServers)
      {
        curCommand = GetPMCommandForSshCmd( "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                         "echo nameserver "+dns+" >> /etc/resolv.conf");
        curCommand -> setCosts(10);
        curCommand -> setDescription("Installing DNS-Server "+dns);
        parCommandsList.append(curCommand);
      }
    }
    // obfuscate time zone:
    curCommand = GetPMCommandForSshCmd( "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                     "timedatectl set-timezone " + curInstance->getConfig()->timeZone );
    curCommand -> setDescription("Obfuscating time zone");
    curCommand -> setCosts(10);
    parCommandsList.append(curCommand);

    // obfuscate locale
    curCommand = GetPMCommandForSshCmd("root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,
                                     "localectl set-locale LANG=" + curInstance->getConfig()->locale
                                     + " LANGUAGE=" + curInstance->getConfig()->locale );
    curCommand -> setDescription("Obfuscating locale");
    curCommand -> setCosts(10);
    parCommandsList.append(curCommand);


    if( currentVpnConfig.vpnType == "OpenVPN" ||
        currentVpnConfig.vpnType == "VPNGate" )
    {
      VmInitVpn vmInitVpn( currentVpnConfig );

      if( currentVpnConfig.vpnType == "OpenVPN" )
      {
        // scp behaves a little weird so we copy the files in the following way:
        //   ssh curVM -c mkdir -p /home/vmConfig/tmp
        //   scp ~./conf/vpn/MyVpn curVM:/home/vmConfig/tmp
        //      results in directory: /home/vmConfig/tmp/MyVpn
        //   ssh curVM -c mv /home/vmConfig/tmp/MyVpn /home/vmConfig/vpn
        //   ssh curVM -c ln -s /home/vmConfig/vpn/germany.ovpn /home/vmConfig/vpn/openvpn_pm.ovpn
        // The OpenVPN-Service always expect a file /home/vmConfig/vpn/openvpn_pm.ovpn

        // Creating VPN configuration directory inside VM-Mask
        curCommand = GetPMCommandForSshCmd(
                       "root",
                       constVmIp,
                       QString::number( curInstance->getConfig()->sshPort ),
                       constRootPw,
                       "mkdir -p " + VmInitVpn::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp" );
        curCommand->setDescription( "Creating VPN configuration directory inside VM-Mask" );
        curCommand->setCosts(200);
        parCommandsList.append(curCommand);


        // Copy OpenVPN configuration files to VM-Mask
        curCommand = GetPMCommandForScp2VM(
                       "root",
                       constVmIp,
                       QString::number( curInstance->getConfig()->sshPort ),
                       constRootPw,
                       vmInitVpn.getHostConfigDirectoryPath(),
                       VmInitVpn::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp/" );
        curCommand->setDescription(
              "Copy OpenVPN configuration files to VM-Mask '" + curInstance->getConfig()->fullName + "'");
        curCommand->setCosts(20);
        parCommandsList.append(curCommand);

        // Move VPN configuration directory to the expected location
        curCommand = GetPMCommandForSshCmd(
                       "root",
                       constVmIp,
                       QString::number( curInstance->getConfig()->sshPort ),
                       constRootPw,
                       "mv '" + VmInitVpn::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/tmp/" + vmInitVpn.getHostConfigDirectoryName() + "' " + VmInitVpn::VM_OPENVPN_CONFIG_DIRECTORY_PATH + "/vpn");
        curCommand->setDescription( "Move VPN configuration directory to the expected location" );
        curCommand->setCosts(200);
        parCommandsList.append(curCommand);

        // Create link for of random choosen VPN-config-file to to default name
        curCommand = GetPMCommandForSshCmdPMInstance( curInstance, vmInitVpn.getCommandVmLinkConfig() );
        curCommand->setDescription( "Create link for of random choosen VPN-config-file to to default name" );
        curCommand->setCosts(200);
        parCommandsList.append(curCommand);
      }
      else // "VPNGate"
      {
        // start a script which downloads the list of currently available VPNGate-Servers and configures a
        // file named /home/vmConfig/openvpn_pm.ovpn

        curCommand = GetPMCommandForSshCmd(
          "root",
          constVmIp,
          QString::number( curInstance->getConfig()->sshPort ),
          constRootPw,
          "wget -O /tmp/vpngate.csv http://www.vpngate.net/api/iphone/" );
        curCommand->setDescription( "Downloading VPNGate server list" );
        curCommand->setCosts(1000);
        curCommand->setRetries(3);
        parCommandsList.append(curCommand);

        curCommand = GetPMCommandForSshCmd(
          "root",
          constVmIp,
          QString::number( curInstance->getConfig()->sshPort ),
          constRootPw,
          "/pm/scripts/configureVpnGate.py /tmp/vpngate.csv" );
        curCommand->setDescription( "Creating a OpenVPN configuration file for VPN-Gate" );
        curCommand->setCosts(1000);
        curCommand->setRetries(3);
        parCommandsList.append(curCommand);
      }

      // Set up openvpn_log trigger
      curCommand = GetPMCommandForSshCmd(
        "root",
        constVmIp,
        QString::number( curInstance->getConfig()->sshPort ),
        constRootPw,
        "systemctl start openvpn_log.path" );
      curCommand->setDescription( "Set up openvpn_log trigger" );
      curCommand->setCosts(20);
      parCommandsList.append( curCommand );

      // Before setting up the browser's path service, make sure it is configured correctly.
      appendCommandsConfigureBrowser( curInstance, parCommandsList );

      QString browser = curInstance->getConfig()->browser;
      curCommand = GetPMCommandForSshCmd(
        "root",
        constVmIp,
        QString::number( curInstance->getConfig()->sshPort ),
        constRootPw,
        "systemctl start " + browser + ".path");
      curCommand->setDescription( "Set up " + browser + " trigger" );
      curCommand->setCosts(20);
      parCommandsList.append( curCommand );

      // Trigger VPN connection, which triggers:
      // * Log output display in dedicated xterm window
      // * an update of resolv.conf on successful connection, which in turn triggers
      // * starting firefox
      curCommand = GetPMCommandForSshCmdPMInstance( curInstance, vmInitVpn.getCommandVmConnect() );
      curCommand->setDescription( "Connect VPN" );
      curCommand->setCosts(200);
      parCommandsList.append(curCommand);

    }
    // If there is no OpenVPN connection configured, firefox can be started straightaway without waiting for the
    // connection to be established:
    else
    {
//      // start firefox
//      curCommand = GetPMCommandForSshCmd(
//        "root",constVmIp,QString::number(curInstance->getConfig()->sshPort),constRootPw,"systemctl start firefox.service");
//      curCommand -> setDescription("Start firefox");
//      curCommand -> setCosts(10);
//      parCommandsList.append(curCommand);

      // start configured Browser
      appendCommandsConfigureBrowser( curInstance, parCommandsList );
      appendCommandsToStartBrowserService(curInstance,parCommandsList);
    }

    return true;

  }
  return false;
}

void PMManager::appendCommandsConfigureBrowser( PMInstance *pPmInstance, QList<PMCommand*>& parCommandsList )
{
  if( pPmInstance->getConfig()->browser == "firefox" )
  {
    // obfuscate firefox adddons
    PMCommand *pCurrentCommand = 
      GetPMCommandForSshCmd(
        "root",constVmIp,
        QString::number(pPmInstance->getConfig()->sshPort),
        constRootPw,
        "/pm/firefox_addons/rmAddons.py  /home/liveuser/.mozilla/firefox/al5g3l76.default/extensions/  "
        "/pm/firefox_addons/installed_addons/" );
    pCurrentCommand -> setDescription("Obfuscating firefox addons: remove random addons");
    pCurrentCommand -> setCosts(100);
    parCommandsList.append(pCurrentCommand);

  }
  // TODO:
  // else if( pPmInstance->getConfig()->browser ==...)

}

void PMManager::appendCommandsToStartBrowserService(PMInstance *pPmInstance, QList<PMCommand*>& parCommandsList )
{
  PMCommand *pCurrentCommand;
  pCurrentCommand = GetPMCommandForSshCmd(
                      "root",constVmIp,
                      QString::number(pPmInstance->getConfig()->sshPort),
                      constRootPw,
                      "systemctl start " + pPmInstance->getConfig()->browser + ".service");
  // FIXME AL: for e.g. torbrowser, this won't say "TOR-Browser" anymore. Is this still ok?
  pCurrentCommand -> setDescription( "Start " + pPmInstance->getConfig()->browser );
  pCurrentCommand -> setCosts(10);
  parCommandsList.append(pCurrentCommand);

}

bool PMManager::saveConfiguredVMMasks()
{
  QStringList vmMaskNames;
  foreach (ConfigVmMask* vmMask, configUser_->getConfiguredVmMasks() )
    vmMaskNames.append(vmMask->vmName);
  configSystem_->setConfiguredVMMaskNames(vmMaskNames);
  return configSystem_->write();
}

bool PMManager::createCommandsCleanupVirtualBoxVms(QList<PMCommand*>& parCommandsList)
{
  QStringList args;
  PMCommand *curCommand;

  QDir vboxDir(vboxDefaultMachineFolder_);

  if (!vboxDir.exists())
  {
    IERR("The 'Default machine folder' of VirtualBox does not exist: '" + vboxDefaultMachineFolder_ + "'");
    return false;
  }

  vboxDir.setNameFilters( QStringList( "pm_VmMask_*" ) );
  QStringList foundVmMasks = vboxDir.entryList( QDir::Dirs | QDir::Readable | QDir::CaseSensitive );

  // Add folders/VmMask-names to the list which are in the way for new VmMask creation
  foreach (PMInstance* configuredInstance, pm_)
  {
    QString dirNameForNewVmMask = "pm_VmMask_" + configuredInstance->getConfig()->name;
    if (!foundVmMasks.contains(dirNameForNewVmMask))
      foundVmMasks.append(dirNameForNewVmMask);
  }

  foreach (const QString &oldVmMask, foundVmMasks)
  {
    // To be on the safe side, we attempt to stop the VM
    args.clear();
    args.append("controlvm");
    args.append( oldVmMask );
    args.append("poweroff");
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription( "Attempt to power off old VM-Mask '" + oldVmMask + "'" );
    curCommand->setIgnoreErrors( true );
    parCommandsList.append(curCommand);

    // Delete the VM
    args.clear();
    args.append("unregistervm");
    args.append( oldVmMask );
    args.append("--delete");
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription( "Attempt to delete the old VM-Mask '" + oldVmMask + "'" );
    curCommand->setIgnoreErrors( true );
    parCommandsList.append(curCommand);

    // virtualbox needs some break
    curCommand = new PMCommand(200);
    curCommand->setDescription( "Safety wait for virtualbox after remove of vm");
    parCommandsList.append(curCommand);

    // Delete the folder
    curCommand = new PMCommand(vboxDefaultMachineFolder_ + "/" + oldVmMask);
    curCommand->setDescription( "Attempt to delete the old VM-Mask folder '" + vboxDefaultMachineFolder_ + "/" + oldVmMask + "'" );
    curCommand->setIgnoreErrors( true );
    parCommandsList.append(curCommand);
  }


  // Remove all BaseDisk's from the MediaManager
  QDir baseDiskDir(configSystem_->getBaseDiskPath());

  if (baseDiskDir.exists())
  {
    baseDiskDir.setNameFilters( QStringList( "base-disk_*.vmdk" ) );
    QStringList foundBaseDisks = baseDiskDir.entryList( QDir::Files | QDir::Readable | QDir::CaseSensitive );

    foreach (const QString &oldBaseDisk, foundBaseDisks)
    {
      if (!oldBaseDisk.contains("_flat"))
      {
        args.clear();
        args.append("closemedium");
        args.append( configSystem_->getBaseDiskPath() + oldBaseDisk);
        curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
        curCommand->setDescription( "Attempt to delete the old BaseDisk '" + oldBaseDisk + "'" );
        curCommand->setIgnoreErrors( true );
        parCommandsList.append(curCommand);
      }
    }
  }
  
  // All BaseDisks have the same id: remove it
  args.clear();
  args.append("closemedium");
  args.append( "0ac66e6e-2a04-4d75-9357-c5c3bc87dcf2" );
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription( "Attempt to delete the old BaseDisk by id");
  curCommand->setIgnoreErrors( true );
  parCommandsList.append(curCommand);

  // virtualbox needs some break
  curCommand = new PMCommand(3000);
  curCommand->setDescription( "Safety wait for vboxmanage to organize itself");
  parCommandsList.append(curCommand);

  return true;
}
