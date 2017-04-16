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

#include <QRegExp>



/// \brief The PmVersion class
/// \brief a Version class, for easy comparisions
class PmVersion
{
  public:

    /// \brief The Index enum which identifies the different version numbers
    enum Index {
      Major = 0,
      Minor = 1,
      ComponentMajor = 2,
      ComponentMinor = 3
    };

    PmVersion(): major_(0), minor_(0), componentMajor_(0), componentMinor_(0) {}


    /// \brief parse
    /// \brief parses a string into a PmVersion
    /// \param parVersionString [in]: version which get's parsed
    /// \return true if parsing was successfully
    bool parse(QString parVersionString);


    /// \brief operator <
    /// \brief Comparision Operator <
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is bigger
    bool operator < (const PmVersion& other);

    /// \brief operator >
    /// \brief Comparision Operator >
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is smaller
    bool operator > (const PmVersion& other);

    /// \brief operator <=
    /// \brief Comparision Operator <=
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is bigger or equal
    bool operator <= (const PmVersion& other);

    /// \brief operator >=
    /// \brief Comparision Operator >=
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is smaller or equal
    bool operator >= (const PmVersion& other);


    /// \brief operator ==
    /// \brief Comparision Operator ==
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is equal
    bool operator == (const PmVersion& other);

    /// \brief operator !=
    /// \brief Comparision Operator !=
    /// \param other [in]: PmVersion
    /// \return true if other PmVersion is not equal
    bool operator != (const PmVersion& other) {return !operator ==(other);}

    /// \brief operator []
    /// \brief Acess Operator [PmVersion::Index]
    /// \param parDigit [in]: PmVersion::Index
    /// \return the version number corresponding to parDigit
    int& operator [](PmVersion::Index parDigit);

    /// \brief toString
    /// \brief converts to readable string
    /// \return QString
    QString toString();

    /// \brief isZero
    /// \return true if all version numbers are zero
    bool isZero() {return major_==0 && minor_==0 && componentMajor_==0 && componentMinor_==0;}


    /// getter/setter

    /// \brief getMajor
    /// \return major version number
    int getMajor() const {return major_;}

    /// \brief getMinor
    /// \return minor version number
    int getMinor() const {return minor_;}

    /// \brief getComponentMajor
    /// \return component major version number
    int getComponentMajor() const {return componentMajor_;}

    /// \brief getComponentMinor
    /// \return component minor version number
    int getComponentMinor() const {return componentMinor_;}

    /// \brief setMajor
    /// \brief sets the major version number
    /// \param major [in]: major version number
    void setMajor(int major) {major_ = major;}

    /// \brief setMinor
    /// \brief sets the minor version number
    /// \param minor [in]: minor version number
    void setMinor(int minor) {minor_ = minor;}

    /// \brief setComponentMajor
    /// \brief sets the component major version number
    /// \param componentMajor [in]: component major version number
    void setComponentMajor(int componentMajor) {componentMajor_ = componentMajor;}

    /// \brief setComponentMinor
    /// \brief sets the component minor version number
    /// \param componentMinor [in]: component minor version number
    void setComponentMinor(int componentMinor) {componentMinor_ = componentMinor;}



    /// \brief greaterInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       [in]: left side Argument
    /// \param arg2       [in]: right side Argument
    /// \param parDigit   [in]: PmVersion::Index
    /// \return           true if version numbers before parDigit are the same AND version numbers from parDigit on are greater
    static bool greaterInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);

    /// \brief smallerInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       [in]: left side Argument
    /// \param arg2       [in]: right side Argument
    /// \param parDigit   [in]: PmVersion::Index
    /// \return           true if version numbers before parDigit are the same AND version numbers from parDigit on are smaller
    static bool smallerInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);

    /// \brief sameInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       [in]: left side Argument
    /// \param arg2       [in]: right side Argument
    /// \param parDigit   [in]: PmVersion::Index
    /// \return           true if version numbers till, including parDigit are the same
    static bool sameInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);

    /// \brief fromString
    /// \brief parses a string into a PmVersion
    /// \param parVersionString [in]: version which get's parsed
    /// \return PmVersion
    static PmVersion fromString(QString parVersionString)
    {
      PmVersion result;
      result.parse(parVersionString);
      return result;
    }

  private:

    int major_;
    int minor_;
    int componentMajor_;
    int componentMinor_;
};

