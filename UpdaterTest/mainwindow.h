#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../libpm/CheckUpdate.h"
#include "../libpm/UpdateManager.h"
namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  public slots:
    void slotDownloaderFinished();
    void slotUpdateFound(Update avaiableUpdate);

  private:
    Ui::MainWindow *ui;
    CheckUpdate *dl_;
    UpdateManager updateManager_;
};

#endif // MAINWINDOW_H
