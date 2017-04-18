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

#include <stdexcept>
#include <QRegularExpression>
#include "PmVersion.h"

#include "utils.h"

// Comperation functions
bool PmVersion::greaterInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit)
{
  bool greaterMajor = arg1.major_ > arg2.major_;
  bool greaterMinor = arg1.minor_ > arg2.minor_;
  bool greaterComponentMajor = arg1.componentMajor_ > arg2.componentMajor_;
  bool greaterComponentMinor = arg1.componentMinor_ > arg2.componentMinor_;

  bool sameMajor = arg1.major_ == arg2.major_;
  bool sameMinor = arg1.minor_ == arg2.minor_;
  bool sameComponentMajor = arg1.componentMajor_ == arg2.componentMajor_;

  switch (parDigit)
  {
    case PmVersion::Major:
      if( (greaterMajor) || (sameMajor && greaterMinor) || (sameMajor && sameMinor && greaterComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && greaterComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::Minor:
      if( (sameMajor && greaterMinor) || (sameMajor && sameMinor && greaterComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && greaterComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMajor:
      if( (sameMajor && sameMinor && greaterComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && greaterComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMinor:
      if( (sameMajor && sameMinor && sameComponentMajor && greaterComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    default:
      QString error = "greaterInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit): PmVersion::Index parDigit: \""
                      + QString(parDigit) +"\" is not a valid argument";
      IERR(error);
      throw std::invalid_argument(error.toStdString());
      break;
  }
}

bool PmVersion::smallerInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit)
{
  bool smallerMajor = arg1.major_ < arg2.major_;
  bool smallerMinor = arg1.minor_ < arg2.minor_;
  bool smallerComponentMajor = arg1.componentMajor_ < arg2.componentMajor_;
  bool smallerComponentMinor = arg1.componentMinor_ < arg2.componentMinor_;

  bool sameMajor = arg1.major_ == arg2.major_;
  bool sameMinor = arg1.minor_ == arg2.minor_;
  bool sameComponentMajor = arg1.componentMajor_ == arg2.componentMajor_;

  switch (parDigit)
  {
    case PmVersion::Major:
      if( (smallerMajor) || (sameMajor && smallerMinor) || (sameMajor && sameMinor && smallerComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && smallerComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::Minor:
      if( (sameMajor && smallerMinor) || (sameMajor && sameMinor && smallerComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && smallerComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMajor:
      if( (sameMajor && sameMinor && smallerComponentMajor) ||
          (sameMajor && sameMinor && sameComponentMajor && smallerComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMinor:
      if( (sameMajor && sameMinor && sameComponentMajor && smallerComponentMinor) )
      {
        return true;
      }
      else return false;
      break;
    default:
      QString error = "smallerInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit): PmVersion::Index parDigit: \""
                      + QString(parDigit) +"\" is not a valid argument";
      IERR(error);
      throw std::invalid_argument(error.toStdString());
      break;
  }
}

bool PmVersion::sameInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit)
{
  bool sameMajor = arg1.major_ == arg2.major_;
  bool sameMinor = arg1.minor_ == arg2.minor_;
  bool sameComponentMajor = arg1.componentMajor_ == arg2.componentMajor_;
  bool sameComponentMinor = arg1.componentMinor_ == arg2.componentMinor_;

  switch (parDigit)
  {
    case PmVersion::Major:
      if( sameMajor )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::Minor:
      if( sameMajor && sameMinor )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMajor:
      if( sameMajor && sameMinor && sameComponentMajor )
      {
        return true;
      }
      else return false;
      break;
    case PmVersion::ComponentMinor:
      if( sameMajor && sameMinor && sameComponentMajor && sameComponentMinor )
      {
        return true;
      }
      else return false;
      break;
    default:
      QString error = "sameInRespectTo( const PmVersion& arg1, const PmVersion& arg2, PmVersion::Index parDigit): PmVersion::Index parDigit: \""
                      + QString(parDigit) +"\" is not a valid argument";
      IERR(error);
      throw std::invalid_argument(error.toStdString());
      break;
  }
}

bool PmVersion::parse(QString parVersionString)
{
  major_ = 0;
  minor_ = 0;
  componentMajor_ = 0;
  componentMinor_ = 0;

  QRegularExpression regexVersion( "(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
  QRegularExpressionMatch match = regexVersion.match(parVersionString);
  if (match.hasMatch())
  {
    major_ = match.captured(1).toUInt();
    minor_ = match.captured(2).toUInt();
    componentMajor_ = match.captured(3).toUInt();
    componentMinor_ = match.captured(4).toUInt();
    return true;
  }
  else
  {
    return false;
  }
}

int& PmVersion::operator [](PmVersion::Index parDigit)
{
  switch (parDigit)
  {
    case PmVersion::Major:
      return major_;
      break;
    case PmVersion::Minor:
      return minor_;
      break;
    case PmVersion::ComponentMajor:
      return componentMajor_;
      break;
    case PmVersion::ComponentMinor:
      return componentMinor_;
      break;
    default:
      QString error = "PmVersion::operator [](PmVersion::Index parDigit): PmVersion::Index parDigit: \"" +
                      QString(parDigit) +"\" is not a valid argument";
      IERR(error);
      throw std::invalid_argument(error.toStdString());
      break;
  }
}

bool PmVersion::operator <= (const PmVersion& other)
{
  return (operator <(other)) || (operator ==(other));
}

bool PmVersion::operator >= (const PmVersion& other)
{
  return (operator >(other)) || (operator ==(other));
}

bool PmVersion::operator < (const PmVersion& other)
{
  if (major_ < other.major_)
  {
    return true;
  }
  else if (major_ == other.major_)
  {
    if (minor_ < other.minor_)
    {
      return true;
    }
    else if(minor_ == other.minor_)
    {
      if (componentMajor_ < other.componentMajor_)
      {
        return true;
      }
      else if((componentMajor_ == other.componentMajor_))
      {
        if (componentMinor_ < other.componentMinor_)
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool PmVersion::operator > (const PmVersion& other)
{
  if (major_ > other.major_)
  {
    return true;
  }
  else if (major_ == other.major_)
  {
    if (minor_ > other.minor_)
    {
      return true;
    }
    else if(minor_ == other.minor_)
    {
      if (componentMajor_ > other.componentMajor_)
      {
        return true;
      }
      else if((componentMajor_ == other.componentMajor_))
      {
        if (componentMinor_ > other.componentMinor_)
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool PmVersion::operator == (const PmVersion& other)
{
  return major_ == other.major_
    && minor_ == other.minor_
    && componentMajor_ == other.componentMajor_
    && componentMinor_ == other.componentMinor_;
}

QString PmVersion::toString()
{
  QString version = QString::number(major_) + "." +
                    QString::number(minor_) + "." +
                    QString::number(componentMajor_) + "." +
                    QString::number(componentMinor_);

  return version;
}
