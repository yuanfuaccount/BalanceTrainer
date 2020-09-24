#include "motorcontrol.h"
#include <QUdpSocket>
#include <QByteArray>
#include <stdlib.h>
#include <string.h>
#include <QString>
#include <QDebug>
#include "Eigen/Eigen/Core"
#include "Eigen/Eigen/Geometry"
#include <math.h>
#include "config.h"

#include <iostream>


/* *****************************************
 * 此C文件用于底层控制指令，即与控制器的UDP通信函数，主要功能
 * 1. 读取MDBOX的寄存器
 * 2. 写MDBOX寄存器
 * 3. 平台动作指令，只能设置间隔时间及下一步要到达的位置
 * 4. 关于平台复位，急停等基本操作
 * ***************************************/



//UDP相关函数

/* *******************************************
 * 将16位的数据转化为字节8位，方便发送
 * *src:待转换的ushort类型
 * *dst:转换的字节类型地址
 * num:src数组的大小
 * ********************************************/
void UDPSend::Ushort2Qbytearray(QVector<ushort> &src,QVector<uint8_t> &dst)
{
    for(int i=0;i<src.size();i++)
    {
        dst.push_back((src[i] & 0xff00)>>8);
        dst.push_back(src[i] & 0x00ff);
    }
}

/* **************************************
 * 计算脉冲数，因为位置控制和编码器返回值都是通过脉冲数表示的
 * AcessDistance：要行走的距离
 * ************************************/
uint UDPSend::GetPulse(double AcessDistance)
{
    uint NUM_Pulse;
    NUM_Pulse = (uint)(GearRatio * AcessDistance * PulsePerCycle / PitchDistance);
    return NUM_Pulse;
}

/**********************************
 * 写寄存器操作
 * ChannelCode: 0x0000:写FN参数但不保存  0x0001:写FN参数并保存  0x0002：写CX控制参数
 * RegValue:待写的寄存器的内容
 * TargetIP:发送的目标IP，默认进行广播
 * *******************************/
void UDPSend::WriteRegCmd(ushort ChannelCode, ushort RegAddress, ushort RegValue)  //默认参数在声明和定义中只需写一个
{
    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据

    UDPAppData UDPSendData;

    I16UdpTxBuffer.push_back(UDPSendData.I16ConfirmCode);
    I16UdpTxBuffer.push_back(UDPSendData.I16PassCode);
    I16UdpTxBuffer.push_back(0x1201);  //功能码
    I16UdpTxBuffer.push_back(ChannelCode);  //通道号
    I16UdpTxBuffer.push_back(UDPSendData.I16WhoAcceptCode); //谁接收
    I16UdpTxBuffer.push_back(0xff); //谁回复
    I16UdpTxBuffer.push_back(RegAddress); //参数寄存器起始地址
    I16UdpTxBuffer.push_back(0x0001);//寄存器个数
    I16UdpTxBuffer.push_back(RegValue); //写寄存器的值

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    QString TargetIP="255.255.255.255"; //默认广播
    UDPSocket.writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress(TargetIP),I16MboxRxPort);
}

/* *******************************************************
 * 读寄存器操作，先给机器发一个读取指令，然后机器返回读取的指令
 * ChannelCode：  0x0000:读取DN参数 0x0001:读FN参数
 * RegAddress：参数寄存器的起始地址 RegNum:寄存器数量
 * ******************************************************/
void UDPSend::ReadRegCmd(ushort ChannelCode,ushort RegAddress,ushort RegNum)
{
    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据

    UDPAppData UDPSendData;

    I16UdpTxBuffer.push_back(UDPSendData.I16ConfirmCode); //确认码
    I16UdpTxBuffer.push_back(UDPSendData.I16PassCode); //通过码
    I16UdpTxBuffer.push_back(0x1101);  //功能码
    I16UdpTxBuffer.push_back(ChannelCode);  //通道号
    I16UdpTxBuffer.push_back(UDPSendData.I16WhoAcceptCode); //谁接收
    I16UdpTxBuffer.push_back(0xff); //谁回复
    I16UdpTxBuffer.push_back(RegAddress); //参数寄存器起始地址
    I16UdpTxBuffer.push_back(RegNum);//寄存器个数

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    QString TargetIP="255.255.255.255"; //默认广播
    UDPSocket.writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress(TargetIP),I16MboxRxPort);
}


