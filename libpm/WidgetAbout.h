#ifndef WIDGETABOUT_H
#define WIDGETABOUT_H

#include <QWidget>

namespace Ui {
  class WidgetAbout;
}

class WidgetAbout : public QWidget
{
    Q_OBJECT

  public:
    explicit WidgetAbout(QWidget *parent = 0);
    ~WidgetAbout();
    void addWidget(QWidget *w);

  private:
    Ui::WidgetAbout *ui_;
};

#endif // WIDGETABOUT_H
