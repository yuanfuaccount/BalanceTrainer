#ifndef IMUSENSOR_H
#define IMUSENSOR_H

#include <QObject>
#include "serialport.h"
#include "GaitPhaseDivision.h"
#include "gaitsymmetry.h"

//定义脚部传感器
class FootSensor:public QObject
{
    Q_OBJECT
public:
    FootSensor(QObject* parent=nullptr);
    FootSensor(QString filename, QString com);
    ~FootSensor();
    SerialPort* usart;  //传感器的串口接收
    GaitPhaseDetection* detector; //传感器的后续数据处理
};

//定义腰部传感器
class WaistSensor:public QObject
{
    Q_OBJECT
public:
    WaistSensor(QObject* parent=nullptr);
    WaistSensor(QString filename, QString com);
    ~WaistSensor();
    SerialPort* usart;  //传感器的串口接收
    GaitSymmetry* detector; //传感器的后续数据处理
};

#endif // IMUSENSOR_H
