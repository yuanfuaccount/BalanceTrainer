#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFile>
#include <QFileDialog>
#include <QButtonGroup>
#include <QThread>

#include "trajectoryplanning.h"
#include "motioncontrol.h"
#include "washout.h"


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
    //轨迹规划面板
    void TabTrajectoryPlanningInit();
    //体感仿真面板
    void TabWashoutInit();

public slots:

    //运动控制面板
    void startSpeedModeSlot();
    void startPosModeSlot();

    //轨迹规划部分
    void loadTrajectorySlot();

    //体感仿真/洗出算法部分
    void startWashoutSlot();

signals:
    //运动控制面板
    void startSpeedModeSignal(int x,int y,int z,int roll,int yaw,int pitch);
    void startPosModeSignal(int x,int y,int z,int roll,int yaw,int pitch,int rumTime);

    //轨迹规划部分

    //体感仿真/洗出算法部分
    void startWashoutSignal(double value,double time,double slopeTime,int mode);


private:
    Ui::MainWindow *ui;

    //运动控制面板
    QThread* m_motionControlThread;  //专门用于平台控制的线程
    MotionControl* m_motioncontrol;

    //轨迹规划部分
    TrajectoryPlanning* m_trajectoryPlanning;

    //体感仿真部分
    QButtonGroup* m_modeGrp;

};
#endif // MAINWINDOW_H
