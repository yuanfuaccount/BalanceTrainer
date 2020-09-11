#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QDataStream>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include "IMUDataProcess.h"


class SerialPort:public QObject
{
    Q_OBJECT
public:
    SerialPort(QObject* parent=nullptr);
    SerialPort(const QString filename,const QString COM);
    ~SerialPort();



public slots:
    void readDataSlot();//读取串口数据，只要有数据到来，就读取
    void openSerialPortSlot();  //线程开启时，调用此槽函数，打开串口
    void closeSerialPortslot(); //线程关闭时，调用此槽函数，关闭串口

    void startDataCollectSlot();  //开启数据采集，即开启定时器
    void endDataCollectSlot();  //关闭定时器

    void setAngleZeroSlot(); //角度初始化，将初始角度置零
    void timeoutSlot();  //定时器函数，采集数据
    void processDataSlot(); //处理数据，当定时器采集数据放入allData后，调用此槽函数开始数据处理，主要包括关键点检测，保存到CSV中

signals:
    void portOpenedSignal(); //串口开启完成时，释放此信号
    void initAgnleFinishedSignal(); //角度初始化完成，释放此信号
    void processDataSignal(); //当timer停止时，释放此信号，开始数据处理


    
private:
    QString filename; //文件名称
    QString comname; //蓝牙端口
    QSerialPort* serial;  //串口操作
    QFile* outfile; //输出文件名称
    Filter* filter;  //滤波器
    QTimer* timer;  //定时器，用于数据采集，每20ms采集一次
    GaitPhaseDetection* detector;

    QVector<QVector<double>> allData; //保存所有数据
    double initangle; //初始时角度
    short acc[3];
    short w[3];
    short angle[3];
    short quer[4];
};


#endif // SERIALPORT_H
