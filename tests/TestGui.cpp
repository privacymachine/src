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

#include <QtWidgets>
#include <QtTest/QtTest>

#include "../libpm/WindowMain.h"

class TestGui: public QObject
{
    Q_OBJECT

  private slots:
      void testGui();

};

void TestGui::testGui()
{

  /* Sample with Widgets
  QLineEdit lineEdit;
  QTest::keyClicks(&lineEdit, "hello world");
  QCOMPARE(lineEdit.text(), QString("hello world"));
  */


  /** TODO: fix after the beta-release
  WindowMain mainWindow;
  if (mainWindow.init(QDir::currentPath()))
  {
    mainWindow.show();
  }
  else
  {
    QFAIL("mainWindow.init() failed");
  }
  */

}

QTEST_MAIN(TestGui)
#include "TestGui.moc"

