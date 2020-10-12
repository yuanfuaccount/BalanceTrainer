#ifndef FILTER_H
#define FILTER_H

#define secondHighPassAcc 0
#define secondLowPassAcc 1
#define secondHighPassAng 2
#define otolish 3
#define firstHighPassAcc 4  //一阶高通滤波器
#define firstIntegral 5  //一阶积分
#define secondIntegral 6 //二阶积分
#define semicirulare 7


#include <QObject>
#include <QVector>

//定义相关滤波器，主要是1，2高通或低通IIR滤波器
class Filter
{
public:
    Filter();
    void setFilterType(int type);
    double filter(double input);
    void reset();


private:
    int m_filterType; //滤波器类型
    QVector<double> m_input; //记录历史输入，4*1，m_input[3]当前输入，m_input[2]上一次输入，m_input[1]上上次输入
    QVector<double> m_output; //记录历史输出，4*1，m_output[3]当前输出，m_output[2]上一次输出，m_output[1]上上次输出

    /* ******************************
     * 滤波器bz,az系数，5*3二维数组
     * bz[0],az[0]为二阶高通加速度滤波器参数，传递函数 s^2/(s^2+5s+6.25)
     * bz[1],az[1]为二阶低通加速度滤波器参数，传递函数 25/(s^2+10s+25)
     * bz[2],az[2]为二阶高通角速度滤波参数，传递函数 s^2/(s^2+2s+1)
     * bz[3],az[3]为耳石模型参数，传递函数0.4*(13.2s+1)/((5.33s+1)*(0.66s+1))
     * 已知模拟滤波器传递函数，求bz,az参数，用matlab的bilinear函数
     * *****************************/
    QVector<QVector<double>> m_bz;
    QVector<QVector<double>> m_az;

    /* ********************************
     * 由于半规管模型的bz,az均为四阶，其参数单独处理
     * 半规管传递函数(30s*5.3s)/((30s+1)*(5.3s+1)*(0.1s+1)
     * ********************************/
    QVector<double> m_bz_semicirulare;
    QVector<double> m_az_semicirulare;

};

#endif // FILTER_H
