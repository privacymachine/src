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

#include <QMessageBox>

#include "WidgetCommandExec.h"
#include "UserConfig.h"
#include "PMInstance.h"
#include "PMManager.h"
#include "WidgetNewTab.h"
#include "ui_WidgetNewTab.h"
#include "utils.h"


WidgetNewTab::WidgetNewTab(QWidget *parParent) :
  QWidget(parParent),
  ui_(new Ui::WidgetNewTab)
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
  connect( commandExec_, SIGNAL( signalFinished(CommandResult) ), this, SLOT( slotFinished(CommandResult) ) );

}

void WidgetNewTab::disconnectSignalsAndSlots()
{
  foreach( QAbstractButton *rbItem, radioButtons_->buttons() )
  {
    disconnect( rbItem, SIGNAL( clicked() ), 0, 0 );
  }

  disconnect( ui_->btnStartVmMask, SIGNAL( clicked() ), 0, 0 );
  disconnect( commandExec_, SIGNAL( signalFinished(CommandResult) ), 0, 0 );

}

bool WidgetNewTab::init(PMManager *parPMManager)
{
  pmManager_ = parPMManager;
  radioButtons_ = new QButtonGroup();
  ui_->verticalLayout_2->setAlignment( Qt::AlignTop );
  
  foreach (PMInstance* curPmInstance, pmManager_->getInstances())
  {
    QRadioButton *rbItem = new QRadioButton( curPmInstance->getConfig()->fullName, this);
    radioButtons_->addButton(rbItem, curPmInstance->getConfig()->vmMaskId);
    ui_->chooseLayout->addWidget(rbItem);

  }
  radioButtons_->setExclusive(true);
  ui_->btnStartVmMask->setEnabled( false );

  connectSignalsAndSlots();
  commandExec_->connectSignalsAndSlots();

  // If there is only one configured VM-Mask select it
  if (radioButtons_->buttons().count() == 1)
  {
    radioButtons_->buttons().first()->click();
  }

  return true;
}

WidgetNewTab::~WidgetNewTab()
{
  disconnectSignalsAndSlots();

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
  ui_->btnStartVmMask->setEnabled(false);
}

void WidgetNewTab::slotBtnStart_clicked()
{
  ui_->btnStartVmMask->setEnabled(false);
  int indexVmMask = currentBtnId_ = radioButtons_->checkedId();
  if (indexVmMask >= 0)
  {
    QList<PMCommand*> commandList;

    if(!pmManager_->createCommandsStart(pmManager_->getConfiguredVmMasks().at(indexVmMask)->name,
                                                  commandList))
    {
      IWARN("error creating commands for VM-Mask " + pmManager_->getConfiguredVmMasks().at(indexVmMask)->name);
      slotFinished(failed);
      return;
    }

    commandExec_->setCommands(commandList);

    //// HACK
    //emit newVmMaskReady(indexVmMask);
    //// HACK: For testing FreeRDP, executing the virtualbox-commands is disabled +Ready-Event is fired

    commandExec_->start();
  }
}

void WidgetNewTab::slotRadioBtn_clicked()
{
  if( radioButtons_->checkedId() >= 0 
      && radioButtons_->checkedId() < radioButtons_->buttons().count() )
  {
    ui_->labelDescription->setText( pmManager_->getConfiguredVmMasks()[radioButtons_->checkedId()]->description );
    // From http://www.qtcentre.org/threads/2593-Auto-resize-QLabel
    ui_->labelDescription->adjustSize();
    if( commandExec_->getRunning()  ||  pmManager_->getInstances()[radioButtons_->checkedId()]->getConfig()->vmMaskCreated )
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

void WidgetNewTab::slotFinished(CommandResult exitCode)
{
  int vmMaskId;
  switch(exitCode)
  {
    case success:
      vmMaskId = currentBtnId_;
      radioButtons_->button(vmMaskId)->setChecked( false );
      radioButtons_->button(vmMaskId)->setEnabled( false );

      ui_->labelDescription->clear();
      ui_->labelDescription->adjustSize();
      if (vmMaskId >= 0)
        emit newVmMaskReady(vmMaskId);

      reset();
      break;

    case failed:
      QMessageBox::warning(this, "Starting VM-Mask", "Starting the VirtualMachine failed. Please check the logfile for Details.");
      break;

    case aborted:
      // In this case, the user would expect the "Start" button to be enabled even when not selecting another radio
      // button.
      ui_->btnStartVmMask->setEnabled(false);
      break;

    default:
      break;
  }
}


void WidgetNewTab::slotVmMaskClosed( int vmMaskId )
{
  QAbstractButton *vmButton = radioButtons_->button( vmMaskId );
  if( vmButton != NULL )
  {
    vmButton->setEnabled( true );
  }
}
