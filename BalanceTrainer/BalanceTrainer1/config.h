#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <Eigen/Core>
#include <QMutex>

//电动缸硬件配置相关参数
extern ushort I16HostTxPort; //上位机发送端口，默认8410
extern ushort I16HostRxPort;  //接收端口
extern ushort I16MboxTxPort;
extern ushort I16MboxRxPort;

extern uint8_t WhoAcceptIPGroup;
extern uint8_t WhoAcceptIPNode;
extern uint8_t WhoReplyIPGroup;
extern uint8_t WhoReplyIPNode;

extern float AccessDistance;//电动缸行程
extern float PitchDistance;//电动缸导程
extern uint PulsePerCycle;//一圈脉冲数
extern double GearRatio;//减速比

//各个电动缸位置
extern uint xpos;
extern uint ypos;
extern uint zpos;
extern uint upos;
extern uint vpos;
extern uint wpos;
extern QMutex MotorPosMutex;


//上平台中心坐标
extern double px,py,pz;
extern double roll,pitch,yaw;
extern QMutex PlatformPosMutex;

//平台计算相关尺寸参数
/* *************************************************
 * 关于6自由度平台位置反解算法
 * 基本参数如下：下平台分布圆半径R1=655.32mm,上平台分布圆半径328.07mm，上平台原点相对于下平台的初始坐标(PosX0,PosY0,PosZ0)=（0，0，Z）
 * 上平台从X轴逆时针各点相对上平台坐标：
 * P1(213.23,249.33) P2(109.31,309.33) P3(-322.54,60) P4(-322.54,-60), P5(109.31,-309.33) P6(213.23,-249.33)
 * 下平台对应点相对下平台坐标系的坐标：
 * B1(652.56,60) B2(-274.32,595.14) B3(378.24,535.14) B4(-378.24,-535.14), B5(-274.32,-595.14) B6(652.56,-60)
* **************************************************/
class PlatformPara
{
public:
    static const double UpperRadius;
    static const double BaseRadius;

    static const Eigen::Vector3d B1_0;
    static const Eigen::Vector3d B2_0;
    static const Eigen::Vector3d B3_0;
    static const Eigen::Vector3d B4_0;
    static const Eigen::Vector3d B5_0;
    static const Eigen::Vector3d B6_0;

    static const Eigen::Vector3d P1_0;
    static const Eigen::Vector3d P2_0;
    static const Eigen::Vector3d P3_0;
    static const Eigen::Vector3d P4_0;
    static const Eigen::Vector3d P5_0;
    static const Eigen::Vector3d P6_0;

    static const Eigen::Vector3d T0;

};


#endif // CONFIG_H
