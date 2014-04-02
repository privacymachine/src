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
  nextFreeRDPPort_(4242)
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

bool PMManager::init()
{
  configUser_ = new UserConfig();
  if (!configUser_->readFromFile())
    return false;

  configSystem_ = new SystemConfig();
  if (!configSystem_->init())
    return false;

  if (!initAllUsecases())
    return false;

  return true;
}

QList<QString> PMManager::getUseCaseNames()
{
  QList<QString> temp;
  for(int i=0; i<configUser_->getConfiguredUseCases().size(); i++)
    temp.push_back(configUser_->getConfiguredUseCases().at(i)->name);
  return temp;
}

bool PMManager::createCommandsForOneInstance(PMInstance* parCurInstance, QList<PMCommand*>& parCommandsList)
{
  // Usable here:
  // parCurInstance->getConfig()->rdpPort;
  // parCurInstance->getConfig()->hostName;

  PMCommand* curCommand;
  QStringList args;

  // Create a new Clone for this usecase
  args.clear();
  args.append("clonevm");
  args.append("pm_base");
  args.append("--snapshot");
  args.append("BaseForCloneOffline");
  args.append("--options");
  args.append("link");
  args.append("--register");
  args.append("--name");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Create a VM-Clone for Usecase: '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);


  // Enable sharing of memory <http://www.virtualbox.org/manual/ch04.html#guestadd-pagefusion>
  args.clear();
  args.append("modifyvm");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("--pagefusion");
  args.append("on");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Configure VM-Clone: enable pagefusion");
  parCommandsList.append(curCommand);

  // Add a NAT-Rule for FreeRDP
  parCommandsList.append(parCurInstance->getCommandToBindNatPort());

  // Startup
  args.clear();
  args.append("startvm");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("--type");
  args.append("headless"); // 'gui' or 'headless'
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Startup VM-Clone for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);


  // Create a special command for BootUpDetection (regulary check the Size of the Display via a Screenshot)
  args.clear();
  args.append("controlvm");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("screenshotpng");
  QString tmpPngFile = QCoreApplication::applicationDirPath() + "/logs/tmpBootUpScreen.png";
  args.append(tmpPngFile);
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, tmpPngFile);
  curCommand->setDescription("Wait till system-boot is done for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(configSystem_->getMachineBootUpTime());
  parCommandsList.append(curCommand);

  // Wait 5 seconds
  for(int i = 0; i < 5; i++)
  {
    curCommand = new PMCommand(1000);
    curCommand->setDescription("Wait "+QString::number(5-i)+"sec till still VM for Usecase '" + parCurInstance->getConfig()->fullName + "' is restored");
    parCommandsList.append(curCommand);
  }

  // Check for a specific process which starts lately
  args.clear();
  args.append("guestcontrol");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("execute");
  args.append("--username");
  args.append("pm");
  args.append("--password");
  args.append("123");
  args.append("--wait-exit");
  args.append("--wait-stdout");
  args.append("--image");
  args.append("/pm/scripts/wait_till_desktop_running.sh");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Wait till startup is done for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  curCommand->setCosts(configSystem_->getMachineServiceStartupTime());
  parCommandsList.append(curCommand);

  // Create a snapshot
  args.clear();
  args.append("snapshot");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("take");
  args.append("UpAndRunning");
  if (!RunningOnWindows()) args.append("--pause");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Create a Snapshot for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  // Wait some time for safety on older machines
  int waittime=configSystem_->getWaitTimeAfterCreateSnapshot();
  for(int i = 0; i < waittime; i++)
  {
    curCommand = new PMCommand(1000);
    curCommand->setDescription("Wait "+QString::number(waittime-i)+"sec till snapshot for Usecase '" + parCurInstance->getConfig()->fullName + "' is created");
    parCommandsList.append(curCommand);
  }

  args.clear();
  args.append("controlvm");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("poweroff");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Poweroff VM for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  waittime=configSystem_->getWaitTimeAfterPowerOff();
  for(int i = 0; i < waittime; i++)
  {
    curCommand = new PMCommand(1000);
    curCommand->setDescription("Wait "+QString::number(waittime-i)+"sec till Usecase '" + parCurInstance->getConfig()->fullName + "' is Poweroff");
    parCommandsList.append(curCommand);
  }

  // Restore to the saved snapshot
  args.clear();
  args.append("snapshot");
  args.append("pm_Usecase_" + parCurInstance->getConfig()->name);
  args.append("restore");
  args.append("UpAndRunning");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Restore the Snapshot for Usecase '" + parCurInstance->getConfig()->fullName + "'");
  parCommandsList.append(curCommand);

  return true;
}

