#ifndef GAITPHASEDIVISION_H
#define GAITPHASEDIVISION_H

#include <QVector>
#include <deque>
#include <QObject>
#include <QFile>
#include <math.h>

using namespace std;

class Butterworth
{
private:
    QVector<QVector<double>> x; //9*3
    QVector<QVector<double>> y; //9*3
    QVector<double> bz;
    QVector<double> az;
public:
    Butterworth()
    {
        bz={0.2066,0.4131,0.2066};
        az={1.0000,-0.3695,0.1958};
        x.resize(9);
        y.resize(9);
        for(int i=0;i<x.size();i++)
        {
            x[i].resize(3);
            y[i].resize(3);
        }
    }
    void filter(QVector<double>& rawdata);
};


/* ***********************
 * 步态时相划分
 * 利用角度最大值和最小值求出Heel Strike 和 Toe Off的点
 * 利用加速度零检测来检测ToeStrike 和 Heel Off的点
 * ***********************/
class GaitPhaseDetection:public QObject
{
    Q_OBJECT
public:
    /* *******************************************
     * 底层的数据处理，主要用于检测峰值，零点等特殊点
     * *****************************************/
    GaitPhaseDetection(QObject* parent=nullptr);
    GaitPhaseDetection(int n):winSize(n),minAngThred(-10),AccThred(0.02),AccUnderThred(0)
    {
        maxWXThred=50;
        stepTotal=0;
        stepEffecitve=0;
        avgGatiPhaseTime.resize(5);
    }
    ~GaitPhaseDetection();

    void allDataClear(); //为了实现多次测量，在下一次测量时，上一次的所有历史数据要清除

    void push(QVector<double> input);
    void pop(QVector<double> input);
    double minAng(); //返回最小角度
    int size();  //返回缓存窗口的大小

    double maxWX(); //返回最大角速度
    QVector<double> getWindowMiddle();
    int isKeyPoint(QVector<double>& input);

    /*************************************
     * 二次数据处理，针对第一次提取的关键点，计算二次特征
     * *********************************/

    //统计数据,由于需要在主窗口访问，设置为public
    QVector<QVector<double>> gaitPhaseTime; //各个步态时相的时间占比
    int stepTotal; //总的步数
    int stepEffecitve; //有效步数，即能完整检测出四个相的一步
    QVector<double> avgGatiPhaseTime; //脚跟离地相，摆动相，脚跟着地相，完全站立相，总周期时长

public slots:
    void gaitPhaseDataProcessSlot(QVector<QVector<double>>* allData,QFile* outfile); //当串口接收数据完成时，调用此函数处理数据

signals:
    void gaitPhaseDataProcessFinished(); //处理数据完成，发送此信号给UI界面显示

private:
    deque<QVector<double>> data;
    deque<double> Acc; //和加速度
    deque<double> WX;

    deque<double> minAngQ; //单调递增队列，返回窗口中角度最小值,用于判断走过的步数
    deque<double> maxWXQ; //单调递减队列，返回窗口中WX最大值，用于判定HS

    int winSize;  //窗口大小

    double minAngThred; //角度最小阈值，小于此值才认为是角度最小值
    double AccThred; //加速度阈值，小于此值才认为静止
    int AccUnderThred; //窗口内低于加速度阈值的点数

    double maxWXThred; //WX最大阈值，大于此值才认为是WX最大值

    static double deltaT; //采样时间间隔

};



#endif // FILTER_H
