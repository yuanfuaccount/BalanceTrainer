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
    filter=new Filter();
    outfile=new QFile(filename); //创建保存文件
    timer=new QTimer();
    detector=new GaitPhaseDetection(11); //11为窗口大小

    //串口相关设置
    serial->setPortName(comname);
    while(!serial->open(QIODevice::ReadWrite));
    serial->setBaudRate(QSerialPort::Baud115200);//设置波特率为115200
    serial->setDataBits(QSerialPort::Data8);//设置数据位8
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

    connect(serial,&QSerialPort::readyRead,this,&SerialPort::readDataSlot);
    connect(timer,&QTimer::timeout,this,&SerialPort::timeoutSlot);
    connect(this,&SerialPort::processDataSignal,this,&SerialPort::processDataSlot);

    emit portOpenedSignal();
}


void SerialPort::closeSerialPortslot()
{

    delete detector;
    delete serial;
    delete filter;
    delete outfile;
    delete timer;
}


void SerialPort::readDataSlot()
{
    QByteArray buf1;
    buf1=serial->readAll();
    size_t buflen=buf1.length();
    if(buflen>2000)  buflen=2000;
    static unsigned char buf[2000];
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

//开启或关闭定时器,只能开启一次，即只能采集一次数据
void SerialPort::startDataCollectSlot()
{
    timer->start(20);
}

void SerialPort::endDataCollectSlot()
{
    serial->clear();
    serial->close();
    timer->stop();
    emit processDataSignal();
}

//定时器数据采集任务
void SerialPort::timeoutSlot()
{
    QVector<double> rawdata(9,0);
    rawdata[0]=acc[0]*16.0/32768.0;
    rawdata[1]=acc[1]*16.0/32768.0;
    rawdata[2]=acc[2]*16.0/32768.0;
    rawdata[3]=w[0]*2000/32768.0;
    rawdata[4]=w[1]*2000/32768.0;
    rawdata[5]=w[2]*2000/32768.0;
    rawdata[6]=angle[0]*180/32768.0-initangle;
    rawdata[7]=angle[1]*180/32768.0-initangle;
    rawdata[8]=angle[2]*180/32768.0-initangle;
//    double quer0=quer[0]/32768.0;
//    double quer1=quer[1]/32768.0;
//    double quer2=quer[2]/32768.0;
//    double quer3=quer[3]/32768.0;

    filter->filter(rawdata);

    allData.push_back(rawdata);

    if(allData.size()>=500)
    {
        serial->clear();
        serial->close();
        timer->stop();
        emit processDataSignal();
    }
}

void SerialPort::processDataSlot()
{
    //处理并保存数据
    if(outfile->open(QIODevice::ReadWrite))
        outfile->write("AccX,AccY,AccZ,Wx,Wy,Wz,AngleX,AngleY,AngleZ,type,\n");

    for(auto mem:allData)
    {
        int type=detector->isKeyPoint(mem);


        if(type!=-1)
        {
            QVector<double> rawdata=detector->getWindowMiddle();
            QString data="";
            data=data+QString::number(rawdata[0],'f',4)+",";
            data=data+QString::number(rawdata[1],'f',4)+",";
            data=data+QString::number(rawdata[2],'f',4)+",";
            data=data+QString::number(rawdata[3],'f',4)+",";
            data=data+QString::number(rawdata[4],'f',4)+",";
            data=data+QString::number(rawdata[5],'f',4)+",";
            data=data+QString::number(rawdata[6],'f',4)+",";
            data=data+QString::number(rawdata[7],'f',4)+",";
            data=data+QString::number(rawdata[8],'f',4)+",";
            data=data+QString::number(type,10)+",\n";

            QByteArray ba=data.toLatin1();
            const char* data1=ba.data();
            outfile->write(data1);
        }
    }
    outfile->close();
}








