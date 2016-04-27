/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 5134 F10C 58ED FCA2 7F25 7483 50F4 40FC 4347 D242

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
    connect( rbItem, SIGNAL( clicked() ), this, SLOT( radioBtn_clicked() ) );
  }

  connect( ui_->btnStartVmMask, SIGNAL( clicked() ), this, SLOT( btnStart_clicked() ) );
  connect( commandExec_, SIGNAL( signalFinished(commandResult) ), this, SLOT( slotFinished(commandResult) ) );

}

void WidgetNewTab::disconnectSignalsAndSlots()
{
  foreach( QAbstractButton *rbItem, radioButtons_->buttons() )
  {
    disconnect( rbItem, SIGNAL( clicked() ), 0, 0 );
  }

  disconnect( ui_->btnStartVmMask, SIGNAL( clicked() ), 0, 0 );
  disconnect( commandExec_, SIGNAL( signalFinished(commandResult) ), 0, 0 );

}

bool WidgetNewTab::init(PMManager *parPMManager)
{
  pmManager_ = parPMManager;
  radioButtons_ = new QButtonGroup();
  startInProgress_ = false;
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

void WidgetNewTab::btnStart_clicked()
{
  ui_->btnStartVmMask->setEnabled(false);
  int indexVmMask = radioButtons_->checkedId();
  if (indexVmMask >= 0)
  {
    QList<PMCommand*> commandList;

    if(!pmManager_->createCommandsStart(pmManager_->getConfiguredVmMasks().at(indexVmMask)->name,
                                                  commandList))
    {
      IWARN("error creating commands for VM-Mask " + pmManager_->getConfiguredVmMasks().at(indexVmMask)->name);
      return;
    }

    commandExec_->setCommands(commandList);

    //// HACK
    //emit newVmMaskReady(indexVmMask);
    //// HACK: For testing FreeRDP, executing the virtualbox-commands is disabled +Ready-Event is fired

    startInProgress_ = true;
    commandExec_->start();
  }
}

void WidgetNewTab::radioBtn_clicked()
{
  if( radioButtons_->checkedId() >= 0 
      && radioButtons_->checkedId() < radioButtons_->buttons().count() )
  {
    ui_->labelDescription->setText( pmManager_->getConfiguredVmMasks()[radioButtons_->checkedId()]->description );
    // From http://www.qtcentre.org/threads/2593-Auto-resize-QLabel
    ui_->labelDescription->adjustSize();
    if( startInProgress_  ||  pmManager_->getInstances()[radioButtons_->checkedId()]->getConfig()->vmMaskCreated )
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

void WidgetNewTab::slotFinished(commandResult exitCode)
{
  startInProgress_ = false;
  int vmMaskId;
  switch(exitCode)
  {
    case success:
      vmMaskId = radioButtons_->checkedId();
      radioButtons_->checkedButton()->setChecked( false );
      radioButtons_->checkedButton()->setEnabled( false );
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
