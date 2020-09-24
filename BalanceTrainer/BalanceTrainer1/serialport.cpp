#include "serialport.h"
#include <QObject>
#include <QDebug>
#include <synchapi.h>


SerialPort::SerialPort(QObject* parent):
    QObject(parent)
{
}

SerialPort::SerialPort(const QString filename,const QString COM):
    filename(filename),
    comname(COM)
{
}




SerialPort::~SerialPort()
{ 
}


void SerialPort::openSerialPortSlot()
{
    serial=new QSerialPort;  //申请新的串口

    timer=new QTimer();

    outfile=new QFile(filename); //创建保存文件
    if(outfile->open(QIODevice::ReadWrite))
        outfile->write("AccX,AccY,AccZ,Wx,Wy,Wz,AngleX,AngleY,AngleZ,\n");

    //串口相关设置
    serial->setPortName(comname);
    while(!serial->open(QIODevice::ReadWrite));
    serial->setBaudRate(QSerialPort::Baud115200);//设置波特率为115200
    serial->setDataBits(QSerialPort::Data8);//设置数据位8
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

    connect(serial,&QSerialPort::readyRead,this,&SerialPort::readDataSlot);
    connect(timer,&QTimer::timeout,this,&SerialPort::saveDataSlot);

    emit portOpenedSignal();

}


void SerialPort::startTimerSlot()
{
    timer->start(20);
}

void SerialPort::stopTimerSlot()
{
    timer->stop();
}

void SerialPort::closeSerialPortslot()
{
    timer->stop();
    outfile->close();

    delete timer;
    delete outfile;

    serial->clear();
    serial->close();
    serial->deleteLater();
}



void SerialPort::readDataSlot()
{
    QByteArray buf1;
    buf1=serial->readAll();
    size_t buflen=buf1.length();
    if(buflen>2000)  buflen=2000;
    char buf[2000];
    memcpy(buf,buf1.data(),buflen);
    while(buflen>=11)
    {
        if(buf[0]!=0x55)
        {
            buflen--;
            memcpy(&buf[0],&buf[1],buflen);
            continue;
        }
        switch (buf[1]) {
        case 0x51: memcpy(acc,&buf[2],6);break;
        case 0x52: memcpy(w,&buf[2],6);break;
        case 0x53: memcpy(angle,&buf[2],6);break;
        //case 0x59: memcpy(quer,&buf[2],8);break;
        }
        buflen-=11;
        memcpy(&buf[0],&buf[11],buflen);
    }
}


void SerialPort::setAngleZeroSlot()
{
    int i=0;
    double sum=0;
    while(i<20)
    {
        sum+=angle[0]*180/32768.0;
        i++;
        Sleep(50);
    }
    initangle=sum/20;
    emit initAgnleFinishedSignal();
}


void SerialPort::saveDataSlot()
{
    QString data="";
    double acc0=acc[0]*16.0/32768.0;
    double acc1=acc[1]*16.0/32768.0;
    double acc2=acc[2]*16.0/32768.0;
    double w0=w[0]*2000/32768.0;
    double w1=w[1]*2000/32768.0;
    double w2=w[2]*2000/32768.0;
    double angle0=angle[0]*180/32768.0-initangle;
    double angle1=angle[1]*180/32768.0-initangle;
    double angle2=angle[2]*180/32768.0-initangle;
//    double quer0=quer[0]/32768.0;
//    double quer1=quer[1]/32768.0;
//    double quer2=quer[2]/32768.0;
//    double quer3=quer[3]/32768.0;
    data=data+QString::number(acc0,'f',4)+",";
    data=data+QString::number(acc1,'f',4)+",";
    data=data+QString::number(acc2,'f',4)+",";
    data=data+QString::number(w0,'f',4)+",";
    data=data+QString::number(w1,'f',4)+",";
    data=data+QString::number(w2,'f',4)+",";
    data=data+QString::number(angle0,'f',4)+",";
    data=data+QString::number(angle1,'f',4)+",";
    data=data+QString::number(angle2,'f',4)+",\n";
//    data=data+QString::number(quer0,'f',4)+",";
//    data=data+QString::number(quer1,'f',4)+",";
//    data=data+QString::number(quer2,'f',4)+",";
//    data=data+QString::number(quer3,'f',4)+",\n";

    char* data1=data.toLatin1().data();
    outfile->write(data1);

}




