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
#include "imusensor.h"
#include "paintchart.h"

#define USELEFTFOOT 1
#define USERIGHTFOOT 1
#define USEWAIST 1


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
    //传感器数据采集面板
    void TabSensorInit();

    //分析图像加载的面板
    void TabImgInit();

    //步态时相数据显示面板
    void TabGaitPhaseExhibitionInit();

    //步态对称性显示面板
    void TabGaitSymmtreyInit();


public slots:

    //运动控制面板
    void startSpeedModeSlot();
    void startPosModeSlot();

    //轨迹规划部分
    void loadTrajectorySlot();

    //体感仿真/洗出算法部分
    void startWashoutSlot();

    //传感器数据采集部分
    void portOpenedSlot(); //当所有串口打开时，提示
    void angleInitedSlot(); //当所有串口角度初始化完成时提示

    //原始数据图像显示部分
    void loadImgSlot();
    void clearImgSlot();

    //步态时相数据显示部分
    void showLeftGaitPhaseTimeSlot();
    void showRightGaitPhaseTimeSlot();

    //步态对称性数据显示部分
    void showGaitSymmetrySlot();


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

    //数据采集部分，主要是三个传感器，每个传感器单独开一个线程
    QThread m_sensorThread1;
    FootSensor* leftFootSensor;
    QThread m_sensorThread2;
    FootSensor* rightFootSensor;
    QThread m_sensorThread3;
    WaistSensor* waistSensor;

    //原始图像显示部分
    ChartWidget* m_chartWidget;

    //步态时相显示部分
    GaitPhaseExhibition* m_gaitPhaseExhibition;

    //步态对称性显示部分
    AutoCorrChart* m_ychart;
    AutoCorrChart* m_zchart;

    QString time=QDateTime::currentDateTime().toString("MMddhhmm");
};
#endif // MAINWINDOW_H
