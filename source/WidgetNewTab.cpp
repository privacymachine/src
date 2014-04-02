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

#include "WidgetCommandExec.h"
#include "UserConfig.h"
#include "PMInstance.h"
#include "PMManager.h"
#include "WidgetNewTab.h"
#include "ui_WidgetNewTab.h"


WidgetNewTab::WidgetNewTab(QWidget *parParent) :
  QWidget(parParent),
  ui_(new Ui::WidgetNewTab)
{
  ui_->setupUi(this);
  m_parent_ = parParent;
  pmManager_ = NULL;
  commandExec_ = new WidgetCommandExec(this);
  ui_->btnStartUsecase->setEnabled(false);
  ui_->execLayout->addWidget(commandExec_);


}

bool WidgetNewTab::init(PMManager *parPMManager)
{
  pmManager_ = parPMManager;

  QList<QString> useCaseNames;
  useCaseNames = pmManager_->getUseCaseNames();
  radioButtons_ = new QButtonGroup();
  for(int i=0; i<useCaseNames.size(); i++)
  {
    QRadioButton *temp= new QRadioButton(
                         useCaseNames.at(i),
                         this);
    radioButtons_->addButton(temp,i); // using i as Button-ID
    ui_->chooseLayout->addWidget(temp);

    connect(temp,
            SIGNAL(clicked()),
            this,
            SLOT(radioBtn_clicked()));
  }

  radioButtons_->setExclusive(true);

  connect(ui_->btnStartUsecase,
          SIGNAL(clicked()),
          this,
          SLOT(btnStart_clicked()));

  connect(commandExec_,
          SIGNAL(signalFinished(commandResult)),
          this,
          SLOT(handleCommandStatus(commandResult)));

  return true;
}

WidgetNewTab::~WidgetNewTab()
{
  delete commandExec_;
  while(radioButtons_->buttons().size())
    delete radioButtons_->buttons().takeFirst();

  delete ui_;
}

void WidgetNewTab::reset()
{
  QAbstractButton* checkedButton = radioButtons_->checkedButton();
  if (checkedButton)
  {
    radioButtons_->setExclusive(false);
    checkedButton->setChecked(false);
    radioButtons_->setExclusive(true);
  }

  commandExec_->reset();
  ui_->btnStartUsecase->setEnabled(false);
}

void WidgetNewTab::btnStart_clicked()
{
  int indexUsecase = radioButtons_->checkedId();
  if (indexUsecase >= 0)
  {
    QList<PMCommand*> commandList;

    if(!pmManager_->createCommandsToStartInstance(
         pmManager_->getUseCaseNames().at(indexUsecase),commandList))
    {
      //TODO: error handling
    }

    commandExec_->init(commandList);

    commandExec_->start();
  }
}

void WidgetNewTab::radioBtn_clicked()
{
  ui_->btnStartUsecase->setEnabled(true);
}

void WidgetNewTab::handleCommandStatus(commandResult exitCode)
{
  int useCaseId;
  switch(exitCode)
  {
    case success:
      useCaseId = radioButtons_->checkedId();
      if (useCaseId >= 0)
        emit newUseCaseReady(useCaseId);

      reset();
      break;

    case failed:

      break;

    case aborted:

      break;

    default:
      break;
  }
}
