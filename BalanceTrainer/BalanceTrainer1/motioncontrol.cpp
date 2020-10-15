#include "motioncontrol.h"


MotionControl::MotionControl(QObject* parent)
{
    this->setParent(parent);

    m_ModeFlag=0;
    m_changeFlag=0;

    m_speedX=0;m_speedY=0;m_speedZ=0;m_speedRoll=0;m_speedYaw=0;m_speedPitch=0;

    m_actualRunTime=0;m_setRunTime=0;

    //理论标号的初始电动缸长度
    QVector<double> jointInitPos=PositionReverse(0,0,PlatformPara::T0(2),0,0,0);

    /* ***************************************
     * 实际标号为1(xpos)，2(ypos)，3(zpos)，4(upos)，5(vpos)，6(wpos)
     * 对应的理论标号为4，3，2，1，6，5
     * 实际每个MDBOX控制的电动缸标号与理论不对应，此处需要根据具体情况修改
    * ****************************************/
    m_xInitPos=jointInitPos[3];m_yInitPos=jointInitPos[2];m_zInitPos=jointInitPos[1];m_uInitPos=jointInitPos[0];m_vInitPos=jointInitPos[5];m_wInitPos=jointInitPos[4];
}


//所有槽函数定义
/* ********************************
 * 在子线程中开启定时器，定时器中断中执行此槽函数，实时检测当前运动模式，并下发运动指令
 * ********************************/
void MotionControl::SpeedAndPosMotionSlot()
{
    if(m_changeFlag==0) //m_ModeFlag没在改变中
    {
        if(m_ModeFlag==1 || m_ModeFlag==2) //速度控制模式
        {
            g_px=g_px+cmdInterval*m_speedX;
            g_py=g_py+cmdInterval*m_speedY;
            g_pz=g_pz+cmdInterval*m_speedZ;
            g_roll=g_roll+cmdInterval*m_speedRoll;
            g_yaw=g_yaw+cmdInterval*m_speedYaw;
            g_pitch=g_pitch+cmdInterval*m_speedPitch;  //6维空间位姿改变

            QVector<double> jointPos=PositionReverse(g_px,g_py,g_pz,g_yaw,g_pitch,g_roll); //求取每根电动缸的长度

            /***********************************************
             * 此处也涉及到理论标号与实际标号的对应问题，需要根据实际情况修改
             * PlayActionCmd的下发顺序是x,y,z,u,v,w
             * ***********************************************/
            IncreasePlayLine(); //时间序列增加
            if(!PlayActionCmd(jointPos[3]-m_xInitPos,jointPos[2]-m_yInitPos,jointPos[1]-m_zInitPos,jointPos[0]-m_uInitPos,jointPos[5]-m_vInitPos,jointPos[4]-m_wInitPos))
                m_ModeFlag=0;  //执行失败，说明有电动缸行程超过行程限制

            //判断是否到达终点位置，即运行时间到了runTime
            if(m_ModeFlag==2)
            {
                m_actualRunTime+=cmdInterval;
                if(m_actualRunTime>=m_setRunTime)
                    m_ModeFlag=0;
            }
        }
        else if(m_ModeFlag==3) //轨迹规划模式
        {
            if(m_trajectoryPtr>=m_trajectoryPath->size())
                m_ModeFlag=0;
            g_px=g_px+(*m_trajectoryPath)[m_trajectoryPtr][0];
            g_py=g_py+(*m_trajectoryPath)[m_trajectoryPtr][1];
            g_pz=g_pz+(*m_trajectoryPath)[m_trajectoryPtr][2];
            g_roll=g_roll+(*m_trajectoryPath)[m_trajectoryPtr][3];
            g_yaw=g_yaw+(*m_trajectoryPath)[m_trajectoryPtr][4];
            g_pitch=g_pitch+(*m_trajectoryPath)[m_trajectoryPtr][5];

            QVector<double> jointPos=PositionReverse(g_px,g_py,g_pz,g_yaw,g_pitch,g_roll); //求取每根电动缸的长度

            IncreasePlayLine(); //时间序列增加
            if(!PlayActionCmd(jointPos[3]-m_xInitPos,jointPos[2]-m_yInitPos,jointPos[1]-m_zInitPos,jointPos[0]-m_uInitPos,jointPos[5]-m_vInitPos,jointPos[4]-m_wInitPos))
                m_ModeFlag=0;  //执行失败，说明有电动缸行程超过行程限制

            m_trajectoryPtr++;
        }
        else if(m_ModeFlag==4) //体感仿真部分
        {
            //如果是m_mode==3，表示为颠簸模式，输出为位置增量
            m_inputAccW=m_washout->calAccW(m_value,m_time,m_slopeTime,m_runtime,m_mode);
            m_runtime+=cmdInterval;

            if(m_mode==3)  //颠簸模式
                m_deltaPos=m_inputAccW;
            if(m_mode<3)
                m_deltaPos=m_washout->getWashOut(m_inputAccW);

            /*
            if(fabs(m_deltaPos[0])<m_deltaPosLimit && fabs(m_deltaPos[1])<m_deltaPosLimit && fabs(m_deltaPos[2])<m_deltaPosLimit && fabs(m_deltaPos[3])<m_deltaAngLimit && fabs(m_deltaPos[4])<m_deltaAngLimit && fabs(m_deltaPos[5])<m_deltaAngLimit)
                m_ModeFlag=0; //到达终点，体感仿真结束
                */

            //求全局坐标
            g_px=g_px+m_deltaPos[0];
            g_py=g_py+m_deltaPos[1];
            g_pz=g_pz+m_deltaPos[2];
            g_roll=g_roll+m_deltaPos[3];
            g_pitch=g_pitch+m_deltaPos[4];
            g_yaw=g_yaw+m_deltaPos[5];



            QVector<double> jointPos=PositionReverse(g_px,g_py,g_pz,g_yaw,g_pitch,g_roll); //求取每根电动缸的长度

            IncreasePlayLine(); //时间序列增加

            if(!PlayActionCmd(jointPos[3]-m_xInitPos,jointPos[2]-m_yInitPos,jointPos[1]-m_zInitPos,jointPos[0]-m_uInitPos,jointPos[5]-m_vInitPos,jointPos[4]-m_wInitPos))
                m_ModeFlag=0;  //执行失败，说明有电动缸行程超过行程限制

            qDebug()<<g_px<<" "<<g_py<<" "<<g_pz<<" "<<g_roll<<" "<<g_pitch<<" "<<g_yaw;
            //qDebug()<<m_deltaPos[6];
        }

    }
}

