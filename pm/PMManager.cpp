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

#include "utils.h"
#include "PMManager.h"
#include "SystemConfig.h"
#include "UserConfig.h"

#include <QSettings>
#include <QDebug>
#include <QCoreApplication>

PMManager::PMManager() :
  configUser_(0),
  configSystem_(0),
  firstFreeLocalPort_(4242)
{
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

bool PMManager::init(QString pmInstallPath)
{
  pmInstallPath_ = pmInstallPath;

  configUser_ = new UserConfig();
  if (!configUser_->readFromFile())
    return false;

  configSystem_ = new SystemConfig();
  if (!configSystem_->init())
    return false;

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
  bool allFound = true;
  QList< QString > vmsExisting;
  QList< QString > vmsMissing;
  QString output;
  foreach( ConfigVmMask* vmMask, getConfiguredVmMasks() )
  {
    if( vmsExisting.contains( vmMask->vmName )
      || vmsMissing.contains( vmMask->vmName ) )
    {
      continue;
    }

    if( ExecShort( configSystem_->getVBoxCommand() + " showvminfo " + vmMask->vmName, &output, true, 0, true) )
    {
      vmsExisting.append( vmMask->vmName );

    }
    else
    {
      vmsMissing.append( vmMask->vmName );
      allFound = false;
      break;
    }

  }

  return allFound;

}

bool PMManager::createCommandsCloneStartCreateSnapshot(
  PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList)
{
  // Usable here:
  // parCurInstance->getConfig()->rdpPort;
  // parCurInstance->getConfig()->hostName;

  PMCommand* curCommand;
  QStringList args;

  // Create a new VM based on the base-disk.vdi for this VM-Mask
  args.clear();
  args.append("createvm");
  args.append("--name");
  args.append(parCurInstance->getConfig()->vmName);
  args.append("--ostype");
  args.append("Ubuntu_64");
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
  args.append("base-disk.vdi");
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


  // wait 3 secounds because VBoxSRV must start before VBoxManage is possible
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
  QString tmpPngFile = QCoreApplication::applicationDirPath() + "/logs/tmpBootUpScreen.png";
  args.append(tmpPngFile);
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, tmpPngFile);
  curCommand->setDescription(
    "Wait until system-boot is done for VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(configSystem_->getMachineBootUpTime());
  parCommandsList.append(curCommand);


  // Poll until VM listens on SSH
  curCommand = GetPMCommandForSshCmd( 
    "root","127.0.0.1",QString::number( parCurInstance->getConfig()->sshPort),"123","echo Hello PM!" );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 500 );
  curCommand->setRetries( 5 );
  curCommand->setDescription( 
    "Waiting until VM for VM-Mask " + parCurInstance->getConfig()->fullName + "' is restored." );
  curCommand->setCosts( 100 );
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
#endif
  parCommandsList.append( curCommand );


  // Set password for liveuser
  curCommand = GetPMCommandForSshCmd(
    "root","127.0.0.1",QString::number( parCurInstance->getConfig()->sshPort),"123","usermod -p $(echo 123 | openssl passwd -1 -stdin) liveuser" );
  curCommand->setType( pollingShellCommand );
  curCommand->setTimeoutMilliseconds( 500 );
  curCommand->setRetries( 5 );
  curCommand->setDescription(
    "Set password for liveuser in VM-Mask " + parCurInstance->getConfig()->fullName );
  curCommand->setCosts( 50 );
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
#endif
  parCommandsList.append( curCommand );


//  // Copy folder pm to VM
//  //TODO: At startup: ensure that we are at the location of the executable
//  //TODO: On Windows: the executable flag gets lost, and scripts have the wrong lineendings
//  args.clear();
//  curCommand = GetPMCommandForScp2VM("liveuser","127.0.0.1",QString::number(parCurInstance->getConfig()->sshPort),"123","../../packaging/build_base-disk/pm_files/pm","/");
//  curCommand->setDescription("Copy obfuscation-scripts to VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
//  curCommand->setCosts(configSystem_->getCopyScriptsPerSshTime());
//#ifdef PM_WINDOWS
//  curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
//#endif
//  parCommandsList.append(curCommand);

  // Copy liveusers home directory
#ifndef USE_OUTSIDE_FIREFOX_PROFILE
  curCommand = GetPMCommandForScp2VM("liveuser","127.0.0.1",QString::number(parCurInstance->getConfig()->sshPort),"123","pm_data/VM_ROOTFS/home/liveuser","/home/");
  curCommand->setDescription("Copy liveusers home directory to VM-Mask '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(200);
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
#endif
  parCommandsList.append(curCommand);
#endif


  // Set X-keybord layout to DE
  curCommand = GetPMCommandForSshCmd("liveuser","127.0.0.1",QString::number(parCurInstance->getConfig()->sshPort),"123","/pm/scripts/set_keybord_layout_to_de.sh");
  curCommand->setDescription(
    "Set X-keybord layout to DE in VM-Mask " + parCurInstance->getConfig()->fullName );
  curCommand->setCosts( 50 );
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
#endif
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
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
#endif
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
#ifdef PM_WINDOWS
  curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
#endif
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

  newPMInstConfig     = new PMInstanceConfiguration(parCurVmMask, configUser_, configSystem_);
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
    if( !createCommandsClose( curInstance, parCommandsList ) )
    {
      return false;
    }
  }

}

