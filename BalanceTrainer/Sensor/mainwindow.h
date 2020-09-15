#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "serialport.h"
#include "paintchart.h"
#include <QMainWindow>
#include <QChart>
#include <QTableWidget>

QT_CHARTS_USE_NAMESPACE
QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

public slots:

    void portOpenedSlot();
    void angleInitedSlot();

    void loadImgSlot();
    void clearImgSlot();

    void showGaitPhaseTimeSlot(); //将步态时相检测结果显示到界面上

public:
    void connectSignalAndSlot();//界面上所有按钮信号的槽的连接

    void fillTableAndPie(SerialPort* usart,bool leftFoot); //表格填充



private:
    Ui::MainWindow *ui;
    ChartWidget* chartwidget;

    QThread thread1;
    SerialPort* usart1;
    QThread thread2;
    SerialPort* usart2;
    QThread thread3;
    SerialPort* usart3;
    QString time=QDateTime::currentDateTime().toString("MMddhhmm");

};
#endif // MAINWINDOW_H
