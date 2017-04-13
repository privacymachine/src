#include "WidgetInteraktiveUpdate.h"
#include "ui_WidgetInteraktiveUpdate.h"
#include <QRadioButton>

WidgetInteraktiveUpdate::WidgetInteraktiveUpdate(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::WidgetInteraktiveUpdate)
{
  ui->setupUi(this);
  ui->widgetTextEdit->setVisible(false);
  ptrRadioButtonGroup_ = NULL;
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
void WidgetInteraktiveUpdate::setUpdateSelectorVisible(bool visible)
{
  ui->widgetUpdateSelector->setVisible(visible);
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

void WidgetInteraktiveUpdate::setProgressBarRange(int min, int max)
{
  ui->progressBar->setRange(min,max);
}
void WidgetInteraktiveUpdate::setProgressBarText(QString text)
{
  ui->labelProgressBar->setText(text);
}
void WidgetInteraktiveUpdate::setupRadioButtons(QList<Update> updates)
{
  updateList_=updates;
  if (ptrRadioButtonGroup_ != NULL) ptrRadioButtonGroup_->deleteLater();
  ptrRadioButtonGroup_ = new QButtonGroup(this);

  ui->buttonShowAllUpdates->setVisible(true);

  int latestUpdateIndex = 0;
  for (int i=0; i<updateList_.size(); i++)
  {
    if (updateList_[i].Version > updateList_[latestUpdateIndex].Version) latestUpdateIndex=i;

    QRadioButton *rbItem = new QRadioButton(updateList_[i].Version.toString()+": "+updateList_[i].Title, this);
    rbItem->setVisible(false);
    rbItem->setChecked(false);
    ptrRadioButtonGroup_->addButton(rbItem,i);
    ui->layoutRadioButtons->addWidget(rbItem);
    connect( rbItem, SIGNAL(clicked()), this, SLOT(slotRadioButtonClicked()) );
  }
  ptrRadioButtonGroup_->button(latestUpdateIndex)->setVisible(true);
  ptrRadioButtonGroup_->button(latestUpdateIndex)->setChecked(true);
  ptrRadioButtonGroup_->setExclusive(true);
  ui->textEditDescription->setText(updateList_[latestUpdateIndex].Description);

  connect( ui->buttonShowAllUpdates, SIGNAL(clicked()), this, SLOT(slotShowAllUpdates()) );
  connect( ui->buttonInstallNow, SIGNAL(clicked()), this, SLOT(slotEmitUpdateRequested()) );
}
void WidgetInteraktiveUpdate::slotRadioButtonClicked()
{
  int selectedUpdateIndex = ptrRadioButtonGroup_->checkedId();
  if (selectedUpdateIndex >= 0)
  {
    ui->textEditDescription->setText(updateList_[selectedUpdateIndex].Description);
  }
}

void WidgetInteraktiveUpdate::slotEmitUpdateRequested()
{
  int selectedUpdateIndex = ptrRadioButtonGroup_->checkedId();
  if (selectedUpdateIndex >= 0)
  {
      disconnect( ui->buttonInstallNow, SIGNAL(clicked()), NULL, NULL);
      emit signalUpdateRequested(updateList_[selectedUpdateIndex]);
  }
}

void WidgetInteraktiveUpdate::slotShowAllUpdates()
{
  disconnect( ui->buttonShowAllUpdates, SIGNAL(clicked()), NULL, NULL);
  for (int i=0; i<updateList_.size(); i++)
  {
    ptrRadioButtonGroup_->button(i)->setVisible(true);
  }
  ui->buttonShowAllUpdates->setVisible(false);
}
