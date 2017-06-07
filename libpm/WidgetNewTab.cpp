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

#include <QMessageBox>

#include "WidgetCommandExec.h"
#include "UserConfig.h"
#include "VmMaskInstance.h"
#include "PmManager.h"
#include "WidgetNewTab.h"
#include "ui_WidgetNewTab.h"
#include "utils.h"


WidgetNewTab::WidgetNewTab(QWidget *parParent) :
  QWidget(parParent),
  ui_(new Ui::WidgetNewTab),
  currentSelectedVmMaskId_(-1)
{
  ui_->setupUi(this);
  m_parent_ = parParent;
  pmManager_ = nullptr;
  commandExec_ = new WidgetCommandExec(this);
  ui_->btnStartVmMask->setEnabled(false);
  ui_->execLayout->addWidget(commandExec_);
}

void WidgetNewTab::connectSignalsAndSlots()
{
  for( QAbstractButton *rbItem : radioButtons_->buttons() )
  {
    connect( rbItem,
             &QAbstractButton::clicked,
             this,
             &WidgetNewTab::slotRadioBtn_clicked);
  }

  connect( ui_->btnStartVmMask,
           &QAbstractButton::clicked,
           this,
           &WidgetNewTab::slotBtnStart_clicked);

  connect( commandExec_,
           &WidgetCommandExec::signalFinished,
           this,
           &WidgetNewTab::slotFinished);
}

void WidgetNewTab::disconnectSignalsAndSlots()
{
  for(QAbstractButton *rbItem : radioButtons_->buttons())
  {
    disconnect( rbItem,
                &QAbstractButton::clicked,
                this,
                &WidgetNewTab::slotRadioBtn_clicked);
  }

  disconnect(ui_->btnStartVmMask,
             &QAbstractButton::clicked,
             this,
             &WidgetNewTab::slotBtnStart_clicked);

  disconnect(commandExec_,
             &WidgetCommandExec::signalFinished,
             this,
             &WidgetNewTab::slotFinished);

}

bool WidgetNewTab::init(PmManager *parPmManager)
{
  pmManager_ = parPmManager;
  radioButtons_ = new QButtonGroup();
  ui_->verticalLayout_2->setAlignment(Qt::AlignTop);
  
  // Add a radiobutton for each VmMask
  for (int vmMaskId = 0; vmMaskId < pmManager_->getVmMaskData().count(); vmMaskId++)
  {
    VmMaskData* vmMask = pmManager_->getVmMaskData()[vmMaskId];
    QRadioButton *rbItem = new QRadioButton(vmMask->UserConfig->getFullName(), this);
    radioButtons_->addButton(rbItem, vmMaskId);
    ui_->chooseLayout->addWidget(rbItem);
  }
  radioButtons_->setExclusive(true);
  ui_->btnStartVmMask->setEnabled(false);

  connectSignalsAndSlots();
  commandExec_->connectSignalsAndSlots();

  // If there is only one configured VM-Mask select it
  if ( radioButtons_->buttons().count() == 1 )
  {
    radioButtons_->buttons().first()->click();
  }

  return true;
}

WidgetNewTab::~WidgetNewTab()
{
  disconnectSignalsAndSlots();

  if (commandExec_)
  {
    delete commandExec_;
    commandExec_ = nullptr;
  }

  while(radioButtons_->buttons().size())
    delete radioButtons_->buttons().takeLast();

  if (ui_)
  {
    delete ui_;
    ui_ = nullptr;
  }
}

void WidgetNewTab::slotBtnStart_clicked()
{
  ui_->btnStartVmMask->setEnabled(false);
  currentSelectedVmMaskId_ = radioButtons_->checkedId();
  if (currentSelectedVmMaskId_ >= 0)
  {
    QList<PmCommand*> commandList;
    VmMaskData* vmMask = pmManager_->getVmMaskData()[currentSelectedVmMaskId_];

    if(!pmManager_->createCommandsToStartVmMask(currentSelectedVmMaskId_,
                                                commandList))
    {
      IWARN("error creating commands for VM-Mask " + vmMask->UserConfig->getName());
      slotFinished(failed);
      return;
    }

    // mark VmMaks as active
    vmMask->Instance->setVmMaskIsActive(true);

    commandExec_->setCommands(commandList);
    commandExec_->start();

    // What happens next?
    // the commandExec_ sends a signal signalFinished(ePmCommandResult)) to the slot slotFinished() indicating
    // a success or failure in ePmCommandResult

    // disable all radio buttons in the meantime
    for (QAbstractButton* btn : radioButtons_->buttons())
    {
      btn->setEnabled(false);
    }
  }
}

void WidgetNewTab::slotFinished(ePmCommandResult parExitCode)
{
  if (parExitCode == failed)
  {
    QMessageBox::warning(this, "Starting VM-Mask", "Starting the VirtualMachine failed. Please check the logfile for Details.");
  }

  if (parExitCode == failed || parExitCode == aborted )
  {
    // Mark the VmMask as inactive
    if (currentSelectedVmMaskId_ >= 0)
      pmManager_->getVmMaskData()[currentSelectedVmMaskId_]->Instance->setVmMaskIsActive(false);
  }

  // enable/disable all radio buttons based on the active status of the VmMasks
  for (int i = 0; i < pmManager_->getVmMaskData().count(); i++)
  {
    if (pmManager_->getVmMaskData()[i]->Instance != nullptr && pmManager_->getVmMaskData()[i]->Instance->getVmMaskIsActive())
      radioButtons_->button(i)->setEnabled(false);
    else
      radioButtons_->button(i)->setEnabled(true);
  }

  if (parExitCode == success)
  {
    radioButtons_->button(currentSelectedVmMaskId_)->setChecked(false);

    ui_->labelDescription->clear();
    ui_->labelDescription->adjustSize();
    emit signalNewVmMaskReady(currentSelectedVmMaskId_);

    commandExec_->reset();

    ui_->btnStartVmMask->setEnabled(false);
  }
  else
  {
    ui_->btnStartVmMask->setEnabled(true);
  }

}

void WidgetNewTab::slotVmMaskClosed(int parVmMaskId)
{
  if ( parVmMaskId >= 0 && parVmMaskId < radioButtons_->buttons().count() )
  {
    radioButtons_->button(parVmMaskId)->setEnabled(true);
  }
}

void WidgetNewTab::slotRadioBtn_clicked()
{
  // The VmMask-Id equals the RadioButton-Id
  int vmMaskId = radioButtons_->checkedId();

  if( vmMaskId >= 0 && vmMaskId < pmManager_->getVmMaskData().count() )
  {
    VmMaskData* vmMaskData = pmManager_->getVmMaskData()[vmMaskId];
    ui_->labelDescription->setText(vmMaskData->UserConfig->getDescription());

    //TODO: @Bernhard: this caused a bug when you click a radio button multiple times, why is this neccesary?
//    ui_->labelDescription->adjustSize(); // From http://www.qtcentre.org/threads/2593-Auto-resize-QLabel

    // cannot start if the some VmMask is currently created or if the current VmMask is already running
    if(commandExec_->isStillExecuting() ||
        (!vmMaskData->Instance.isNull() && vmMaskData->Instance->getVmMaskIsActive() ))
    {
      ui_->btnStartVmMask->setEnabled(false);
    }
    else
    {
      ui_->btnStartVmMask->setEnabled(true);
    }
  }
  else
  {
    ui_->labelDescription->clear();
    ui_->labelDescription->adjustSize();
    ui_->btnStartVmMask->setEnabled(false);
  }
}

