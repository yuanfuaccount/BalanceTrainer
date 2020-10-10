#ifndef FUZZYCONTROL_H
#define FUZZYCONTROL_H



#include <QVector>


class FuzzyControl
{
public:
    FuzzyControl();

    double trimf(double x,int a,int b,int c); //三角隶属度函数
    double realizeAcc(double AccErr,double AccDif); //模糊控制实现函数
    double realizeW(double WErr,double WDif);

private:
    static const int NUM=5; //模糊隶属度的论域
    static const int THRE1=5; //阈值1
    static const int THRE2=50; //阈值2
    static const double VN;
    static const double N;
    static const double Z;
    static const double P;
    static const double VP;



    QVector<QVector<double>> m_rule; //模糊控制规则

    QVector<QVector<int>> m_AccEFunc; //加速度偏差隶属度函数的相关值,NUM*3
    QVector<QVector<int>> m_WEFunc; //角速度偏差隶属度函数的相关值,NUM*3
    QVector<QVector<int>> m_AccWDFunc; //加速度，角速度微分隶属度函数的相关值,NUM*3

    double m_AccErr,m_WErr,m_AccDif,m_WDif;
    double m_eAccRatio[NUM],m_eWRatio[NUM],m_dAccRatio[NUM],m_dWRatio[NUM]; //分别存储加速度偏差，角速度偏差，加速度微分，角速度微分对应的隶属度
};

#endif // FUZZYCONTROL_H
