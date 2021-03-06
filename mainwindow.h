#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendbt_clicked();

    void on_connectbt_clicked();

    void on_clearbt_clicked();

    void on_actionAbout_QT_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
