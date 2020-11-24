#ifndef EXERCISEMODE_H
#define EXERCISEMODE_H

#include <math.h>
#include "config.h"
/* ****************************
 * 平衡训练模式设计,主要有四大模式：
 * 1. 平衡板模拟：利用腰部传感器检测人体倾斜，控制平台以同样的角度倾斜
 * 2. 激流冲浪，前后倾斜代表前后加速度，左右倾斜代表平台左右角度
 * *****************************/

class ExerciseMode
{
public:
    ExerciseMode();
    QVector<double> balanceBoard();
    QVector<double> surfing();
    QVector<double> earthquake(); //颠簸效果
    void ResetRuntime();

private:
    //平衡板模拟相关参数
    double m_maxAngLimit; //平台倾斜的最大角度
    double m_angRatio; //实际倾斜角度与平台倾斜角度的比例，平台倾斜角度=m_angRatio*实际倾斜角度
    double m_angX,m_angY,m_lastAngX,m_lastAngY;
    double m_maxErrAng; //为了控制速度，本次角度跟上次角度之差的绝对值不能超过此值


    //激流冲浪相关参数
    double m_maxAccLimit; //最大加速度限制

    double m_accRatio; //由倾斜角度转变为洗出算法加速度的比例，洗出加速度=m_accRatio*实际倾斜角度
    double m_accX,m_lastAccX,m_accY,m_lastAccY;
    double m_maxErrAcc; //控制速度

    //颠簸模拟效果参数
    int m_peakX,m_peakY; //正弦角度的幅值
    double m_wx,m_wy; //正弦运动的角频率
    double m_runTime; //记录运行时间


};

#endif // EXERCISEMODE_H
