#include "WidgetInteractiveUpdate.h"
#include "ui_WidgetInteractiveUpdate.h"
#include <algorithm>    // std::sort

WidgetInteractiveUpdate::WidgetInteractiveUpdate(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::WidgetInteractiveUpdate)
{
  ui_->setupUi(this);
  ui_->widgetTextEdit->setVisible(false);
  ui_->buttonDownloadOnly->setVisible(false); // no nonblokin updates implemented now

  connect(ui_->buttonIgnore,
          &QAbstractButton::clicked,
          this,
          &WidgetInteractiveUpdate::slotEmitUpdateSkipped);

  connect(ui_->buttonAbort,
          &QAbstractButton::clicked,
          this,
          &WidgetInteractiveUpdate::slotEmitAbortButtonPressed);
}

WidgetInteractiveUpdate::~WidgetInteractiveUpdate()
{
  if (ui_)
  {
    delete ui_;
    ui_ = NULL;
  }
}

void WidgetInteractiveUpdate::setTitle(const QString title)
{
  ui_->labelTitle->setText(title);
}
void WidgetInteractiveUpdate::setButtonsVisible(bool visible)
{
  ui_->widgetButtons->setVisible(visible);
}
void WidgetInteractiveUpdate::setTextEditVisible(bool visible)
{
  ui_->widgetTextEdit->setVisible(visible);
}
void WidgetInteractiveUpdate::setProgressBarVisible(bool visible)
{
  ui_->widgetProcessBar->setVisible(visible);
}
void WidgetInteractiveUpdate::setSkipButtonVisible(bool visible)
{
  ui_->buttonIgnore->setVisible(visible);
}
void WidgetInteractiveUpdate::setTextEditTitleVisible(bool visible)
{
  ui_->widgetUpdateTitle->setVisible(visible);
}
void WidgetInteractiveUpdate::setUpdateEffectsVisible(bool visible)
{
  ui_->widgetUpdateEffects->setVisible(visible);
}
void WidgetInteractiveUpdate::setProgressBarAbortButtonVisible(bool visible)
{
  ui_->buttonAbort->setVisible(visible);
}

void WidgetInteractiveUpdate::setProgressBarRange(int min, int max)
{
  ui_->progressBar->setRange(min,max);
}
void WidgetInteractiveUpdate::setProgressBarText(QString text)
{
  ui_->labelProgressBar->setText(text);
}
void WidgetInteractiveUpdate::setUpdateEffectsText(QString text)
{
  ui_->labelUpdateEffects->setText(text);
}
void WidgetInteractiveUpdate::slotProgressBarUpdate(qint64 current, qint64 max)
{
  ui_->progressBar->setValue(current);
  ui_->progressBar->setMaximum(max);
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
  ui_->textEditDescription->setText(changelog);
  ui_->labelUpdateTitle->setText(update_.Title);
  connect( ui_->buttonInstallNow,
           &QAbstractButton::clicked,
           this,
           &WidgetInteractiveUpdate::slotEmitUpdateRequested);
}


