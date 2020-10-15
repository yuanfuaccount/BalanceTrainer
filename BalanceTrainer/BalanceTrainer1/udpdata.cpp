#include "udpdata.h"


UdpData::UdpData(QObject* parent)
{
    this->setParent(parent);
}

/* *******************************************
 * 将16位的数据转化为字节8位，方便发送
 * *src:待转换的ushort类型
 * *dst:转换的字节类型地址
 * num:src数组的大小
 * ********************************************/
void UdpData::Ushort2Qbytearray(QVector<ushort> &src,QVector<uint8_t> &dst)
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
int UdpData::GetPulse(double AcessDistance)
{
    int NUM_Pulse;
    NUM_Pulse = static_cast<int>(GearRatio * AcessDistance * PulsePerCycle / PitchDistance);
    return NUM_Pulse;
}

/**********************************
 * 写寄存器操作
 * ChannelCode: 0x0000:写FN参数但不保存  0x0001:写FN参数并保存  0x0002：写CX控制参数
 * RegAddress: 代写寄存器的起始地址
 * RegNum: 待写寄存器的个数
 * 待写寄存器的值，每个寄存器的的值为一个short类型
 * *******************************/
void UdpData::WriteRegCmd(ushort ChannelCode, ushort RegAddress, ushort RegNum, QVector<ushort> RegValue)  //默认参数在声明和定义中只需写一个
{
    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据


    I16UdpTxBuffer.push_back(I16ConfirmCode);
    I16UdpTxBuffer.push_back(I16PassCode);
    I16UdpTxBuffer.push_back(0x1201);  //功能码
    I16UdpTxBuffer.push_back(ChannelCode);  //通道号
    I16UdpTxBuffer.push_back(I16WhoAcceptCode); //谁接收
    I16UdpTxBuffer.push_back(I16WhoReplyCode); //谁回复
    I16UdpTxBuffer.push_back(RegAddress); //参数寄存器起始地址
    I16UdpTxBuffer.push_back(RegNum);//寄存器个数
    if(RegNum!=RegValue.size())
    {
        qDebug()<<"寄存器个数与参数不匹配";
        return;
    }
    for(int i=0;i<RegValue.size();i++)
    {
        I16UdpTxBuffer.push_back(RegValue[i]); //写寄存器的值
    }

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    sock->writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress(TargetIP),I16MboxRxPort);
}

/* *******************************************************
 * 读寄存器操作，先给机器发一个读取指令，然后机器返回读取的指令
 * ChannelCode：  0x0000:读取DN参数 0x0001:读FN参数
 * RegAddress：参数寄存器的起始地址 RegNum:寄存器数量
 * ******************************************************/
void UdpData::ReadRegCmd(ushort ChannelCode,ushort RegAddress,ushort RegNum)
{
    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据


    I16UdpTxBuffer.push_back(I16ConfirmCode); //确认码
    I16UdpTxBuffer.push_back(I16PassCode); //通过码
    I16UdpTxBuffer.push_back(0x1101);  //功能码
    I16UdpTxBuffer.push_back(ChannelCode);  //通道号
    I16UdpTxBuffer.push_back(I16WhoAcceptCode); //谁接收
    I16UdpTxBuffer.push_back(0xffff); //谁回复
    I16UdpTxBuffer.push_back(RegAddress); //参数寄存器起始地址
    I16UdpTxBuffer.push_back(RegNum);//寄存器个数

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    sock->writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress(TargetIP),I16MboxRxPort);
}

/* ***********************************************
 * 运动控制函数
 * XPos~WPos: 每一个电机要到达的位置(单位：mm)，需要转化为脉冲数(此处的位置为相对原点的位置)
 * PointMove: 是否进行点动模式，点动模式下，相对播放时间为0
 * **********************************************/
