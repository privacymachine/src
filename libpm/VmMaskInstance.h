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

#pragma once

#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <QList>

#include "VmMaskCurrentConfig.h"
#include "utils.h"

// forward declarations
class PmCommand;
class VmInfoIpAddress;

/// \brief A specific instance that is started using a certain VmMaskCurrentConfig
class VmMaskInstance
{
  public:

    /// \brief Constructor of \class VmMaskInstance
    /// \param parCurrentConfig [in] holded as reference
    /// \param parVmMaskId [in] id (identical to tab-index)
    explicit VmMaskInstance(VmMaskCurrentConfig* parCurrentConfig, int parVmMaskId);

    VmMaskCurrentConfig* getConfig()  { return currentConfig_; }

    QSharedPointer< VmInfoIpAddress > getInfoIpAddress()  { return vmInfoIpAddress_; }

    // copy of VmMaskUserConfig::vmMaskId_
    int getVmMaskId() { return vmMaskId_; }

    bool getVmMaskIsActive() const;
    void setVmMaskIsActive(bool value);

  private:
    int vmMaskId_;
    bool vmMaskIsActive_; // 'true'  when the VmMask is up and running

    VmMaskCurrentConfig *currentConfig_; // created and deleted outside (in PmManager)
    QSharedPointer< VmInfoIpAddress > vmInfoIpAddress_;
};

