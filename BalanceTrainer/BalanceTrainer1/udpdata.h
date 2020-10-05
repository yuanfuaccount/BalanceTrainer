#ifndef UDPDATA_H
#define UDPDATA_H
#include <QObject>
#include <QVector>
#include <QDebug>
#include <QUdpSocket>
#include <QTimer>

#include "config.h"

/* *****************************************
 * 此C文件用于底层控制指令，即与控制器的UDP通信函数，主要功能
 * 1. 读取MDBOX的寄存器
 * 2. 写MDBOX寄存器
 * 3. 平台动作指令，只能设置间隔时间及下一步要到达的位置
 * 4. 关于平台复位，急停等基本操作
 * ***************************************/



//UDP指令功能码
class UDPFunctionCode
{
public:
    static const uint16_t FunctionCode_Report_DnReg = 0x1001;//MBOX上报功能码
    static const uint16_t FunctionCode_Read_DnFnReg = 0x1101;//读寄存器功能码
    static const uint16_t FunctionCode_Write_DnFnReg = 0x1201;//写寄存器功能码
    static const uint16_t FunctionCode_AbsTimePlay = 0x1301;//绝对时间播放
    static const uint16_t FunctionCode_DeltaTimePlay = 0x1401;//相对时间播放
};

class UdpData:public QObject
{
public:
    Q_OBJECT
    //UdpData()=default;

public:
    UdpData(QObject* parent=nullptr);
    bool PlayActionCmd(double XPos,double YPos,double ZPos,double UPos,double VPos,double WPos,bool PointMove=false);  //位置动作控制指令

    void PlatformReset(void);
    void Halt(void);
    void CancelHalt(void);

    void IncreasePlayLine()
    {
        I32PlayLine++;
    }

    QUdpSocket* sock;


private:

    void Ushort2Qbytearray(QVector<ushort>& src,QVector<uint8_t>& dst);
    int GetPulse(double AcessDistance); //计算脉冲，每个电机的控制量为脉冲
    void WriteRegCmd(ushort ChannelCode, ushort RegAddress,ushort RegNum,QVector<ushort> RegValue); //写寄存器指令
    void ReadRegCmd(ushort ChannelCode, ushort RegAddress,ushort RegNum);  //读取寄存器参数


    ushort I16ConfirmCode=0x55aa; //初始确认码
    ushort I16PassCode = 0x0000;//通过码
    ushort I16WhoAcceptCode = 0xffff;//谁接收
    ushort I16WhoReplyCode = 0xffff;//谁回复

    uint I32PlayLine = 0x00000000;//指令序号
    uint I32PlayTime = 0x00000064;//时间（单位：毫秒，1401相对时间为指令的执行时间，1301绝对时间下相邻两帧指令里的时间相减为执行时间）

    ushort I16BaseDoutCode = 0x0000;//基础开关量输出

    ushort I16DacOneCode = 0x0000;//第1路模拟量输出
    ushort I16DacTwoCode = 0x0000;//第2路模拟量输出

};

#endif // UDPDATA_H
