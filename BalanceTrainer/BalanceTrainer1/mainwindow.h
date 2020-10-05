#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "motioncontrol.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //运动控制面板相关参数
    void TabMotionControlInit();  //运动控制面板初始化

public slots:

    //运动控制面板
    void startSpeedModeSlot();
    void startPosModeSlot();

signals:
    //运动控制面板
    void startSpeedModeSignal(int x,int y,int z,int roll,int yaw,int pitch);
    void startPosModeSignal(int x,int y,int z,int roll,int yaw,int pitch,int rumTime);


private:
    Ui::MainWindow *ui;

    //运动控制面板
    QThread* m_motionControlThread;  //专门用于平台控制的线程
    MotionControl* m_motioncontrol;

};
#endif // MAINWINDOW_H
