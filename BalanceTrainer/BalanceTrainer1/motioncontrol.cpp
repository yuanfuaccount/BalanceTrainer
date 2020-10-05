#include "motioncontrol.h"



/* *********************************
 * 平台运动学逆解，此部分不包装成类，为直接函数调用
 * *******************************/


/* ******************************************
 * 求旋转矩阵
 * 输入：三个欧拉角Yaw,Pitch,Roll
 * 输出：Matrix3d R
 * *****************************************/
Eigen::Matrix3d RotationMatrix(double Yaw,double Pitch,double Roll)
{
    //首先将角度转化为弧度
    Eigen::Vector3d euler(Yaw*PI/180,Pitch*PI/180,Roll*PI/180);  //Yaw,pitch,roll,分别对应绕Z轴偏航，绕Y轴俯仰，绕X轴滚转
    Eigen::AngleAxisd YawAngle(euler[0],Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd PitchAngle(euler[1],Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd RollAngle(euler[2],Eigen::Vector3d::UnitX());

    Eigen::Matrix3d R;
    R=YawAngle*PitchAngle*RollAngle;
    return R;
}


/* ***********************************
 * 平台位置逆解
 * 欧拉角按Z,Y,X的顺序旋转,用弧度表示
 * 输入参数：3个方向的平移+Z,Y,X的欧拉角
 * 返回值：六根电动缸的长度
 * *********************************/
QVector<double> PositionReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll)
{
    Eigen::Matrix3d R;
    R=RotationMatrix(Yaw,Pitch,Roll);

    Eigen::Vector3d t(PosX,PosY,PosZ);
    Eigen::Vector3d T=t+PlatformPara::T0;

    //3X6的矩阵，每一列为上平台的一个坐标
    Eigen::Matrix<double,3,6> P0;
    P0<<PlatformPara::P1_0,PlatformPara::P2_0,PlatformPara::P3_0,PlatformPara::P4_0,PlatformPara::P5_0,PlatformPara::P6_0;
    Eigen::Matrix<double,3,6> P;
    Eigen::Matrix<double,3,6> TT;
    TT<<T,T,T,T,T,T;
    P=R*P0+TT;  //上平台铰点在下平台坐标

    Eigen::Matrix<double,3,6> B0;
    B0<<PlatformPara::B1_0,PlatformPara::B2_0,PlatformPara::B3_0,PlatformPara::B4_0,PlatformPara::B5_0,PlatformPara::B6_0;
    Eigen::Matrix<double,3,6> ScalarL; //每根缸的长度向量
    ScalarL=P-B0;

    //求出六个缸的长度
    QVector<double> L;
    for(int i=0;i<6;i++)
        L.push_back(sqrt(ScalarL.col(i).transpose()*ScalarL.col(i)));
    return L;
}


/* *************************************
 * 速度逆解公式，输入平台现阶段位姿以及末端需要的速度，求解每个电动缸伸缩速度
 * 输入：6维位姿坐标(Posx,PosY,PosZ,Yaw,Pitch,Roll)，6维速度广义坐标(Vx,Vy,Vz,Wx,Wy,Wz)
 * 输出：每个电动缸伸缩速度
 * ************************************/
QVector<double> SpeedReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll,
                             double Vx,double Vy,double Vz,double Wx,double Wy,double Wz)
{
    //1.求出6个上铰点速度矢量Vpi
    Eigen::Matrix3d R;
    R=RotationMatrix(Yaw,Pitch,Roll);

    //上铰点的速度Vpi=V+W×RPi_0
    Eigen::Matrix<double,3,6> P0; //上铰点相对于动坐标系的坐标；
    P0<<PlatformPara::P1_0,PlatformPara::P2_0,PlatformPara::P3_0,PlatformPara::P4_0,PlatformPara::P5_0,PlatformPara::P6_0;

    Eigen::Matrix<double,3,6> VP; //上铰点的速度
    Eigen::Vector3d V(Vx,Vy,Vz); //上平台速度
    Eigen::Vector3d W(Wx,Wy,Wz); //平台角速度
    for(int i=0;i<6;i++)
    {
        VP.col(i)=V+W.cross(R*P0.col(i));  //求出6个上铰点的速度矢量
    }

    //2. 求出6个电动缸单位向量ni
    Eigen::Matrix<double,3,6> P;//上铰点在定坐标系中的坐标
    Eigen::Vector3d t(PosX,PosY,PosZ);
    Eigen::Vector3d T=t+PlatformPara::T0;
    Eigen::Matrix<double,3,6> TT;
    TT<<T,T,T,T,T,T;
    P=TT+R*P0;

    for(int i=0;i<6;i++)
    {
        double l=sqrt(P.col(i).transpose()*P.col(i)); //第i根电动缸长度
        P.col(i)=P.col(i)/l;  //此时求得的P即为6根电动缸的单位向量
    }

    //3. ni.Vpi即为6根电动缸的伸缩速度标量
    QVector<double> ScalarVP;  //速度标量
    for(int i=0;i<6;i++)
    {
        double v=P.col(i).dot(VP.col(i));
        ScalarVP.push_back(v);
    }
    return ScalarVP;
}


MotionControl::MotionControl(QObject* parent)
{
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
    }
}

/* **********************************
 * 线程开启时初始化槽函数，主要是开启定时器，连接定时器中断信号和相关的处理槽函数
 * *********************************/
void MotionControl::threadStartSlot()
{
    sock=new QUdpSocket(this);
    timer=new QTimer(this);
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
 * 点击平台复位按钮，执行此槽函数
 * ********************************/
void MotionControl::platformResetSlot()
{
    PlatformReset();
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

