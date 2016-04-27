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

#include "PMManager.h"
#include "WidgetUpdate.h"
#include "ui_WidgetUpdate.h"

#include <QMessageBox>

WidgetUpdate::WidgetUpdate(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::WidgetUpdate)
{
  ui_->setupUi(this);
  widgetCommandExec_ = new WidgetCommandExec(this);
  ui_->verticalLayout->addWidget(widgetCommandExec_);
}

WidgetUpdate::~WidgetUpdate()
{
  ui_->verticalLayout->removeWidget(widgetCommandExec_);
  delete widgetCommandExec_;
  delete ui_;
}

bool WidgetUpdate::init(PMManager* manager)
{
  manager_ = manager;

  connect(widgetCommandExec_,
          SIGNAL(signalFinished(commandResult)),
          this,
          SLOT(slotCommandsFinished(commandResult)));

  // frmMainWindows should receive Update-Status Messages
  connect(widgetCommandExec_,
          SIGNAL(signalUpdateProgress(QString)),
          this->parentWidget(),
          SLOT(slotUpdateProgress(QString)));

  widgetCommandExec_->connectSignalsAndSlots();

  // create all commands
  QList<PMCommand*> allCommands;
  manager->createCommandsUpdateAllVmMasks(allCommands);

  if (!widgetCommandExec_->setCommands(allCommands)) // widgetCommandExec_ also deletes the memory of the commands
    return false;

  return true;
}

void WidgetUpdate::start()
{
  widgetCommandExec_->start();
}

/*---------------------------------------------*/
/*            PRIVATE SLOTS:                   */
void WidgetUpdate::slotCommandsFinished(commandResult result)
{
  // emit similar signal to frmMainWindow
  emit signalUpdateFinished(result);
}

void WidgetUpdate::closeEvent(QCloseEvent * event)
{
  widgetCommandExec_->abort();

  emit signalUpdateFinished(aborted);
}

void WidgetUpdate::abort()
{
  widgetCommandExec_->abort();
}
