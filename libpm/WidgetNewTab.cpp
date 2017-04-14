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
  pmManager_ = NULL;
  commandExec_ = new WidgetCommandExec(this);
  ui_->btnStartVmMask->setEnabled(false);
  ui_->execLayout->addWidget(commandExec_);
}

void WidgetNewTab::connectSignalsAndSlots()
{
  foreach( QAbstractButton *rbItem, radioButtons_->buttons() )
  {
    connect( rbItem, SIGNAL( clicked() ), this, SLOT( slotRadioBtn_clicked() ) );
  }

  connect( ui_->btnStartVmMask, SIGNAL( clicked() ), this, SLOT( slotBtnStart_clicked() ) );
  connect( commandExec_, SIGNAL( signalFinished(ePmCommandResult) ), this, SLOT( slotFinished(ePmCommandResult) ) );
}

void WidgetNewTab::disconnectSignalsAndSlots()
{
  foreach(QAbstractButton *rbItem, radioButtons_->buttons())
  {
    disconnect( rbItem, SIGNAL( clicked() ), 0, 0 );
  }

  disconnect(ui_->btnStartVmMask, SIGNAL( clicked() ), 0, 0);
  disconnect(commandExec_, SIGNAL( signalFinished(ePmCommandResult) ), 0, 0);

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
    commandExec_ = NULL;
  }

  while(radioButtons_->buttons().size())
    delete radioButtons_->buttons().takeLast();

  if (ui_)
  {
    delete ui_;
    ui_ = NULL;
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

    // mark as active
    vmMask->Instance->setVmMaskIsActive(true);

    commandExec_->setCommands(commandList);
    commandExec_->start();

    // What happens next?
    // the commandExec_ sends a signal signalFinished(ePmCommandResult)) to the slot slotFinished() indicating
    // a success or failure in ePmCommandResult

    // disable all radio buttons in the meantime
    foreach (QAbstractButton* btn, radioButtons_->buttons())
    {
      btn->setEnabled(false);
    }
  }
}

void WidgetNewTab::slotFinished(ePmCommandResult parExitCode)
{
  // enable all radio buttons as default
  foreach (QAbstractButton* btn, radioButtons_->buttons())
  {
    btn->setEnabled(true);
  }

  switch(parExitCode)
  {
    case success:
      radioButtons_->button(currentSelectedVmMaskId_)->setChecked( false );
      radioButtons_->button(currentSelectedVmMaskId_)->setEnabled( false );

      ui_->labelDescription->clear();
      ui_->labelDescription->adjustSize();
      emit signalNewVmMaskReady(currentSelectedVmMaskId_);

      commandExec_->reset();
      ui_->btnStartVmMask->setEnabled(false);
      break;

    case failed:
      QMessageBox::warning(this, "Starting VM-Mask", "Starting the VirtualMachine failed. Please check the logfile for Details.");

      // mark as inactive
      if (currentSelectedVmMaskId_ >= 0)
        pmManager_->getVmMaskData()[currentSelectedVmMaskId_]->Instance->setVmMaskIsActive(false);

      break;

    case aborted:
      // In this case, the user would expect the "Start" button to be enabled even when not selecting another radio
      // button.
      ui_->btnStartVmMask->setEnabled(false);

      // mark as inactive
      if (currentSelectedVmMaskId_ >= 0)
        pmManager_->getVmMaskData()[currentSelectedVmMaskId_]->Instance->setVmMaskIsActive(false);

      break;

    default:
      break;
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
    ui_->labelDescription->adjustSize(); // From http://www.qtcentre.org/threads/2593-Auto-resize-QLabel

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

