#include "WidgetInteraktiveUpdate.h"
#include "ui_WidgetInteraktiveUpdate.h"
#include <algorithm>    // std::sort

WidgetInteraktiveUpdate::WidgetInteraktiveUpdate(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::WidgetInteraktiveUpdate)
{
  ui->setupUi(this);
  ui->widgetTextEdit->setVisible(false);
  ui->buttonDownloadOnly->setVisible(false); // no nonblokin updates implemented now

  connect(ui->buttonIgnore,SIGNAL(clicked()),
          this,SLOT(slotEmitUpdateSkipped()));
}

WidgetInteraktiveUpdate::~WidgetInteraktiveUpdate()
{
  delete ui;
}

void WidgetInteraktiveUpdate::setTitle(const QString title)
{
  ui->labelTitle->setText(title);
}
void WidgetInteraktiveUpdate::setButtonsVisible(bool visible)
{
  ui->widgetButtons->setVisible(visible);
}
void WidgetInteraktiveUpdate::setTextEditVisible(bool visible)
{
  ui->widgetTextEdit->setVisible(visible);
}
void WidgetInteraktiveUpdate::setProgressBarVisible(bool visible)
{
  ui->widgetProcessBar->setVisible(visible);
}
void WidgetInteraktiveUpdate::setSkipButtonVisible(bool visible)
{
  ui->buttonIgnore->setVisible(visible);
}
void WidgetInteraktiveUpdate::setUpdateTitleVisible(bool visible)
{
  ui->widgetUpdateTitle->setVisible(visible);
}
void WidgetInteraktiveUpdate::setUpdateEffectsVisible(bool visible)
{
  ui->widgetUpdateEffects->setVisible(visible);
}

void WidgetInteraktiveUpdate::setProgressBarRange(int min, int max)
{
  ui->progressBar->setRange(min,max);
}
void WidgetInteraktiveUpdate::setProgressBarText(QString text)
{
  ui->labelProgressBar->setText(text);
}
void WidgetInteraktiveUpdate::setUpdateEffectsText(QString text)
{
  ui->labelUpdateEffects->setText(text);
}

void WidgetInteraktiveUpdate::showUpdate(QList<Update> parUpdateList)
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


