#ifndef FILTER_H
#define FILTER_H
#include <QVector>
#include <deque>

using namespace std;

class Filter
{
private:
    QVector<QVector<double>> x; //9*3
    QVector<QVector<double>> y; //9*3
    QVector<double> bz;
    QVector<double> az;
public:
    Filter()
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
class GaitPhaseDetection
{
private:
    deque<QVector<double>> data;
    deque<double> Acc; //和加速度
    deque<double> maxQ; //单调递减队列，返回窗口中角度最大值
    deque<double> minQ; //单调递增队列，返回窗口中角度最小值
    int winSize;  //窗口大小
    double maxAngThred; //角度最大阈值，大于此值才认为是角度最大值
    double minAngThred; //角度最小阈值，小于此值才认为是角度最小值
    double AccThred; //加速度阈值，小于此值才认为静止
public:
    GaitPhaseDetection(){}
    GaitPhaseDetection(int n):winSize(n),maxAngThred(30),minAngThred(-10),AccThred(0.1){}

    void push(QVector<double> input);
    void pop(QVector<double> input);
    double maxAng(); //返回最大角度
    double minAng(); //返回最小角度
    int size();  //返回缓存窗口的大小

    QVector<double> getWindowMiddle();

    int isKeyPoint(QVector<double> input);
};

#endif // FILTER_H
