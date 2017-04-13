#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include "../libpm/WidgetInteraktiveUpdate.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->progressBar->setRange(0,0);
  ui->labelSuccess->setVisible(false);
  ui->labelFail->setVisible(false);
  dl_ = new CheckUpdate(this);
  //dl_->setUrl("https://update.privacymachine.eu/test.xml");
  dl_->setUrl(QUrl::fromLocalFile("../UpdaterTest/test.xml"));

  connect(dl_,
          SIGNAL(finished()),
          this,
          SLOT(slotDownloaderFinished()));
  connect(dl_,
          SIGNAL(signalUpdateFound(Update)),
          this,
          SLOT(slotUpdateFound(Update)));

  dl_->start();

}

MainWindow::~MainWindow()
{
  delete ui;
  delete dl_;
}

void MainWindow::slotDownloaderFinished()
{
  ui->labelWait->setVisible(false);
  if (dl_->getError() == CheckUpdate::NoError)
    ui->labelSuccess->setVisible(true);
  else
    ui->labelFail->setVisible(true);

  if (true)
  {
  // test Update Manager
  updateManager_.setAppcastUrl(QUrl::fromLocalFile("../UpdaterTest/test.xml"));

  PmVersion cBinary, cBaseDisk, cConfig;

  cBinary.parse("0.10.4.1");
  cBaseDisk.parse("0.10.2.0");
  cConfig.parse("0.10.1.0");

  updateManager_.setCurrentBaseDiskVersion(cBaseDisk);
  updateManager_.setCurrentBinaryVersion(cBinary);
  updateManager_.setCurrentConfigVersion(cConfig);

  WidgetInteraktiveUpdate* updateWidget = updateManager_.getUpdateWidget();
  ui->showWidget->setVisible(false);
  ui->insertLayout->addWidget(updateWidget);
  updateWidget->show();
  updateManager_.findUpdates();
  }
}

void MainWindow::slotUpdateFound(Update avaiableUpdate)
{
  QMessageBox box;
  box.setStyleSheet("QLabel{min-width: 700px;}");
  box.setWindowTitle(avaiableUpdate.Title);
  QString Type;
  switch (avaiableUpdate.Type)
  {
    case Update::Binary:
      Type = "PrivacyMachine";
      break;
    case Update::BaseDisk:
      Type = "BaseDisk";
      break;
    case Update::Config:
      Type = "PM-configuration";
      break;
  }
  box.setText("<h1>New "+Type+" version is avaiable!</h1>");
  box.setInformativeText(avaiableUpdate.Description);
  box.exec();
}
