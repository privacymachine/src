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

#pragma once

#include "PMInstanceConfiguration.h"

#include <QWidget>
#include <QList>

class PMCommand;

/// A specific instance that is started using a certain PMInstanceConfiguration
class PMInstance
{
  public:
    explicit PMInstance(PMInstanceConfiguration *parConfig = 0);
    PMCommand *getCommandToBindNatPort();
    PMInstanceConfiguration* getConfig()  { return config_; }

  private:
    PMInstanceConfiguration *config_;
};

