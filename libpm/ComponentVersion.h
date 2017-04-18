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

#ifndef COMPONENT_VERSION_H
#define COMPONENT_VERSION_H

#include <QString>

struct ComponentVersion
{
  ComponentVersion( uint newMajor, uint newMinor, uint newComponentMajor, uint newComponentMinor )
  { 
    major = newMajor;
    minor = newMinor;
    componentMajor = newComponentMajor;
    componentMinor = newComponentMinor;

  }

  QString ToString()
  {
    return QString::number( major ) + "." 
      + QString::number( minor ) + "." 
      + QString::number( componentMajor ) + "." 
      + QString::number( componentMinor );

  }

  ComponentVersion( const ComponentVersion& original )
  {
    major = original.major;
    minor = original.minor;
    componentMajor = original.componentMajor;
    componentMinor = original.componentMinor;
    
  }

  uint major;
  uint minor;
  uint componentMajor;
  uint componentMinor;

};


#endif // COMPONENT_VERSION_H