/* **********************************
 * 线程开启时初始化槽函数，主要是开启定时器，连接定时器中断信号和相关的处理槽函数
 * *********************************/
void MotionControl::threadStartSlot()
{
    sock=new QUdpSocket(this);
    timer=new QTimer(this);

    m_washout=new WashOut(this);

    connect(timer,&QTimer::timeout,this,&MotionControl::SpeedAndPosMotionSlot);
    timer->start(100); //执行周期100ms
}

/* *************************************
 * 点击运动模式中速度模式开始按钮，触发此槽函数
 * ***********************************/
void MotionControl::startSpeedModeSlot(int x,int y,int z,int roll,int yaw,int pitch)
{
    //保证所有数据改变的原子性
    m_changeFlag=1;
    m_ModeFlag=1;
    m_speedX=x;
    m_speedY=y;
    m_speedZ=z;
    m_speedRoll=roll;
    m_speedYaw=yaw;
    m_speedPitch=pitch;
    m_changeFlag=0;
}

/* *************************************
 * 点击运动模式中速度模式停止按钮或者位置模式停止按钮，触发此槽函数
 * ***********************************/
void MotionControl::stopSpeedAndPosModeSlot()
{
    m_changeFlag=1;
    m_ModeFlag=0;  //停止运动
    m_changeFlag=0;
}

/* ***********************************
 * 点击平台零位按钮，执行此槽函数，寻找初始零位
 * ********************************/
void MotionControl::platformResetSlot()
{
    PlatformReset();
    m_changeFlag=1;
    m_ModeFlag=0;
    m_changeFlag=0;
}

/* ******************************
 * 点击平台急停按钮，执行此槽函数
 * *****************************/
void MotionControl::platformHaltSlot()
{
    Halt();
}

/* ******************************
 * 点击取消急停按钮，执行此槽函数
 * *****************************/
void MotionControl::platformCancelHaltSlot()
{
    CancelHalt();
}

/******************************
 * 点击平台复位按钮，运动到中间位置，此为执行各种运动的初始位置
 * 采用位置模式实现此功能
 * ***************************/
void MotionControl::platformToMiddleSlot()
{
    m_changeFlag=1;
    m_ModeFlag=2;

    //计算运行速度
    m_speedX=(0-g_px)/5;
    m_speedY=(0-g_py)/5;
    m_speedZ=(PlatformPara::T0(2)+140-g_pz)/5;
    m_speedRoll=(0-g_roll)/5;
    m_speedYaw=(0-g_yaw)/5;;
    m_speedPitch=(0-g_pitch)/5;

    m_actualRunTime=0;
    m_setRunTime=6;

    m_changeFlag=0;
}



/* *************************************
 * 点击运动模式中位置模式开始按钮，触发此槽函数
 * ***********************************/
void MotionControl::startPositionModeSlot(int x,int y,int z,int roll,int yaw,int pitch,int runTime)
{
    m_changeFlag=1;
    m_ModeFlag=2;

    //计算运行速度
    m_speedX=x/runTime;
    m_speedY=y/runTime;
    m_speedZ=z/runTime;
    m_speedRoll=roll/runTime;
    m_speedYaw=yaw/runTime;
    m_speedPitch=pitch/runTime;

    m_actualRunTime=0;
    m_setRunTime=runTime;


    m_changeFlag=0;
}

/* *************************************
 * 点击轨迹规划中开始运动按钮，首先触发TrajectoryPlanning::startTrajectroyPlanningSlot函数
 * TrajectoryPlanning::startTrajectroyPlanningSlot发出startTrajectroyPlanningSignal信号，并传递路径数组的地址
 * TrajectoryPlanning::startTrajectroyPlanningSignal信号跟此槽函数连接，触发此槽函数
 * ***********************************/
void MotionControl::startTrajectoryPlanningSlot(QVector<QVector<double>>* path)
{
    m_changeFlag=1;
    m_trajectoryPath=nullptr;
    if(path!=nullptr && !path->empty())
    {
        m_ModeFlag=3;
        m_trajectoryPath=path;
        m_trajectoryPtr=0;
    }
    m_changeFlag=0;
}


/* ***************************************
 * 体感仿真槽函数,首先点击体感仿真主控面板开始运动按钮，通过MainWindow将主控面板的参数发送到此槽函数中，同时设定对应的MotionFlag
 * ****************************************/
void MotionControl::startWashoutSlot(double value,double time,double slopeTime,int mode)
{
    m_changeFlag=1;
    m_ModeFlag=4;
    m_washout->reset(); //所有滤波器等都需要重置
    m_value=value;
    m_time=time;
    m_slopeTime=slopeTime;
    m_mode=mode;

    m_runtime=0;
    m_changeFlag=0;
}


