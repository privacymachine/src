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

#include "PMInstance.h"
#include "PMCommand.h"
#include "SystemConfig.h"
#include <QStringList>

PMInstance::PMInstance(PMInstanceConfiguration *parConfig)
{
  config_ = parConfig;
}

PMCommand* PMInstance::getCommandToBindNatPort()
{
  QStringList args;

  QString pmName="";
  pmName += "pm_Usecase_";
  pmName += config_->name;
  QString natRule="";
  natRule += "guestRDP,tcp,,";
  natRule += QString::number(config_->rdpPort);  //host port
  natRule += ",,3389";                        //guest port (std rdp)

  args.append("--nologo"); //suppress the logo
  args.append("modifyvm");
  args.append(pmName);
  args.append("--natpf1");
  args.append(natRule);

  PMCommand *curCommand;
  QString description="Setup NAT rule for use case ";
  description += pmName;
  description += " to Port ";
  description += QString::number(config_->rdpPort);
  description += ".";
  curCommand = new PMCommand(
        config_->systemConfig->getVBoxCommand(),
        args,true,true);
  curCommand->setDescription(description);
  return curCommand;
}
