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

#include <QtTest>

#include "../libpm/XmlUpdateParser.h"

/// \brief Test the class XmlUpdateParser which is responsible for parsing the the update rss feed
class TestXmlUpdateParser : public QObject
{
    Q_OBJECT
  public:

  private slots:


    /// \brief called from QTestLib before first test function is called
    void initTestCase() {}

    /// \brief called from QTestLib after last test function is called
    void cleanupTestCase() {}

    /// \brief called from QTestLib before each test function is called
    void init() {}

    /// \brief called from QTestLib after each test function is called
    void cleanup() {}


    /// \brief  a simple xmp parser test
    void testParseSimpleXml();
};

