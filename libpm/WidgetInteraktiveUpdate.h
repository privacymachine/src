#ifndef WIDGETINTERAKTIVEUPDATE_H
#define WIDGETINTERAKTIVEUPDATE_H


#include "CheckUpdate.h" //for Update
#include <QWidget>
#include <QButtonGroup>


namespace Ui {
  class WidgetInteraktiveUpdate;
}

/// \brief The WidgetInteraktiveUpdate class
/// \brief widget to interact with the user during a update process
class WidgetInteraktiveUpdate : public QWidget
{
    Q_OBJECT

  public:

    explicit WidgetInteraktiveUpdate(QWidget *parent = 0);
    ~WidgetInteraktiveUpdate();

    /// setter

    /// \brief setButtonsVisible
    /// \param visible
    void setButtonsVisible(bool visible);

    /// \brief setTextEditVisible
    /// \param visible
    void setTextEditVisible(bool visible);

    /// \brief setProgressBarVisible
    /// \param visible
    void setProgressBarVisible(bool visible);

    /// \brief setSkipButtonVisible
    /// \param visible
    void setSkipButtonVisible(bool visible);

    /// \brief setTextEditTitleVisible
    /// \param visible
    void setTextEditTitleVisible(bool visible);

    /// \brief setUpdateEffectsVisible
    /// \param visible
    void setUpdateEffectsVisible(bool visible);

    /// \brief setProgressBarAbortButtonVisible
    /// \param visible
    void setProgressBarAbortButtonVisible(bool visible);


    /// \brief setTitle
    /// \brief sets the Headline
    /// \param title
    void setTitle(const QString title);

    /// \brief setProgressBarRange
    /// \brief setProgressBarRange(0,0) indicates busy
    /// \param min
    /// \param max
    void setProgressBarRange(int min, int max);

    /// \brief setProgressBarText
    /// \brief sets the text of the label below the process bar
    /// \param text
    void setProgressBarText(QString text);

    /// \brief setUpdateEffectsText
    /// \brief sets the text of the secound Headline lable
    /// \param text
    void setUpdateEffectsText(QString text);

    /// \brief showUpdate
    /// \brief shows the changelog starting at the current version
    /// \brief shows a update dialog for the latest (relative to version) update in parUpdateList
    /// \param parUpdateList [in]: List of installable Updates
    void showUpdate(QList<Update> parUpdateList);


  signals:

    /// \brief signalUpdateSkipped
    /// \brief emited when the user presses the skip button
    void signalUpdateSkipped();

    ///
    /// \brief signalUpdateRequested
    /// \brief emited when the user presses the install button
    /// \param update [out]: the update the user wants to install
    void signalUpdateRequested(Update update);

    ///
    /// \brief signalAbortButtonPressed
    /// \brief emited when the user presses the abort button to cancle a download
    void signalAbortButtonPressed();


  public slots:

    /// \brief slotProgressBarUpdate
    /// \brief updates the progress bar
    /// \param current [in]: the value the progress bar will take
    /// \param max [in]: the maximum of the progressbar value range
    void slotProgressBarUpdate(qint64 current, qint64 max);


  private:

    Ui::WidgetInteraktiveUpdate *ui;
    Update update_;


  private slots:

    /// \brief slotEmitUpdateRequested
    /// \brief emits signalUpdateRequested whith the current processed update
    void slotEmitUpdateRequested() {emit signalUpdateRequested(update_);}

    /// \brief slotEmitUpdateSkipped
    /// \brief emits signalUpdateSkipped
    void slotEmitUpdateSkipped() {emit signalUpdateSkipped();}

    /// \brief slotEmitAbortButtonPressed
    /// \brief emitssignalAbortButtonPressed
    void slotEmitAbortButtonPressed() {emit signalAbortButtonPressed();}
};

#endif // WIDGETINTERAKTIVEUPDATE_H
