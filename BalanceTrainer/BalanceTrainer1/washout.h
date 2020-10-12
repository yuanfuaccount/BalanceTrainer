#ifndef WASHOUT_H
#define WASHOUT_H

#include <QObject>
#include <QVector>
#include <math.h>

#include "filter.h"
#include "fuzzycontrol.h"
#include "config.h"
#include "kinematics.h"

/* ************************************
 * 11个滤波器对应的编号按照matlab中的位置顺序排列，编号见图片，分别如下
 * 0：过滤输入的耳石滤波器,m_desiredAccOutput由输入经过此滤波器得到
 * 1：
 * ***********************************/


class WashOut:public QObject
{
    Q_OBJECT
public:
    WashOut(QObject* parent=nullptr);

    QVector<double> getWashOut(QVector<double> input);
    QVector<double> calAccW(double AccX,double AccY,double AccZ,double WX,double WY,double WZ,double AccTime,double WTime,double AccSlopeTime,double WSlopeTime,double runtime);
    void reset();  //完成一次洗出算法之后需要重置所有参数


private:
    static const double INTERVAL;
    const double LIMITRATE[3]={0.0628,0.0523,0.0454};


    Filter m_otolishFilter[3][2]; //每个Acc需要2个耳石滤波器
    Filter m_highPassAccFilter[3][2]; //每个Acc需要两个高通加速度滤波
    Filter m_lowPassAccFilter[3][2]; //每个Acc需要两个低通加速度滤波
    Filter m_semicirulareFilter[3][2]; //每个W需要两个半规管滤波器
    Filter m_highPassWFilter[3][2]; //每个W需要两个高通角速度滤波
    Filter m_lowPassWFilter[3]; //每个W需要1个低通滤波器

    Filter m_firstHighPassAccFilter[3]; //每个Acc需要一个一阶高通滤波器
    Filter m_firstIntegral[3]; //每个W需要一个一阶积分
    Filter m_secondIntegral[3]; //每个Acc需要一个二阶积分


    FuzzyControl m_fuzzy; //模糊控制器

    double m_senseOutput[6]; //最终输出感觉加速度和角速度
    double m_desiredOutput[6];
    double m_fuzzyOutput[6]; //模糊控制器输出
    double m_lastInput[6]; //上一次的输入

    /* ***************************
     * 此部分用于计算位移
     * ****************************/
    double m_posOutput[6]; //位移输出
    double m_lastPosOutput[6]; //上一次位移输出
    double m_highPassOutput[6];//高通滤波输出


    double m_lowPassAccOutput[3]; //低通加速度滤波输出
    double m_lastLowPassAccOutput[3]; //上一次低通加速度滤波通道输出，即倾斜协调输出

};

#endif // WASHOUT_H
