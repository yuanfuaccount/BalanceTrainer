#include "GaitPhaseDivision.h"
#include <QVector>
#include <QString>
#include <deque>
#include <math.h>
#include <QDebug>
#include <iostream>
#include <QFile>

/***************************************
* IIR滤波器，参数bz,az分别代表滤波器传递函数的分子和分目系数
* bz=[b0,b1,b2……]
* az=[1,a1,a2……]
* 对应传函：H(z)=Y(z)/X(z)=sum(b(k)*z^(-k))/(1+sum(a(k)*z^(-k)))
* b(k)中k从0~M,a(k)中k从1~N
* 输出y(n)=sum(b(k)*x(n-k))-sum(a(k)*y(n-k))
* 可以参考：
* https://wenku.baidu.com/view/adec241352d380eb62946d82.html
* https://blog.csdn.net/ReCclay/article/details/84672725
***************************************/


/* ***************************************
 * 二阶低通滤波器
 * 对加速度，角速度，角度进行原始滤波
 * rawdata:1*9,分别代表accx,,accy,accz,wx,wy,wz,angx,angy,angz
 * x:9*3,每一行分别代表accx,,accy,accz,wx,wy,wz,angx,angy,angz,每一列代表[n-2],[n-1],[n]
 * y:9*3 每一行分别代表accx,,accy,accz,wx,wy,wz,angx,angy,angz,每一列代表[n-2],[n-1],[n]
 * **************************************/
void Filter::filter(QVector<double>& rawdata)
{
    for(int i=0;i<9;i++)
    {
        x[i][0]=x[i][1];
        x[i][1]=x[i][2];
        x[i][2]=rawdata[i];
        y[i][0]=y[i][1];
        y[i][1]=y[i][2];
        y[i][2]=bz[0]*x[i][2]+bz[1]*x[i][1]+bz[2]*x[i][0]-az[1]*y[i][1]-az[2]*y[i][0];
        rawdata[i]=y[i][2];
    }
}


/* ************************************
 * 步态时相检测算法
 * **********************************/
double GaitPhaseDetection::deltaT=0.02;

GaitPhaseDetection::~GaitPhaseDetection()
{}

void GaitPhaseDetection::push(QVector<double> input)
{
    data.emplace_back(input);
    double temp=sqrt(input[0]*input[0]+input[1]*input[1]+input[2]*input[2])-1;
    //qDebug()<<temp;
    Acc.emplace_back(temp);
    WX.push_back(input[3]);

    if(temp<=AccThred) AccUnderThred++;

    while(!minAngQ.empty() && minAngQ.back()>input[6])
        minAngQ.pop_back();
    minAngQ.push_back(input[6]);

    while(!maxWXQ.empty() && maxWXQ.back()<input[3])
        maxWXQ.pop_back();
    maxWXQ.push_back(input[3]);
}


//返回最小角度AngX
double GaitPhaseDetection::minAng()
{
    return minAngQ.front();
}

double GaitPhaseDetection::maxWX()
{
    return maxWXQ.front();
}


//队头元素如果是out，删除它
void GaitPhaseDetection::pop(QVector<double> out)
{
    if(!minAngQ.empty() && minAngQ.front()==out[6])
        minAngQ.pop_front();

    if(!maxWXQ.empty() && maxWXQ.front()==out[3])
        maxWXQ.pop_front();
}



//返回缓存窗口的大小
int GaitPhaseDetection::size()
{
    return static_cast<int>(data.size());
}

//获取窗口中间值
QVector<double> GaitPhaseDetection::getWindowMiddle()
{
    if(size()%2==1) //窗口是满的
        return data[winSize/2];
    else  //删掉了开头的元素
        return data[winSize/2-1];
}



/* *****************************
 * 检测是否是关键点,利用加速度检测TS和HO，利用AngX的峰值检测TO和HS
 * 窗口没填满，返回-1
 * 非特殊情况，返回0
 * AngX最小点：返回1
 * WX最大值点，返回2
 * WX零点，返回3
 * Acc低于阈值：返回4
 * ***************************/
