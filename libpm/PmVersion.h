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

#include <QRegExp>

/// \brief a Version struct, for easy comparisions
class PmVersion
{
  public:
    enum Index {
      Major = 0,
      Minor = 1,
      ComponentMajor = 2,
      ComponentMinor = 3
    };

    /// \brief Contructor
    PmVersion(): major_(0), minor_(0), componentMajor_(0), componentMinor_(0) {}


    /// \brief parses the string
    /// \param parVersionString  in: version which get's parsed
    bool parse(QString parVersionString);

    /// \brief Comparision Operator <
    bool operator < (const PmVersion& other);

    /// \brief Comparision Operator >
    bool operator > (const PmVersion& other);

    /// \brief Comparision Operator <=
    bool operator <= (const PmVersion& other);

    /// \brief Comparision Operator >=
    bool operator >= (const PmVersion& other);

    /// \brief Comparision Operator ==
    bool operator == (const PmVersion& other);

    /// \brief Acess Operator [PmVersion::Index]
    int& operator [](PmVersion::Index parDigit);

    /// \brief converts to readable string
    QString toString();


    bool isZero() {return major_==0 && minor_==0 && componentMajor_==0 && componentMinor_==0;}

    /// getter/setter

    int getMajor() const {return major_;}
    int getMinor() const {return minor_;}
    int getComponentMajor() const {return componentMajor_;}
    int getComponentMinor() const {return componentMinor_;}

    void setMajor(int major) {major_ = major;}
    void setMinor(int minor) {minor_ = minor;}
    void setComponentMajor(int componentMajor) {componentMajor_ = componentMajor;}
    void setComponentMinor(int componentMinor) {componentMinor_ = componentMinor;}



    /// \brief greaterInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       in: left side Argument
    /// \param arg2       in: right side Argument
    /// \param parDigit   in: Version Index
    /// \return           true if version numbers before parDigit are the same AND version numbers from parDigit on are greater
    static bool greaterInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);

    /// \brief smallerInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       in: left side Argument
    /// \param arg2       in: right side Argument
    /// \param parDigit   in: Version Index
    /// \return           true if version numbers before parDigit are the same AND version numbers from parDigit on are smaller
    static bool smallerInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);

    /// \brief sameInRespectTo
    /// \brief compares two PmVersion instances with respect to a Version Index
    /// \param arg1       in: left side Argument
    /// \param arg2       in: right side Argument
    /// \param parDigit   in: Version Index
    /// \return           true if version numbers till, including parDigit are the same
    static bool sameInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit);


  private:

    int major_;
    int minor_;
    int componentMajor_;
    int componentMinor_;
};

