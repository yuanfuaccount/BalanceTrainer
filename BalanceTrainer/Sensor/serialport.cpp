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
    stepTotal=0;
    stepEffecitve=0;
    avgGatiPhaseTime.resize(5);

}

double SerialPort::deltaT=0.02;


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

    //初步标记
    for(int i=0;i<allData.size();i++)
    {
        int type=detector->isKeyPoint(allData[i]); //type是窗口中心点的类型
        if(type!=-1)
        {
            allData[i-detector->size()/2].push_back(type); //说明窗口中心元素的类型
        }
    }

    //二次标记
    bool zeroStart=false;
    bool firstWXPeak=false;
    for(int i=1;i<allData.size()-1;i++)
    {
        if(allData[i].size()==10 && allData[i-1].size()==10 && allData[i+1].size()==10)
        {
            if(allData[i][9]==1)
                stepTotal++;
            if(allData[i][9]==4 && allData[i-1][9]==4 && allData[i+1][9]==0) //HO
            {
                allData[i][9]=5;
                zeroStart=true;
                firstWXPeak=false; //清除标志位
            }
            else if(allData[i][9]==3 && zeroStart) //零加速度起始点后第一个WX零点为TO
            {
                allData[i][9]=6; //TO点
                zeroStart=false;
            }
            else if(allData[i][9]==2 && firstWXPeak==false) //第一个WX峰值
            {
                firstWXPeak=true;
            }
            else if(allData[i][9]==2 && firstWXPeak) //第二个WX峰值为HS
            {
                allData[i][9]=7; //HS点
                firstWXPeak=false;
            }
            else if(allData[i][9]==4 && allData[i-1][9]==0 && allData[i+1][9]==4) //TS
                allData[i][9]=8;

            QString data="";
            data=data+QString::number(allData[i][0],'f',4)+",";
            data=data+QString::number(allData[i][1],'f',4)+",";
            data=data+QString::number(allData[i][2],'f',4)+",";
            data=data+QString::number(allData[i][3],'f',4)+",";
            data=data+QString::number(allData[i][4],'f',4)+",";
            data=data+QString::number(allData[i][5],'f',4)+",";
            data=data+QString::number(allData[i][6],'f',4)+",";
            data=data+QString::number(allData[i][7],'f',4)+",";
            data=data+QString::number(allData[i][8],'f',4)+",";
            data=data+QString::number(static_cast<int>(allData[i][9]),10)+",\n";

            QByteArray ba=data.toLatin1();
            const char* data1=ba.data();
            outfile->write(data1);
        }
    }
    outfile->close();

    //数据计算
    int HO=0,TO=0,HS=0,TS=0;
    double sumHOTime=0,sumswingTime=0,sumHSTime=0,sumstanceTime=0,sumCycle=0;
    for(int i=1;i<allData.size()-1;i++)
    {
        //统计每个周期中四个点是否都存在，都存在才计算数据,一个周期从HO开始
        if(allData[i].size()==10)
        {
            //新的一个周期起点
            if(allData[i][9]==5)
            {
                //上一个周期的所有点完整检测到
                if(HO!=0 && TO!=0 && HS!=0 && TS!=0 && HO<TO && TO<HS && HS<TS && TS<i)
                {
                    //脚跟离地期时长,摆动相时长，承重加载期时长，完全站立期时长
                    double HOTime=(TO-HO)*deltaT;
                    double swingTime=(HS-TO)*deltaT;
                    double HSTime=(TS-HS)*deltaT;
                    double stanceTime=(i-TS)*deltaT;
                    double cycle=(i-HO)*deltaT;
                    gaitPhaseTime.push_back({HOTime,swingTime,HSTime,stanceTime,cycle});
                    stepEffecitve++; //检测到有效的一步
                    sumHOTime+=HOTime;
                    sumswingTime+=swingTime;
                    sumHSTime+=HSTime;
                    sumstanceTime+=stanceTime;
                    sumCycle+=cycle;
                }
                HO=i;
                TO=0;
                HS=0;
                TS=0;
            }
            else if(allData[i][9]==6.0)
            {
                if(HO!=0 && TO==0 && HS==0 && TS==0)
                    TO=i;
                else
                {
                    HO=0;
                    TO=0;
                    HS=0;
                    TS=0;
                }


            }
            else if(allData[i][9]==7.0)
            {
                if(HO!=0 && TO!=0 && HS==0 && TS==0)
                {
                    HS=i;
                    TS=0;
                }
                else
                {
                    HO=0;
                    TO=0;
                    HS=0;
                    TS=0;
                }
            }
            else if(allData[i][9]==8.0)
            {
                if(HO!=0 && TO!=0 && HS!=0 && TS==0)
                    TS=i;
                else
                {
                    HO=0;
                    TO=0;
                    HS=0;
                    TS=0;
                }
            }
        }
    }
    //分别是脚跟着地相，完全站立相，脚跟离地相，摆动相,周期
    avgGatiPhaseTime[0]=sumHSTime/gaitPhaseTime.size();
    avgGatiPhaseTime[1]=sumstanceTime/gaitPhaseTime.size();
    avgGatiPhaseTime[2]=sumHOTime/gaitPhaseTime.size();
    avgGatiPhaseTime[3]=sumswingTime/gaitPhaseTime.size();
    avgGatiPhaseTime[4]=sumCycle/gaitPhaseTime.size();

    emit dataProcessFinished();

}








