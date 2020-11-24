#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#include <QObject>
#include <QVector>


#include "config.h"
#include "udpdata.h"
#include "kinematics.h"
#include "washout.h"
#include "exercisemode.h"


class MotionControl:public UdpData
{
    Q_OBJECT
public:
    MotionControl(QObject* parent=nullptr);



public slots:
    void threadStartSlot();  //槽函数，线程开启时进行初始化，比如开启定时器
    void SpeedAndPosMotionSlot(); //在定时器中断时，执行此槽函数，进行运动模式的实时检查

    //速度运动模式
    void startSpeedModeSlot(int x,int y,int z,int roll,int yaw,int pitch);
    //位置运动模式
    void startPositionModeSlot(int x,int y,int z,int roll,int yaw,int pitch,int runTime);
    //速度模式和位置模式停止都是一个槽函数
    void stopSpeedAndPosModeSlot();

    //平台控制按钮
    void platformResetSlot();
    void platformHaltSlot();
    void platformCancelHaltSlot();
    void platformToMiddleSlot();

    //轨迹规划
    void startTrajectoryPlanningSlot(QVector<QVector<double>>* path);

    //体感仿真
    void startWashoutSlot(double value,double time,double slopeTime,int mode);

    //平台平衡训练部分
    void exerciseStartSlot(int mode);


private:
    int m_ModeFlag;  //控制运动模式，m_ModeFlag=0:平台静止  m_ModeFlag=1：平台速度运动模式  m_ModeFlag=2：位置运动模式
    std::atomic<uint> m_changeFlag;  //用来做类似mutex的作用，保证m_ModeFlag与其他相关参数改变的原子性

    double m_xInitPos,m_yInitPos,m_zInitPos,m_uInitPos,m_vInitPos,m_wInitPos; //各个电动缸的初始长度

    //速度模式使用
    double m_speedX,m_speedY,m_speedZ,m_speedRoll,m_speedYaw,m_speedPitch;
    //位置模式使用
    double m_actualRunTime,m_setRunTime;

    //轨迹规划用于保存轨迹的指针
    QVector<QVector<double>>* m_trajectoryPath;
    int m_trajectoryPtr=0; //指向轨迹规划数组m_trajectoryPath的指针

    //体感仿真模块
    WashOut* m_washout;  //因为Washout需要不断执行getwashout函数，需要用到timer，因此作为motioncontrol的成员变量
    double m_value,m_time,m_slopeTime,m_runtime;
    int m_mode;
    QVector<double> m_inputAccW,m_deltaPos;

    //训练模式
    ExerciseMode* m_exercise;


    QTimer* timer;
};

#endif // MOTIONCONTROL_H
