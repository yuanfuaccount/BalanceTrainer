#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMainWindow>
#include <QChart>
#include <QTableWidget>
#include "imusensor.h"
#include "paintchart.h"

#define USE_LEFT_FOOT 0
#define USE_RIGHT_FOOT 0
#define USE_WAIST 1

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
    void showGaitSymmetrySlot(); //将步态对称性分析结果显示到界面上

public:
    void connectSignalAndSlot();//界面上所有按钮信号的槽的连接

    void fillTableAndPie(FootSensor* sensor,bool leftFoot); //表格填充



private:
    Ui::MainWindow *ui;
    ChartWidget*  chartwidget;

    QThread thread1;
    FootSensor* leftFootSensor;
    QThread thread2;
    FootSensor* rightFootSensor;
    QThread thread3;
    WaistSensor* waistSensor;

    QString time=QDateTime::currentDateTime().toString("MMddhhmm");

};
#endif // MAINWINDOW_H