int GaitPhaseDetection::isKeyPoint(QVector<double>& input)
{
    int res=0;
    //窗口还没填满
    if(size()<winSize-1)
    {
        push(input);
        return -1;
    }
    //窗口满了
    else
    {
        push(input);
        if(minAng()==getWindowMiddle()[6] && minAng()<minAngThred)  res=1;
        else if(maxWX()==getWindowMiddle()[3] && maxWX()>maxWXThred) res=2;
        else if(AccUnderThred>winSize/2) res=4; //必须res=4放在res=3的前面，因为res=4和3的情况经常冲突，在冲突时，以res=4为标准
        else if(WX[winSize/2]*WX[winSize/2-1]<0 && fabs(WX[winSize/2])<fabs(WX[winSize/2-1])) res=3;
        else if(WX[winSize/2]*WX[winSize/2+1]<0 && fabs(WX[winSize/2])<fabs(WX[winSize/2+1])) res=3;

        //移除窗口开头的元素
        pop(data.front());
        if(Acc.front()<=AccThred)
            AccUnderThred--;
        WX.pop_front();
        Acc.pop_front();
        data.pop_front();
    }

    return res;
}

/* ************************************
 * 步态时相二次检测
 * 1. 通过isKeyPoint函数检测WX最大值，最小值，所有小于AccThred的值
 * 2. 对一次关键点进行二次检测，获取HS,TS,HO,TO四个类型关键点，同时求取每个步态周期的步态时相
 * 3. 输入参数：串口采集的原始数据allData，会修改该数据，在最后添加每个数据点的type
 * ***********************************/
void GaitPhaseDetection::gaitPhaseDataProcessSlot(QVector<QVector<double>>* allData,QFile* outfile)
{
    //处理并保存数据
    if(outfile->open(QIODevice::ReadWrite))
        outfile->write("AccX,AccY,AccZ,Wx,Wy,Wz,AngleX,AngleY,AngleZ,type,\n");

    //初步标记
    for(int i=0;i<(*allData).size();i++)
    {
        int type=isKeyPoint((*allData)[i]); //type是窗口中心点的类型
        if(type!=-1)
        {
            (*allData)[i-winSize/2].push_back(type); //说明窗口中心元素的类型
        }
    }

    //二次标记
    bool zeroStart=false;
    bool firstWXPeak=false;
    for(int i=1;i<(*allData).size()-1;i++)
    {
        if((*allData)[i].size()==10 && (*allData)[i-1].size()==10 && (*allData)[i+1].size()==10)
        {
            if((*allData)[i][9]==1)
                stepTotal++;
            if((*allData)[i][9]==4 && (*allData)[i-1][9]==4 && (*allData)[i+1][9]==0) //HO
            {
                (*allData)[i][9]=5;
                zeroStart=true;
                firstWXPeak=false; //清除标志位
            }
            else if((*allData)[i][9]==3 && zeroStart) //零加速度起始点后第一个WX零点为TO
            {
                (*allData)[i][9]=6; //TO点
                zeroStart=false;
            }
            else if((*allData)[i][9]==2 && firstWXPeak==false) //第一个WX峰值
            {
                firstWXPeak=true;
            }
            else if((*allData)[i][9]==2 && firstWXPeak) //第二个WX峰值为HS
            {
                (*allData)[i][9]=7; //HS点
                firstWXPeak=false;
            }
            else if((*allData)[i][9]==4 && (*allData)[i-1][9]==0 && (*allData)[i+1][9]==4) //TS
                (*allData)[i][9]=8;

            QString data="";
            data=data+QString::number((*allData)[i][0],'f',4)+",";
            data=data+QString::number((*allData)[i][1],'f',4)+",";
            data=data+QString::number((*allData)[i][2],'f',4)+",";
            data=data+QString::number((*allData)[i][3],'f',4)+",";
            data=data+QString::number((*allData)[i][4],'f',4)+",";
            data=data+QString::number((*allData)[i][5],'f',4)+",";
            data=data+QString::number((*allData)[i][6],'f',4)+",";
            data=data+QString::number((*allData)[i][7],'f',4)+",";
            data=data+QString::number((*allData)[i][8],'f',4)+",";
            data=data+QString::number(static_cast<int>((*allData)[i][9]),10)+",\n";

            QByteArray ba=data.toLatin1();
            const char* data1=ba.data();
            outfile->write(data1);
        }
    }

    outfile->close();

    //数据计算
    int HO=0,TO=0,HS=0,TS=0;
    double sumHOTime=0,sumswingTime=0,sumHSTime=0,sumstanceTime=0,sumCycle=0;
    for(int i=1;i<(*allData).size()-1;i++)
    {
        //统计每个周期中四个点是否都存在，都存在才计算数据,一个周期从HO开始
        if((*allData)[i].size()==10)
        {
            //新的一个周期起点
            if((*allData)[i][9]==5)
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
            else if((*allData)[i][9]==6.0)
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
            else if((*allData)[i][9]==7.0)
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
            else if((*allData)[i][9]==8.0)
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

    emit gaitPhaseDataProcessFinished();
}


