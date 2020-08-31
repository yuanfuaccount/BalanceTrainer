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



class SerialPort:public QObject
{
    Q_OBJECT
public:
    SerialPort(QObject* parent=nullptr);
    SerialPort(const QString filename,const QString COM);
    ~SerialPort();

public slots:
    void readDataSlot();
    void openSerialPortSlot();
    void closeSerialPortslot();
    void saveDataSlot();


    void startTimerSlot();
    void setAngleZeroSlot(); //记录初始角度

signals:
    void portOpenedSignal();
    void initAgnleFinishedSignal();
    
private:
    QString filename; //文件名称
    QString comname; //蓝牙端口
    QSerialPort* serial;  //串口操作
    QTimer* timer;  //定时器用来将数据写入csv
    QFile* outfile;

    double initangle; //初始时角度
    short acc[3];
    short w[3];
    short angle[3];
    short quer[4];
};

#endif // SERIALPORT_H
