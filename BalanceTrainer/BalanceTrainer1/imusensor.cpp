#include "imusensor.h"



FootSensor::FootSensor(QString filename, QString com)
{
    usart=new SerialPort(filename,com,1);  //1为脚部传感器
    detector=new GaitPhaseDetection(11); //设置窗口大小为11

    connect(usart,&SerialPort::processDataSignal,detector,&GaitPhaseDetection::gaitPhaseDataProcessSlot); //一旦串口接收数据完成，开始处理数据
}

FootSensor::~FootSensor()
{
    delete usart;
    delete detector;
}

WaistSensor::WaistSensor(QString filename, QString com)
{
    usart=new SerialPort(filename,com,0); //0为腰部传感器
    detector=new GaitSymmetry();

    connect(usart,&SerialPort::processDataSignal,detector,&GaitSymmetry::gaitSymmetryDataProcessSlot);
}

WaistSensor::~WaistSensor()
{
    delete usart;
    delete detector;
}