PMInstance* PMManager::createPMInst(ConfigUseCase *parCurUseCase)
{
  PMInstance                  *newPMInst          = 0;
  PMInstanceConfiguration     *newPMInstConfig    = 0;

  newPMInstConfig     = new PMInstanceConfiguration(parCurUseCase, configUser_,configSystem_);
  newPMInst           = new PMInstance(newPMInstConfig);

  // Assign a unique RDP Port and Hostname
  newPMInstConfig->rdpPort = nextFreeRDPPort_;
  nextFreeRDPPort_++;

  newPMInstConfig->hostName = "PMInstance_OnPort_" + QString::number(newPMInstConfig->rdpPort);

  pm_.push_back(newPMInst);
  pmConfigs.push_back(newPMInstConfig);

  return newPMInst;
}

bool PMManager::initAllUsecases()
{
  foreach(ConfigUseCase* curUseCase, configUser_->getConfiguredUseCases())
  {
    // Create a Instance with a 'unique' Fingerprint
    PMInstance* curInstance = createPMInst(curUseCase);
  }

  return true;
}

bool PMManager::createCommandsForUpdateAllUsecases(QList<PMCommand*>& parCommandsList)
{
  foreach (PMInstance* curInstance, pm_)
  {
    // Commands for cloning and startup the instance and create a snapshot
    if(!createCommandsForOneInstance(curInstance, parCommandsList))
      return false;
  }

  return true;
}

bool PMManager::createCommandsToCloseMachine(QString useCaseName, QList<PMCommand*>& parCommandsList)
{
  QStringList args;

  PMCommand *curCommand;
  QString pmName = "pm_Usecase_" + useCaseName;

  args.clear();
  args.append("--nologo");  //suppress the logo
  args.append("controlvm");
  args.append(pmName);
  args.append("poweroff");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Poweroff VM");
  parCommandsList.append(curCommand);

  args.clear();
  args.append("--nologo");  //suppress the logo
  args.append("snapshot");
  args.append(pmName);
  args.append("restorecurrent");
  curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
  curCommand->setDescription("Restore Snapshot of VM");
  parCommandsList.append(curCommand);

  return true;
}

bool PMManager::createCommandsToStartInstance(QString parInstanceName, QList<PMCommand*>& parCommandsList)
{
  int waittime;

  for(int i=0;i<configUser_->getConfiguredUseCases().size();i++)
  {
    if(parInstanceName != configUser_->getConfiguredUseCases().at(i)->name)
      continue;

    ConfigUseCase* curUseCase = configUser_->getConfiguredUseCases().at(i);
    PMInstance* curInstance = createPMInst(curUseCase);
    runningInstances_.push_back(parInstanceName);
    QStringList args;
    PMCommand *curCommand;
    QString pmName="pm_Usecase_";
    pmName += curInstance->getConfig()->name;

    args.append("--nologo"); //suppress the logo
    args.append("modifyvm");
    args.append(pmName);
    args.append("--natpf1");
    args.append("delete");
    args.append("guestRDP");

    args.clear();
    args.append("startvm");
    args.append(pmName);
    args.append("--type");
    args.append("headless"); // 'gui' or 'headless'
    curCommand = new PMCommand(configSystem_->getVBoxCommand(), args, true, false);
    curCommand->setDescription("Startup VM-Clone for Usecase '" + curInstance->getConfig()->fullName + "'");
    curCommand->setCosts(configSystem_->getMachineRestoreTime());

    parCommandsList.append(curCommand);

    // Wait 10 seconds
    waittime = 10;
    for(int i = 0; i < waittime; i++)
    {
      curCommand = new PMCommand(1000);
      curCommand->setDescription("Wait "+QString::number(waittime-i)+"sec till still VM for Usecase '" + curInstance->getConfig()->fullName + "' is restored");
      parCommandsList.append(curCommand);
    }

    return true;
  }
  return false;
}
