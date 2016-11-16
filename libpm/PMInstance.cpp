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

#include "PMInstance.h"
#include "PMCommand.h"
#include "SystemConfig.h"
#include <QStringList>

PMInstance::PMInstance(PMInstanceConfiguration *parConfig)
{
  config_ = parConfig;
  vmInfoIpAddress_ = QSharedPointer< VmInfoIpAddress >( new VmInfoIpAddress( this ) );
  vmInfoIpAddress_->initialize();

}

PMCommand* PMInstance::getCommandToBindNatPort()
{
  QStringList args;
  QString natRule;

  args.append("--nologo"); //suppress the logo

  args.append("modifyvm");
  args.append(config_->vmName);
  /* FreeRDP-Server is disabled now
  args.append("--natpf1");
  natRule = "guestRDP,tcp," + QString( constVmIp ) + ",";          // only on local interface
  natRule += QString::number(config_->rdpPort); // host port
  natRule += ",,3389";                          // guest port (std rdp)
  args.append(natRule);
  */
  args.append("--natpf1");
  natRule = "guestSSH,tcp," + QString( constVmIp ) + ",";          // only on local interface
  natRule += QString::number(config_->sshPort); // host port
  natRule += ",,22";                          // guest port (std ssh)
  args.append(natRule);

  PMCommand *curCommand;
  QString description="Setup NAT rule for use case ";
  description += config_->vmName;
  description += " to Port ";
  description += QString::number(config_->rdpPort);
  description += ".";
  curCommand = new PMCommand(
        config_->systemConfig->getVBoxCommand(),
        args,true,true);
  curCommand->setDescription(description);
  return curCommand;
}
