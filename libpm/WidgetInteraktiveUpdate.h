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
    void setTextEditVisible(bool visible);
    void setProgressBarVisible(bool visible);
    void setSkipButtonVisible(bool visible);
    void setTextEditTitleVisible(bool visible);
    void setUpdateEffectsVisible(bool visible);
    void setProgressBarAbortButtonVisible(bool visible);

    /// sets the Headline
    void setTitle(const QString title);

    void setProgressBarRange(int min, int max);
    void setProgressBarText(QString text);
    void setUpdateEffectsText(QString text);

    void showUpdate(QList<Update> parUpdateList);

  signals:
    void signalUpdateSkipped();
    void signalUpdateRequested(Update);
    void signalAbortButtonPressed();

  public slots:
    void slotProgressBarUpdate(qint64 current, qint64 max);

  private:
    Ui::WidgetInteraktiveUpdate *ui;
    Update update_;

  private slots:
    void slotEmitUpdateRequested() {emit signalUpdateRequested(update_);}
    void slotEmitUpdateSkipped() {emit signalUpdateSkipped();}
    void slotEmitAbortButtonPressed() {emit signalAbortButtonPressed();}
};

#endif // WIDGETINTERAKTIVEUPDATE_H
