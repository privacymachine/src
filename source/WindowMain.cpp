/*==============================================================================
        Copyright (c) 2013-2014 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

 Licensed under the EUPL, Version 1.1 or - as soon they will be approved by the
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

#include "WindowMain.h"
#include "ui_WindowMain.h"
#include "rdp/WidgetRdpView.h"
#include <QMessageBox>
#include <QProcess>


WindowMain::WindowMain(QWidget *parParent) :
  QMainWindow(parParent),
  pmManager_(0),
  ui_(new Ui::WindowMain())
{
  ui_->setupUi(this);
  questionBox_ = NULL;
  updateMessage_ = NULL;
  updateWidget_ = NULL;
  tabWidget_ = NULL;
}

bool WindowMain::init()
{
  // set some window title
  slotUpdateProgress("startup...");

  pmManager_ = new PMManager();
  if(!pmManager_->init()) return false;

  bool updateAvailable=true;
  // TODO: Check for Updates
  if(updateAvailable)
  {
    updateMessage_ = new QLabel(this);
    updateMessage_->setText("An update is available! \n Install ist now?");
    updateMessage_->setFont(QFont("Sans",18));
    updateMessage_->setAlignment(Qt::AlignHCenter);
    ui_->mainLayout_v->setAlignment(Qt::AlignHCenter);
    ui_->mainLayout_v->addWidget(updateMessage_);
    questionBox_ = new QDialogButtonBox(this);
    questionBox_->addButton(QDialogButtonBox::Ok);
    questionBox_->addButton("&Later",QDialogButtonBox::RejectRole);
    ui_->mainLayout_v->addWidget(questionBox_);

    connect(questionBox_,
            SIGNAL(accepted()),
            this,
            SLOT(slotUpdateBtnOK()));

    connect(questionBox_,
            SIGNAL(rejected()),
            this,
            SLOT(slotUpdateBtnLater()));
  }
  else
  {
    return setupTabWidget();
  }

  return true;
}

void WindowMain::slotUpdateBtnLater()
{
  ui_->mainLayout_v->removeWidget(updateMessage_);
  ui_->mainLayout_v->removeWidget(questionBox_);
  questionBox_->close();
  updateMessage_->close();
  delete questionBox_;
  questionBox_ = NULL;
  delete updateMessage_;
  updateMessage_ = NULL;

  setupTabWidget();
}

void WindowMain::slotUpdateBtnOK()
{
  ui_->mainLayout_v->removeWidget(updateMessage_);
  ui_->mainLayout_v->removeWidget(questionBox_);
  questionBox_->close();
  updateMessage_->close();
  delete questionBox_;
  questionBox_ = NULL;
  delete updateMessage_;
  updateMessage_ = NULL;

  updateWidget_ = new WidgetUpdate(this);
  if (!updateWidget_->init(pmManager_))
  {
    IERR("failed to initialize updateWidget");
    delete updateWidget_;
    updateWidget_ = NULL;
    return;
  }

  connect(updateWidget_,
          SIGNAL(signalUpdateFinished(commandResult)),
          this,
          SLOT(slotUpdateFinished(commandResult)));

  ui_->mainLayout_v->addWidget(updateWidget_);

  updateWidget_->start();
}

void WindowMain::slotNewUseCaseStarted(int parIndexUseCase)
{
  if (parIndexUseCase < 0) return;

  PMInstance* curPMInstance = pmManager_->getInstances().at(parIndexUseCase);

  // create a new RdpView as tab
  QUrl url;
  url.setHost("localhost");
  url.setEncodedUserName("pm");
  url.setPort(curPMInstance->getConfig()->rdpPort);
  url.setPassword("123");
  ILOG("connect via rdp to port " + QString::number(curPMInstance->getConfig()->rdpPort));

  WidgetRdpView *rdpView = new WidgetRdpView(url);
  int indexBeforeLast = tabWidget_->count() - 1;
  tabWidget_->insertTab(indexBeforeLast, rdpView, curPMInstance->getConfig()->fullName);
  tabWidget_->setCurrentIndex(tabWidget_->count()-2);
  rdpView->connectViaRdp();
}

void WindowMain::slotUpdateProgress(QString parProgress)
{
  QString title = "PrivacyMachine";
  if (updateWidget_ != NULL)
  {
    title += " - Update: ";
    title += parProgress;
  }
  setWindowTitle(title);
}


void WindowMain::slotTabCloseRequested(int parIndex)
{
  QWidget* curWidget = tabWidget_->widget(parIndex);

  // If we get the property 'instance_index', it is a RdpView-Widget
  QVariant val = curWidget->property("instance_index");
  if (val.isValid())
  {
    int indexUseCase = val.toInt();
    PMInstance* curPMInstance = pmManager_->getInstances().at(indexUseCase);
    // The poweroff command is executed wihtout showing any window
    QList<PMCommand*> commandsList;
    pmManager_->createCommandsToCloseMachine(curPMInstance->getConfig()->name, commandsList);
    foreach(PMCommand* cmd, commandsList)
    {
      cmd->executeBlocking(false);
    }

    // TODO: free memory
    /*
    while commandsList.size()
    {
      commandsList.last();
    }
    */
  }
  tabWidget_->removeTab(parIndex);
  delete curWidget;
}

bool WindowMain::setupTabWidget()
{
  tabWidget_ = new QTabWidget(this);
  tabWidget_->setTabsClosable(true);
  WidgetNewTab *newTab = new WidgetNewTab(this);
  newTab->init(pmManager_);
  ui_->mainLayout_v->addWidget(tabWidget_);
  tabWidget_->addTab(newTab,QIcon(":/images/ApplicationIcon32.png"),"+");
  connect(tabWidget_,
          SIGNAL(tabCloseRequested(int)),
          this,
          SLOT(slotTabCloseRequested(int)));

  connect(newTab,
          SIGNAL(newUseCaseReady(int)),
          this,
          SLOT(slotNewUseCaseStarted(int)));

  return true;
}

WindowMain::~WindowMain()
{
  if (pmManager_) delete pmManager_;
  delete ui_;
}

void WindowMain::show()
{
  setWindowIcon(QIcon(":/images/ApplicationIcon32.png"));
  QMainWindow::show();

}

void WindowMain::slotUpdateFinished(commandResult parResult)
{
  QString message="";

  switch (parResult)
  {
    case aborted:
      message+="Update aborted.";
      break;

    case success:
      message+="Update successful.";
      break;

    default: // failed
      message+="Update failed. Please check the logfile for Details.";
  }

  setWindowTitle("PrivacyMachine " + message);

  QMessageBox::information(this, "PrivacyMachine", message);
  /*
  QMessageBox msgBox;
  msgBox.setInformativeText(message);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  int ret = msgBox.exec();
  */

  ui_->mainLayout_v->removeWidget(updateWidget_);
  delete updateWidget_;
  updateWidget_=NULL;
  setupTabWidget();
}

void WindowMain::closeEvent(QCloseEvent * parEvent)
{
  if (updateWidget_ != NULL)
  {
    // Stop the running update
    updateWidget_->abort();
    ui_->mainLayout_v->removeWidget(updateWidget_);
    delete updateWidget_;
    updateWidget_=NULL;
  }
}

