#include "platformcontrol.h"
#include "motorcontrol.h"
#include <QTimer>
#include "config.h"
#include <math.h>
#include <QVector>
#include <QFile>

/* ****************************
 * 此C文件在motorctrol的基础上，进行进一步封装，实现每个电机的速度，位置模式控制
 * ****************************/

void GetDataCmdThread::run()
{
    UDPSend ud;
    while(!exit)
    {
        ud.GetPositionCmd();
        ud.ReceivePositionData();
        msleep(100);   //每隔100ms发送读取电机位置的指令
    }
}


/* ***********************************
 * 速度控制指令，参数为每个轴的速度，单位mm/s,正值为伸长，负值为缩短
 * 速度控制每个100ms发送一次
 * 输入参数:Speed：每根电动缸的速度 mm/s， BasePos：启动速度命令时每根电动缸的位置,单位pulse  time:调用该命令经过的执行时间
 * *********************************/
void motorSpeedControl(QVector<double> Speed,QVector<uint32_t> BasePos,double time)
{
    //先要进行限速判断，具体限速值待定
     UDPSend ud;
     //转化为距离
    double XDis=Speed[0]*time;
    double YDis=Speed[1]*time;
    double ZDis=Speed[2]*time;
    double UDis=Speed[3]*time;
    double VDis=Speed[4]*time;
    double WDis=Speed[5]*time;

    //转化成相对脉冲
    uint XRPulse=ud.GetPulse(XDis);
    uint YRPulse=ud.GetPulse(YDis);
    uint ZRPulse=ud.GetPulse(ZDis);
    uint URPulse=ud.GetPulse(UDis);
    uint VRPulse=ud.GetPulse(VDis);
    uint WRPulse=ud.GetPulse(WDis);

    //转化成绝对脉冲
    uint XPul=XRPulse+BasePos[0];
    uint YPul=YRPulse+BasePos[1];
    uint ZPul=ZRPulse+BasePos[2];
    uint UPul=URPulse+BasePos[3];
    uint VPul=VRPulse+BasePos[4];
    uint WPul=WRPulse+BasePos[5];

    ud.PlayActionCmd(XPul,YPul,ZPul,UPul,VPul,WPul);
}

/* *******************************
 * 速度控制模式
 * 输入：
 * PlatformSpeed：XSpeed，YSpeed，ZSpeed:前后，左右，上下方向的平移速度，RollSpeed，PitchSpeed，YawSpeed：翻滚(绕x轴),俯仰(绕y轴)，偏航(绕z轴)的旋转速度。
 * BasePos：执行速度指令时每根电动缸的位置，单位pulse   time:速度指令运行时间
 * ******************************/
void PlatformSpeedControl(QVector<double> PlatformSpeed,QVector<uint32_t> BasePos,double time)
{
    //求旋转矩阵
    double w=sqrt(PlatformSpeed[3]*PlatformSpeed[3]+PlatformSpeed[4]*PlatformSpeed[4]+PlatformSpeed[5]*PlatformSpeed[5]);  //合角速度
    double ang=w*time;
    double a=PlatformSpeed[3]/w,b=PlatformSpeed[4]/w,c=PlatformSpeed[5]/w; //旋转轴单位向量
    Eigen::AngleAxisd rotation_vector(ang,Eigen::Vector3d(a,b,c));
    Eigen::Vector3d euler=rotation_vector.matrix().eulerAngles(2,1,0);  //得到欧拉角

    //求上平台的位置坐标
    PlatformPosMutex.lock();
    px=px+PlatformSpeed[0]*time;
    py=py+PlatformSpeed[1]*time;
    pz=pz+PlatformSpeed[2]*time;
    yaw=yaw+euler(0);
    pitch=pitch+euler(1);
    roll=roll+euler(2);
    PlatformPosMutex.unlock();

    //2.求速度逆解
    QVector<double> V=SpeedReverse(px,py,pz,yaw,pitch,roll,PlatformSpeed[0],PlatformSpeed[1],PlatformSpeed[2],PlatformSpeed[3],PlatformSpeed[4],PlatformSpeed[5]);

    motorSpeedControl(V,BasePos,time);
}

/* *********************************
 * 位置控制模式
 * 输入参数：Position:x,y,z:平移相对距离 Rang,Pang,Yang:RPY角相对旋转角度  time:运行时间，决定速度,范围0-30s
 * *******************************/
void PlatFormPositionControl(QVector<double> Position,ushort time)
{
    PlatformPosMutex.lock();
    px=px+Position[0];py=py+Position[1];pz=pz+Position[2];
    roll+=Position[3];pitch+=Position[4];yaw+=Position[5];
    PlatformPosMutex.unlock();

    QVector<double> L=PositionReverse(px,py,pz,yaw,pitch,roll); //每根电动缸的长度

    UDPSend ud;
    //转化为每个电动缸的脉冲
    uint XPul=ud.GetPulse(L[0]);
    uint YPul=ud.GetPulse(L[1]);
    uint ZPul=ud.GetPulse(L[2]);
    uint UPul=ud.GetPulse(L[3]);
    uint VPul=ud.GetPulse(L[4]);
    uint WPul=ud.GetPulse(L[5]);

    //控制点动模式的速度，向Fn013寄存器发送点动时间
    time=time*1000;  //转化成ms;
    ud.WriteRegCmd(0x0000,0x0d,time);

    //采用点动模式到达最终位置
    ud.PlayActionCmd(XPul,YPul,ZPul,UPul,VPul,WPul,true);
}


/* *********************************
 * 轨迹规划函数，轨迹点在matlab内已规划好
 * 只需要每隔100ms发送轨迹点就行了
 * 目前暂定轨迹点以头文件的形式包含到一个数组中
 * *******************************/
void ReadPathData(QVector<QVector<double>>& PathData)
{
    QFile fd("trajectorypath.txt");
    if(!fd.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug()<<"Open path file failed";
    QTextStream in(&fd);
    while(!in.atEnd())
    {
        QString line=in.readLine();
        QStringList strline=line.split(" ");
        QVector<double> linedata;
        for(int i=0;i<strline.length();i++)
        {
            linedata.push_back(strline[i].toDouble());
        }
        PathData.push_back(linedata);
        qDebug()<<linedata;
    }
    fd.close();
}

void ExecPath(QVector<double>& PathData)
{
    UDPSend ud;
    uint x=ud.GetPulse(PathData[0]);
    uint y=ud.GetPulse(PathData[1]);
    uint z=ud.GetPulse(PathData[2]);
    uint u=ud.GetPulse(PathData[3]);
    uint v=ud.GetPulse(PathData[4]);
    uint w=ud.GetPulse(PathData[5]);

    ud.PlayActionCmd(x,y,z,u,v,w);
}


