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

#pragma once

#include <QXmlStreamReader>
#include <QDomDocument>
#include <QDateTime>

#include "PmVersion.h"



/// \brief The XmlUpdateParser class
/// \brief XML Parser of the RSS-Feed which contains user readable info plus
/// \brief additional XML-Tags which are interpreted by the PrivacyMachine
class XmlUpdateParser
{
  public:

    /// \brief The UpdateType enum
    enum UpdateType{
      BaseDisk = 0,
      Binaries,
      Config,
    };

    /// \brief The CheckSumListBinary struct
    /// \brief used as QList enty of UpdateInfoBinary to store the OS depending update information
    struct CheckSumListBinary
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
    };

    /// \brief The UpdateInfoBinary struct
    /// \brief Stores the information of a possible Binary update
    struct UpdateInfoBinary
    {
      PmVersion Version;
      QString Title;
      QString Description;
      QDateTime Date;
      QList<CheckSumListBinary> CheckSums;
    };

    /// \brief The CheckSumListConfig struct
    /// \brief used as QList enty of UpdateInfoConfig to store the OS depending update information
    struct CheckSumListConfig
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
    };

    /// \brief The UpdateInfoConfig struct
    /// \brief Stores the information of a possible Config update
    struct UpdateInfoConfig
    {
      PmVersion Version;
      QString Title;
      QString Description;
      QDateTime Date;
      QList<CheckSumListConfig> CheckSums;
    };

    /// \brief The CheckSumListBaseDisk struct
    /// \brief used as QList enty of UpdateInfoBaseDisk to store the different possibilities to update to an new BaseDisk
    struct CheckSumListBaseDisk
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
      // when its a incremental update the BaseDisk ComponentMajor this delta referes to
      // when the same as UpdateInfoBaseDisk.Version.ComponentMajor then this enty referes the complete BaseDisk
      int ComponentMajorUp;
    };

    /// \brief The UpdateInfoBaseDisk struct
    /// \brief Stores the information of a possible BaseDisk update
    struct UpdateInfoBaseDisk
    {
      PmVersion Version;
      QString Title;
      QString Description;
      QDateTime Date;
      QList<CheckSumListBaseDisk> CheckSums;
    };

    /// \brief Constructor of \class VmMaskInstance
    /// \param parXmlDom    in: xml doc
    explicit XmlUpdateParser();

    /// \brief parse
    /// \brief Parses the XML-Data
    /// \param parRawData   [in]: raw data
    /// \return false on error
    bool parse(QByteArray parRawData);

    /// \brief getBinaryVersionList
    /// \return a list of possible Binary updates
    QList<XmlUpdateParser::UpdateInfoBinary> getBinaryUpdateList() {return binaries_;}

    /// \brief getBaseDiskVersionList
    /// \return a list of possible BaseDisk updates
    QList<XmlUpdateParser::UpdateInfoBaseDisk> getBaseDiskUpdateList() {return baseDisks_;}

    /// \brief getConfigVersionList
    /// \return a list of possible Config updates
    QList<XmlUpdateParser::UpdateInfoConfig> getConfigUpdateList() {return configs_;}

  private:
    QDomDocument xmlDom_;
    QList<UpdateInfoBinary> binaries_;
    QList<UpdateInfoBaseDisk> baseDisks_;
    QList<UpdateInfoConfig> configs_;
};

