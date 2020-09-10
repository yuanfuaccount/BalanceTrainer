#include "IMUDataProcess.h"
#include <QVector>
#include <QString>
#include <deque>
#include <math.h>
#include <QDebug>
#include <iostream>

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
void GaitPhaseDetection::push(QVector<double> input)
{

    data.emplace_back(input);
    double temp=sqrt(input[0]*input[0]+input[1]*input[1]+input[2]*input[2])-1;
    //qDebug()<<temp;
    Acc.emplace_back(temp);
    if(temp<=AccThred) AccUnderThred++;
    //检测的是AngX的极值，在input中的序号为6
    while(!maxQ.empty() && maxQ.back()<input[6])
        maxQ.pop_back();
    maxQ.push_back(input[6]);
    while(!minQ.empty() && minQ.back()>input[6])
        minQ.pop_back();
    minQ.push_back(input[6]);
}

//返回最大角度AngX
double GaitPhaseDetection::maxAng()
{
    return maxQ.front();
}

//返回最小角度AngX
double GaitPhaseDetection::minAng()
{
    return minQ.front();
}

//队头元素如果是out，删除它
void GaitPhaseDetection::pop(QVector<double> out)
{
    if(!maxQ.empty() && maxQ.front()==out[6])
        maxQ.pop_front();
    if(!minQ.empty() && minQ.front()==out[6])
        minQ.pop_front();
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
 * 检测是否是关键点
 * 窗口没填满，返回-1
 * 非特殊情况，返回0
 * AngX最大点：返回1
 * AngX最小点：返回2
 * Acc低于阈值：返回3
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
        if(maxAng()==getWindowMiddle()[6] && maxAng()>maxAngThred)  res=1;
        else if(minAng()==getWindowMiddle()[6] && minAng()<minAngThred) res=2;
        else if(AccUnderThred>winSize/2) res=3;

        //移除窗口开头的元素
        pop(data.front());
        if(Acc.front()<=AccThred)
            AccUnderThred--;
        Acc.pop_front();
        data.pop_front();
    }
    if(res==1) stepNum++; //检测到峰值，说明走过一步

    return res;
}