bool UdpData::PlayActionCmd(double XPos,double YPos,double ZPos,double UPos,double VPos,double WPos,bool PointMove)
{

    QVector<ushort> I16UdpTxBuffer;  //存放UDP数据信息
    QVector<uint8_t> U8UdpDataSend; //存放发送的字节数据

    I16UdpTxBuffer.push_back(I16ConfirmCode);//确认码
    I16UdpTxBuffer.push_back(I16PassCode); //通过码
    I16UdpTxBuffer.push_back(0x1401); //功能码，相对时间播放
    I16UdpTxBuffer.push_back(0x0001); //6轴数据
    I16UdpTxBuffer.push_back(I16WhoAcceptCode); //谁接收，默认广播模式
    I16UdpTxBuffer.push_back(I16WhoReplyCode);
    I16UdpTxBuffer.push_back(static_cast<ushort>((I32PlayLine & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(I32PlayLine & 0x0000ffff)); //指令序列，默认0X00000000

    if(PointMove==true) //点动模式
    {
        I32PlayTime=0;
    }

    I16UdpTxBuffer.push_back(static_cast<ushort>((I32PlayTime & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(I32PlayTime & 0x0000ffff));// 播放时间

    int XPul=GetPulse(XPos);
    int YPul=GetPulse(YPos);
    int ZPul=GetPulse(ZPos);
    int UPul=GetPulse(UPos);
    int VPul=GetPulse(VPos);
    int WPul=GetPulse(WPos);
    if(XPul<0 || XPul>=MaxAccessPul ||YPul<0 || YPul>=MaxAccessPul || ZPul<0 || ZPul>=MaxAccessPul || UPul<0 || UPul>=MaxAccessPul || VPul<0 || VPul>=MaxAccessPul || WPul<0 || WPul>=MaxAccessPul)
    {
        qDebug()<<"行程超出运动范围，运动停止";
        return false;
    }
    g_xpul=XPul;
    g_ypul=YPul;
    g_zpul=ZPul;
    g_upul=UPul;
    g_vpul=VPul;
    g_wpul=WPul;


    I16UdpTxBuffer.push_back(static_cast<ushort>((g_xpul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_xpul & 0x0000ffff));  //X轴位置信息
    I16UdpTxBuffer.push_back(static_cast<ushort>((g_ypul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_ypul & 0x0000ffff));  //Y轴位置信息
    I16UdpTxBuffer.push_back(static_cast<ushort>((g_zpul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_zpul & 0x0000ffff));  //Z轴位置信息
    I16UdpTxBuffer.push_back(static_cast<ushort>((g_upul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_upul & 0x0000ffff));  //U轴位置信息
    I16UdpTxBuffer.push_back(static_cast<ushort>((g_vpul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_vpul & 0x0000ffff));  //V轴位置信息
    I16UdpTxBuffer.push_back(static_cast<ushort>((g_wpul & 0xffff0000)>>16));
    I16UdpTxBuffer.push_back(static_cast<ushort>(g_wpul & 0x0000ffff));  //W轴位置信息

    I16UdpTxBuffer.push_back(I16BaseDoutCode); //12路数字输出，默认关闭
    I16UdpTxBuffer.push_back(I16DacOneCode);
    I16UdpTxBuffer.push_back(I16DacTwoCode); //2路模拟量输出，默认都关闭

    Ushort2Qbytearray(I16UdpTxBuffer,U8UdpDataSend);
    sock->writeDatagram((char*)U8UdpDataSend.begin(),U8UdpDataSend.size(),QHostAddress(TargetIP),I16MboxRxPort);
    return true;
};

/* ******************************
 * 平台复位指令，向Cn 00 寄存器写入0x00
 * ChannelCode:0x0002 写CX寄存器
 * RegAddress：地址0x0000
 * RegValue: 0x0000 写入参数
 * *****************************/
void UdpData::PlatformReset(void)
{
    QVector<ushort> RegVal;
    RegVal.push_back(0x0000);
    WriteRegCmd(0x0002,0x0000,1,RegVal);
    g_px=0;g_py=0;g_pz=PlatformPara::T0(2);g_roll=0;g_yaw=0;g_pitch=0;
    g_xpul=0;g_ypul=0;g_zpul=0;g_upul=0;g_vpul=0;g_wpul=0;

}

/* *********************************
 * 急停指令：向Fn 090写入1
 * ChannelCode： 0x0000 写Fn寄存器不保存
 * RegAddress： 0x0090
 * RegValue: 0x0001
 * ********************************/
void UdpData::Halt(void)
{
    QVector<ushort> RegVal;
    RegVal.push_back(0x0001);
    WriteRegCmd(0x0000,0x0090,1,RegVal);
}

/* *******************************
 * 取消急停：向Fn 090写入0
 * ChannelCode： 0x0000 写Fn寄存器不保存
 * RegAddress： 0x0090
 * RegValue: 0x0000
 * *******************************/
void UdpData::CancelHalt(void)
{
    QVector<ushort> RegVal;
    RegVal.push_back(0x0000);
    WriteRegCmd(0x0000,0x0090,1,RegVal);
}





