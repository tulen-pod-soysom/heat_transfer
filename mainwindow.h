#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "heat_transfer_program.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    heat_transfer_program program;

private slots:
    void on_pushButton_clicked();
    void timer1_start();


    void on_pushButton_3_clicked();

    void on_do_izolines_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QTimer timer1;
};
#endif // MAINWINDOW_H
