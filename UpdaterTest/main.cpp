#include "mainwindow.h"
#include <QApplication>
#include "../libpm/XmlUpdateParser.h"

#include <QDomDocument>
#include <QFile>
#include <QProgressDialog>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  /// @todo: bernhard: move to unit test

  QFile file("../UpdaterTest/test.xml");
  if (!file.open(QIODevice::ReadOnly))
      return -1;

  XmlUpdateParser parser;
  if (!parser.parse(file.readAll()))
    return -1;

  file.close();

  XmlUpdateParser::UpdateInfoBinary* newBinary = parser.getLatestBinaryVersion();
  if (newBinary != 0)
    ILOG("new binary available: " + newBinary->Version.toString());



  a.setApplicationVersion("0.10.0.1");


  MainWindow w;
  w.show();
//  QProgressDialog d;
//  d.setRange(0,0);
//  d.show();

  return a.exec();
}
