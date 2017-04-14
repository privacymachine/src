#include "mainwindow.h"
#include <QApplication>
#include "../libpm/XmlUpdateParser.h"

#include <QDomDocument>
#include <QFile>
#include <QProgressDialog>
#include <QCryptographicHash>

#include <QDateTime>

int main(int argc, char *argv[])
{

  QDateTime dt;
  QString format = "ddd, dd MMM yyyy HH:mm:ss";
  QString dateStr= "Fri, 17 Mar 2017 13:55:27 CET";
  dt = QDateTime::fromString(dateStr.left(dateStr.size()-4),format);

  qDebug() << dt.toString(format);
  qDebug() << "\n" <<dateStr.right(3);

  qDebug() << QCryptographicHash::hash(QByteArray(""), QCryptographicHash::Sha3_256).toHex();

  exit(0);

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
