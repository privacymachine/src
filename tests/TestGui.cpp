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


  /* TODO: fix after the beta-release
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

