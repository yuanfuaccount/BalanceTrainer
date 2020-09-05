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
    void saveDataSlot();  //定时中断中调用此函数，将数据保存到CSV


    void startTimerSlot();  //开启数据采集
    void setAngleZeroSlot(); //角度初始化，将初始角度置零

signals:
    void portOpenedSignal(); //串口开启完成时，释放此信号
    void initAgnleFinishedSignal(); //角度初始化完成，释放此信号
    
private:
    QString filename; //文件名称
    QString comname; //蓝牙端口
    QSerialPort* serial;  //串口操作
    QTimer* timer;  //定时器用来将数据写入csv
    QFile* outfile;
    Filter* filter;
    GaitPhaseDetection* detector;

    double initangle; //初始时角度
    short acc[3];
    short w[3];
    short angle[3];
    short quer[4];
};

#endif // SERIALPORT_H
