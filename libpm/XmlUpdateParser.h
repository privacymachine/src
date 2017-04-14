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

#include <QXmlStreamReader>
#include <QDomDocument>
#include <QDateTime>

#include "PmVersion.h"


/// \brief XML Parser of the RSS-Feed which contains user readable info plus
/// \brief additional XML-Tags which are interpreted by the PrivacyMachine
class XmlUpdateParser
{
  public:

    enum UpdateType{
      BaseDisk = 0,
      Binaries,
      Config,
    };

    struct CheckSumListBinary
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
    };

    struct UpdateInfoBinary
    {
      PmVersion Version;
      QString Title;
      QString Description;
      QDateTime Date;
      QList<CheckSumListBinary> CheckSums;
    };

    struct CheckSumListConfig
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
    };

    struct UpdateInfoConfig
    {
      PmVersion Version;
      QString Title;
      QString Description;
      QDateTime Date;
      QList<CheckSumListConfig> CheckSums;
    };

    struct CheckSumListBaseDisk
    {
      QString Os;
      QString Url; // validity not checked
      QString CheckSum; // in hex
      int ComponentMajorUp;
    };

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

    /// \brief Parses the XML-Data
    /// \param parRawData   in: raw data
    /// \return false on error
    bool parse(QByteArray parRawData);

    QList<XmlUpdateParser::UpdateInfoBinary> getBinaryVersionList() {return binaries_;}

    QList<XmlUpdateParser::UpdateInfoBaseDisk> getBaseDiskVersionList() {return baseDisks_;}

    QList<XmlUpdateParser::UpdateInfoConfig> getConfigVersionList() {return configs_;}

    /// \brief get the latest binary update details
    /// \return zero pointer if no new binary is available
    XmlUpdateParser::UpdateInfoBinary* getLatestBinaryVersion();

    /// \brief get the latest BaseDisk update details
    /// \return zero pointer if no new binary is available
    XmlUpdateParser::UpdateInfoBaseDisk* getLatestBaseDiskVersion();

    /*
    bool getLatestPmBinary("Win64", QString& parUrlm QString& parCheckSum);
    int getLatestPmConfigVersionString();
    bool getLatestPmconfig("Win64", QString& parUrlm QString& parCheckSum);

    int getLatestPmBaseDiskVersionString();
    bool getLatestPmconfig(QList<BaseDiskUpdate>& parBasDiskUpdates);
    */

  private:
    QDomDocument xmlDom_;
    QList<UpdateInfoBinary> binaries_;
    QList<UpdateInfoBaseDisk> baseDisks_;
    QList<UpdateInfoConfig> configs_;
};