bool PMManager::createCommandsStart(
  QString parInstanceName, QList<PMCommand*>& parCommandsList)
{
  int waittime;

  for(int indexOfVmMask=0; indexOfVmMask < configUser_->getConfiguredVmMasks().size(); indexOfVmMask++)
  {
    if(parInstanceName != configUser_->getConfiguredVmMasks().at(indexOfVmMask)->name)
      continue;

    ConfigVmMask* curVmMask = configUser_->getConfiguredVmMasks().at(indexOfVmMask);
    // XXX AL FIXME - potential SIGSEGV-source. Align VmMasks and PMInstances. We shall not assume to have the same indices in both lists.
    PMInstance* curInstance = pm_.at( indexOfVmMask );
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
      "root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123","echo Hello PM!" );
    curCommand->setType( pollingShellCommand );
    curCommand->setTimeoutMilliseconds( 500 );
    curCommand->setRetries( 5 );
    curCommand->setDescription( 
      "Waiting until VM for VM-Mask " + curInstance->getConfig()->fullName + "' is restored." );
    curCommand->setCosts( 100 );
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors( true ); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append( curCommand );

    // Copy liveusers home directory
#ifdef USE_OUTSIDE_FIREFOX_PROFILE
    curCommand = GetPMCommandForScp2VM("liveuser","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123","pm_data/VM_ROOTFS/home/liveuser","/home/");
    curCommand->setDescription("Copy liveusers home directory to VM-Mask '" + curInstance->getConfig()->fullName + "'");
    curCommand->setCosts(200);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);
#endif

    // Copy live pm_on_restore to VM
    curCommand = GetPMCommandForScp2VM("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123","pm_data/VM_ROOTFS/pm_on_restore","/");
    curCommand->setDescription("Copy obfuscation-scripts to restored VM-Mask '" + curInstance->getConfig()->fullName + "'");
    curCommand->setCosts(configSystem_->getCopyScriptsPerSshTime());
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    // TODO: Obfuscate fingerprint here

    // obfuscate fonts:
    curCommand = GetPMCommandForSshCmd("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123","/pm_on_restore/fonts/install_random_fonts.sh " + QString::number(curInstance->getConfig()->fontCount));
    curCommand -> setDescription("Obfuscating fonts: install random fonts");
    curCommand -> setCosts(100);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    // obfuscate firefox adddons
    curCommand = GetPMCommandForSshCmd("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123",
                                     "/pm_on_restore/rmAddons.py  /home/liveuser/.mozilla/firefox/5sv1yflp.default-1448031113287/extensions  /pm_on_restore/installed_addons_firefox");
    curCommand -> setDescription("Obfuscating firefox addons: remove random addons");
    curCommand -> setCosts(100);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    // obfuscate time zone
    curCommand = GetPMCommandForSshCmd( "root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123",
                                     "timedatectl set-timezone " + curInstance->getConfig()->timeZone_ );
    curCommand -> setDescription("Obfuscating time zone");
    curCommand -> setCosts(10);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    // obfuscate locale
    curCommand = GetPMCommandForSshCmd("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123",
                                     "localectl set-locale LANG=" + curInstance->getConfig()->locale_ );
    curCommand -> setDescription("Obfuscating locale - LANG");
    curCommand -> setCosts(10);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    curCommand = GetPMCommandForSshCmd("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123",
                                     "localectl set-locale LANGUAGE=" + curInstance->getConfig()->locale_ );
    curCommand -> setDescription("Obfuscating locale - LANGUAGE");
    curCommand -> setCosts(10);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);

    // start firefox
    curCommand = GetPMCommandForSshCmd("root","127.0.0.1",QString::number(curInstance->getConfig()->sshPort),"123","systemctl start firefox.service");
    curCommand -> setDescription("Start firefox");
    curCommand -> setCosts(10);
  #ifdef PM_WINDOWS
    curCommand->setIgnoreErrors(true); // TODO: check what's wrong on windows
  #endif
    parCommandsList.append(curCommand);
    return true;
  }
  return false;
}
