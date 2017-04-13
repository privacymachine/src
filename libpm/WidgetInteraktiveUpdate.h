#ifndef WIDGETINTERAKTIVEUPDATE_H
#define WIDGETINTERAKTIVEUPDATE_H


#include "CheckUpdate.h" //for Update
#include <QWidget>
#include <QButtonGroup>


namespace Ui {
  class WidgetInteraktiveUpdate;
}

class WidgetInteraktiveUpdate : public QWidget
{
    Q_OBJECT

  public:

    explicit WidgetInteraktiveUpdate(QWidget *parent = 0);
    ~WidgetInteraktiveUpdate();

    ///setter
    void setButtonsVisible(bool visible);
    void setUpdateSelectorVisible(bool visible);
    void setTextEditVisible(bool visible);
    void setProgressBarVisible(bool visible);
    void setSkipButtonVisible(bool visible);

    /// sets the Headline
    void setTitle(const QString title);

    void setProgressBarRange(int min, int max);
    void setProgressBarText(QString text);

    void setupRadioButtons(QList<Update> updates);

  signals:
    void signalUpdateSkipped();
    void signalUpdateRequested(Update);

  private:
    Ui::WidgetInteraktiveUpdate *ui;
    QList<Update> updateList_;
    QButtonGroup *ptrRadioButtonGroup_;

  private slots:
    void slotShowAllUpdates();
    void slotEmitUpdateRequested();
    void slotRadioButtonClicked();
    void slotEmitUpdateSkipped() {emit signalUpdateSkipped();}
};

#endif // WIDGETINTERAKTIVEUPDATE_H
