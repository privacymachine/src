#include "WidgetInteractiveUpdate.h"
#include "ui_WidgetInteractiveUpdate.h"
#include <algorithm>    // std::sort

WidgetInteractiveUpdate::WidgetInteractiveUpdate(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::WidgetInteractiveUpdate)
{
  ui->setupUi(this);
  ui->widgetTextEdit->setVisible(false);
  ui->buttonDownloadOnly->setVisible(false); // no nonblokin updates implemented now

  connect(ui->buttonIgnore,SIGNAL(clicked()),
          this,SLOT(slotEmitUpdateSkipped()));
  connect(ui->buttonAbort,SIGNAL(clicked()),
          this,SLOT(slotEmitAbortButtonPressed()));
}

WidgetInteractiveUpdate::~WidgetInteractiveUpdate()
{
  delete ui;
}

void WidgetInteractiveUpdate::setTitle(const QString title)
{
  ui->labelTitle->setText(title);
}
void WidgetInteractiveUpdate::setButtonsVisible(bool visible)
{
  ui->widgetButtons->setVisible(visible);
}
void WidgetInteractiveUpdate::setTextEditVisible(bool visible)
{
  ui->widgetTextEdit->setVisible(visible);
}
void WidgetInteractiveUpdate::setProgressBarVisible(bool visible)
{
  ui->widgetProcessBar->setVisible(visible);
}
void WidgetInteractiveUpdate::setSkipButtonVisible(bool visible)
{
  ui->buttonIgnore->setVisible(visible);
}
void WidgetInteractiveUpdate::setTextEditTitleVisible(bool visible)
{
  ui->widgetUpdateTitle->setVisible(visible);
}
void WidgetInteractiveUpdate::setUpdateEffectsVisible(bool visible)
{
  ui->widgetUpdateEffects->setVisible(visible);
}
void WidgetInteractiveUpdate::setProgressBarAbortButtonVisible(bool visible)
{
  ui->buttonAbort->setVisible(visible);
}

void WidgetInteractiveUpdate::setProgressBarRange(int min, int max)
{
  ui->progressBar->setRange(min,max);
}
void WidgetInteractiveUpdate::setProgressBarText(QString text)
{
  ui->labelProgressBar->setText(text);
}
void WidgetInteractiveUpdate::setUpdateEffectsText(QString text)
{
  ui->labelUpdateEffects->setText(text);
}
void WidgetInteractiveUpdate::slotProgressBarUpdate(qint64 current, qint64 max)
{
  ui->progressBar->setValue(current);
  ui->progressBar->setMaximum(max);
}


void WidgetInteractiveUpdate::showUpdate(QList<Update> parUpdateList)
{
  std::sort( parUpdateList.begin(), parUpdateList.end(), Update::compare );
  update_ = parUpdateList[0];
  QString changelog = "";
  foreach (Update update, parUpdateList)
  {
    changelog += update.Description;
    changelog += "\n\n";
  }
  ui->textEditDescription->setText(changelog);
  ui->labelUpdateTitle->setText(update_.Title);
  connect( ui->buttonInstallNow, SIGNAL(clicked()), this, SLOT(slotEmitUpdateRequested()) );
}


