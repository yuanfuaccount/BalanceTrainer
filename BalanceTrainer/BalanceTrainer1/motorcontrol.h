#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include "Eigen/Eigen/Core"
#include "Eigen/Eigen/Geometry"


//UDP指令参数
class UDPAppData
{
public:
        ushort I16ConfirmCode=0x55aa; //初始确认码
        ushort I16PassCode = 0x0000;//通过码
        ushort I16FunctionCode = 0x1401;//功能码：0x1401-相对对时间码,0x1301-绝对时间播放
        ushort I16ChannelCode = 0x0001;//通道号：六轴模式
        ushort I16WhoAcceptCode = 0xffff;//谁接收
        ushort I16WhoReplyCode = 0x0000;//谁回复，默认不回复

        int I32PlayLine = 0x00000000;//指令序号
        int I32PlayTime = 0x00000064;//时间（单位：毫秒，1401相对时间为指令的执行时间，1301绝对时间下相邻两帧指令里的时间相减为执行时间）

        //六轴位置信息？
        uint32_t I32PlayXpos = 0x00000000;//第1轴位置信息
        uint32_t I32PlayYpos = 0x00000000;//第2轴位置信息
        uint32_t I32PlayZpos = 0x00000000;//第3轴位置信息
        uint32_t I32PlayUpos = 0x00000000;//第4轴位置信息
        uint32_t I32PlayVpos = 0x00000000;//第5轴位置信息
        uint32_t I32PlayWpos = 0x00000000;//第6轴位置信息

        ushort I16BaseDoutCode = 0x0000;//基础开关量输出？

        ushort I16DacOneCode = 0x0000;//第1路模拟量输出？
        ushort I16DacTwoCode = 0x0000;//第2路模拟量输出？
        ushort I16ExtDoutCode = 0x0000;//扩展12路模拟量输出？
};


//UDP指令功能码
class UDPDataFunctionCode
{
public:
    static const uint16_t FunctionCode_Report_DnReg = 0x1001;//MBOX上报功能码
    static const uint16_t FunctionCode_Read_DnFnReg = 0x1101;//读寄存器功能码
    static const uint16_t FunctionCode_Write_DnFnReg = 0x1201;//写寄存器功能码
    static const uint16_t FunctionCode_AbsTimePlay = 0x1301;//绝对时间播放
    static const uint16_t FunctionCode_DeltaTimePlay = 0x1401;//相对时间播放
};



class UDPSend
{
public:
    UDPSend()
    {
    }
    ~UDPSend()
    {
    }

    void Ushort2Qbytearray(QVector<ushort>& src,QVector<uint8_t>& dst);
    uint GetPulse(double AcessDistance);

    void WriteRegCmd(ushort ChannelCode, ushort RegAddress, ushort RegValue); //写寄存器指令
    void ReadRegCmd(ushort ChannelCode, ushort RegAddress,ushort RegNum);  //读取寄存器参数

    void PlayActionCmd(uint32_t XPos,uint32_t YPos,uint32_t ZPos,uint32_t UPos,uint32_t VPos,uint32_t WPos,bool PointMove=false);  //位置动作控制指令

    void GetPositionCmd(void);  //获取位置指令

    void ReceivePositionData(void);//获取UDP数据

private:
    QUdpSocket UDPSocket;

};


//封装底层通讯的基本控制指令
void PlatformReset(void);  //平台复位指令
void Halt(void); //急停指令
void CancelHalt(void); //取消急停


//位置，速度反解函数
Eigen::Matrix3d RotationMatrix(double Yaw,double Pitch,double Roll);
QVector<double> PositionReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll);  //已知空间位置，求
QVector<double> SpeedReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll,
                             double Vx,double Vy,double Vz,double Wx,double Wy,double Wz);


#endif // MOTORCONTROL_H

