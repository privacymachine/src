#include "WidgetAbout.h"
#include "ui_WidgetAbout.h"

WidgetAbout::WidgetAbout(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::WidgetAbout)
{
  ui_->setupUi(this);
}

WidgetAbout::~WidgetAbout()
{
  delete ui_;
}

void WidgetAbout::addWidget(QWidget *w)
{
  ui_->horizontalLayout_2->addWidget(w);
}
