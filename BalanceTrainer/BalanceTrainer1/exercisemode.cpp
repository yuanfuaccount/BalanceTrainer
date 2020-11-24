#include "exercisemode.h"

ExerciseMode::ExerciseMode()
{
    //平衡板模拟相关参数初始化
    m_maxAngLimit=5;
    m_angRatio=0.7;
    m_angX=0;m_angY=0;m_lastAngX=0;m_lastAngY=0;
    m_maxErrAng=2;

    //冲浪游戏相关参数初始化
    m_maxAccLimit=0.6; //最高加速度
    m_accRatio=0.05;
    m_accX=0;m_lastAccX=0;m_accY=0;m_lastAccY=0;
    m_maxErrAcc=0.07;  //加速度变化率不能超过0.7

    //颠簸模拟相关参数初始化
    m_peakX=4;m_peakY=4;
    m_wx=PI/5;m_wy=PI/6;
    m_runTime=0;
}


QVector<double> ExerciseMode::balanceBoard()
{
    QVector<double> res(2,0);
    m_angX=m_angRatio*waistAngX;
    m_angY=m_angRatio*waistAngY;

    //限制变化率
    if(m_angX-m_lastAngX>m_maxErrAng)
        m_angX=m_lastAngX+m_maxErrAng;
    if(m_angX-m_lastAngX<-1*m_maxErrAng)
        m_angX=m_lastAngX-m_maxErrAng;
    if(m_angY-m_lastAngY>m_maxErrAng)
        m_angY=m_lastAngY+m_maxErrAng;
    if(m_angY-m_lastAngY<-1*m_maxErrAng)
        m_angY=m_lastAngY-m_maxErrAng;

    //限幅
    if(m_angX>m_maxAngLimit)
        m_angX=m_maxAngLimit;
    if(m_angX<-1*m_maxAngLimit)
        m_angX=-1*m_maxAngLimit;
    if(m_angY>m_maxAngLimit)
        m_angY=m_maxAngLimit;
    if(m_angY<-1*m_maxAngLimit)
        m_angY=-1*m_maxAngLimit;

    res[0]=m_angX-m_lastAngX; //传感器X方向角度变化对应前后倾斜，即平台绕Y轴旋转
    res[1]=-(m_angY-m_lastAngY);   //传感器Y方向角度变化对应左右倾斜，即平台绕X轴旋转

    m_lastAngX=m_angX;
    m_lastAngY=m_angY;
    return res;
}

/**********************
 * 激流冲浪
 * 输出:向量，1*2，分别是AccX, AccY
 * ********************/
QVector<double> ExerciseMode::surfing()
{
    QVector<double> res(2,0);

    m_accX=m_accRatio*waistAngX;
    m_accY=m_accRatio*waistAngY;


    //限制变化率
    if(m_accX-m_lastAccX>m_maxErrAcc)
        m_accX=m_lastAccX+m_maxErrAcc;
    if(m_accX-m_lastAccX<-1*m_maxErrAcc)
        m_accX=m_lastAccX-m_maxErrAcc;
    if(m_accY-m_lastAccY>m_maxErrAcc)
        m_accY=m_lastAccY+m_maxErrAcc;
    if(m_accY-m_lastAccY<-1*m_maxErrAcc)
        m_accY=m_lastAccY-m_maxErrAcc;

    //限幅
    if(m_accX>m_maxAccLimit)
        m_accX=m_maxAccLimit;
    if(m_accX<-1*m_maxAccLimit)
        m_accX=-m_maxAccLimit;
    if(m_accY>m_maxAccLimit)
        m_accY=m_maxAccLimit;
    if(m_accY<-1*m_maxAccLimit)
        m_accY=-m_maxAccLimit;


    res[0]=m_accX;
    res[1]=m_accY;

    m_lastAccX=m_accX;
    m_lastAccY=m_accY;
    return res;
}

/* ***********************************
 * 颠簸效果模拟，输出6轴的位置增量
 * **********************************/
QVector<double> ExerciseMode::earthquake()
{
    m_runTime+=cmdInterval;
    QVector<double> res(6,0);
    res[2]=rand()%10-5; //Z方向位移增量随机生成范围【-5，4】
    while(g_pz+res[2]<PlatformPara::T0(2)+30)
        res[2]=rand()%10-5;
    while(g_pz+res[2]>PlatformPara::T0(2)+250)
        res[2]=rand()%10-5;
    res[3]=m_peakX*sin(m_wx*m_runTime)-m_peakX*sin(m_wx*(m_runTime-cmdInterval));
    res[4]=m_peakY*sin(m_wy*m_runTime)-m_peakY*sin(m_wy*(m_runTime-cmdInterval));
    return res;
}

void ExerciseMode::ResetRuntime()
{
    m_runTime=0;
}