void UDPSend::PlayActionCmd(uint32_t XPos,uint32_t YPos,uint32_t ZPos,uint32_t UPos,uint32_t VPos,uint32_t WPos,bool PointMove)
{
    UDPAppData UDPSendData;

    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据

    I16UdpTxBuffer.push_back(UDPSendData.I16ConfirmCode);//确认码
    I16UdpTxBuffer.push_back(UDPSendData.I16PassCode); //通过码
    I16UdpTxBuffer.push_back(UDPSendData.I16FunctionCode); //功能码，相对时间播放
    I16UdpTxBuffer.push_back(UDPSendData.I16ChannelCode); //默认6轴数据
    I16UdpTxBuffer.push_back(UDPSendData.I16WhoAcceptCode); //谁接收，默认广播模式
    I16UdpTxBuffer.push_back(UDPSendData.I16WhoReplyCode);
    I16UdpTxBuffer.push_back((ushort)((UDPSendData.I32PlayLine & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(UDPSendData.I32PlayLine & 0x0000ffff)); //指令序列，默认0X00000000

    if(PointMove==true) //点动模式
    {
        UDPSendData.I32PlayTime=0;
    }

    I16UdpTxBuffer.push_back((ushort)((UDPSendData.I32PlayTime & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(UDPSendData.I32PlayTime & 0x0000ffff));// 播放时间

    I16UdpTxBuffer.push_back((ushort)((XPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(XPos & 0x0000ffff));  //X轴位置信息
    I16UdpTxBuffer.push_back((ushort)((YPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(YPos & 0x0000ffff));  //Y轴位置信息
    I16UdpTxBuffer.push_back((ushort)((ZPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(ZPos & 0x0000ffff));  //Z轴位置信息
    I16UdpTxBuffer.push_back((ushort)((UPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(UPos & 0x0000ffff));  //U轴位置信息
    I16UdpTxBuffer.push_back((ushort)((VPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(VPos & 0x0000ffff));  //V轴位置信息
    I16UdpTxBuffer.push_back((ushort)((WPos & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back((ushort)(WPos & 0x0000ffff));  //W轴位置信息

    I16UdpTxBuffer.push_back(UDPSendData.I16BaseDoutCode); //12路数字输出，默认关闭
    I16UdpTxBuffer.push_back(UDPSendData.I16DacOneCode);
    I16UdpTxBuffer.push_back(UDPSendData.I16DacTwoCode); //2路模拟量输出，默认都关闭

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    UDPSocket.writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress("255.255.255.255"),I16MboxRxPort);
};


/* *********************************
 * 获取6轴绝对位置
 * 读取Dn寄存器的值,每个寄存器16个字节
 * Dn6/Dn7:X轴位置  Dn8/Dn9:Y轴  DnA/DnB:Z轴 Dn21/Dn22:U轴  Dn23/Dn24:V轴  Dn25/Dn26:W轴
 * 参数：RegAddr:接收哪个寄存器的数据, addr：接收地址 port：端口号
 * 返回值：X,Y,Z或U,V,W的位置值(单位：脉冲数)
 * *********************************/
void UDPSend::GetPositionCmd(void)
{
    ReadRegCmd(0x0000,0x0006,0x0006);
    ReadRegCmd(0x0000,0x0021,0x0006);
}

void UDPSend::ReceivePositionData()
{
    uint8_t RcvFlag=0;
    uint8_t receivedata[50];

    QHostAddress addr;
    quint16 port; //用于保存数据地址和端口号
    uint x=0,y=0,z=0,u=0,v=0,w=0;
    while(RcvFlag!=0x11)
    {
        UDPSocket.readDatagram((char*)receivedata,50,&addr,&port);
        if(receivedata[0]==0x55 && receivedata[1]==0xaa ) //确认码
        {
            if(receivedata[2]==0x00 && receivedata[3]==0x00) //通过码
            {
                if(receivedata[4]==0x11 && receivedata[5]==0x02) //功能码
                {
                    if(receivedata[6]==0x00 && receivedata[7]==0x00)  //读DN参数
                    {
                        //8,9,10,11四个字节代表谁接收，谁回复
                            if(receivedata[12]==0x00 && receivedata[13]==0x06 && receivedata[14]==0x00 && receivedata[15]==0x06) //接收地址和寄存器数
                            {
                                x=(receivedata[16]<<24|receivedata[17]<<16 | receivedata[18]<<8 | receivedata[19]); //X轴数据
                                y=(receivedata[20]<<24|receivedata[21]<<16 | receivedata[22]<<8 | receivedata[23]); //Y轴数据
                                z=(receivedata[24]<<24|receivedata[25]<<16 | receivedata[26]<<8 | receivedata[27]); //Y轴数据
                                RcvFlag=RcvFlag | 0x01;
                            }
                            if(receivedata[12]==0x00 && receivedata[13]==0x21 && receivedata[14]==0x00 && receivedata[15]==0x06) //接收地址和寄存器数
                            {
                                u=(receivedata[16]<<24|receivedata[17]<<16 | receivedata[18]<<8 | receivedata[19]); //U轴数据
                                v=(receivedata[20]<<24|receivedata[21]<<16 | receivedata[22]<<8 | receivedata[23]); //V轴数据
                                w=(receivedata[24]<<24|receivedata[25]<<16 | receivedata[26]<<8 | receivedata[27]); //W轴数据
                                RcvFlag=RcvFlag | 0x10;
                            }


                    }
                }
            }
        }

    }
    MotorPosMutex.lock();
    xpos=x;
    ypos=y;
    zpos=z;
    upos=u;
    vpos=v;
    wpos=w;
    MotorPosMutex.unlock();
}



/* ******************************
 * 平台复位指令，向Cn 00 寄存器写入0x00
 * ChannelCode:0x0002 写CX寄存器
 * RegAddress：地址0x0000
 * RegValue: 0x0000 写入参数
 * *****************************/
void PlatformReset(void)
{
    UDPSend sender;
    sender.WriteRegCmd(0x0002,0x0000,0x0000);
}

/* *********************************
 * 急停指令：向Fn 090写入1
 * ChannelCode： 0x0000 写Fn寄存器不保存
 * RegAddress： 0x0090
 * RegValue: 0x0001
 * ********************************/
void Halt(void)
{
    UDPSend sender;
    sender.WriteRegCmd(0x0000,0x0090,0x0001);
}

/* *******************************
 * 取消急停：向Fn 090写入0
 * ChannelCode： 0x0000 写Fn寄存器不保存
 * RegAddress： 0x0090
 * RegValue: 0x0000
 * *******************************/
void CancelHalt(void)
{
    UDPSend sender;
    sender.WriteRegCmd(0x0000,0x0090,0x0000);
}





/* *****************************
 * 平台控制部分，主要包含位置逆解
 * ***************************/


/* ******************************************
 * 求旋转矩阵
 * 输入：三个欧拉角Yaw,Pitch,Roll
 * 输出：Matrix3d R
 * *****************************************/
Eigen::Matrix3d RotationMatrix(double Yaw,double Pitch,double Roll)
{
    Eigen::Vector3d euler(Yaw,Pitch,Roll);  //Yaw,pitch,roll,分别对应绕Z轴偏航，绕Y轴俯仰，绕X轴滚转
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



