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

#include "VmInfoIpAddress.h"
#include "VmMaskInstance.h"
#include "PmCommand.h"
#include "SystemConfig.h"
#include <QStringList>

VmMaskInstance::VmMaskInstance(VmMaskCurrentConfig* parCurrentConfig, int parVmMaskId)
{
  currentConfig_ = parCurrentConfig;
  vmInfoIpAddress_ = QSharedPointer<VmInfoIpAddress>(new VmInfoIpAddress(currentConfig_->getIpAddressProviders(),
                                                                         currentConfig_->getFullName(),
                                                                         currentConfig_->getBrowser(),
                                                                         currentConfig_->getSshPort()));
  vmInfoIpAddress_->initialize();
  vmMaskId_ = parVmMaskId;
}
