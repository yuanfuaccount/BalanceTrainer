#include "fuzzycontrol.h"

const double FuzzyControl::VN=-5.0;
const double FuzzyControl::N=-2.5;
const double FuzzyControl::Z=0.0;
const double FuzzyControl::P=2.5;
const double FuzzyControl::VP=5.0;

FuzzyControl::FuzzyControl()
{
    //模糊控制规则的初始化
    m_rule.resize(NUM);
    for(int i=0;i<NUM;i++)
    {
        m_rule[i].resize(NUM);
    }
    m_rule={{VN,VN,VN,N,Z},
            {VN,N,N,Z,Z},
            {N,N,Z,P,P},
            {Z,Z,P,P,VP},
            {Z,P,VP,VP,VP}}; //模糊控制规则

    //加速度偏差隶属度函数的一系列a,b,c的参数
    m_AccEFunc={{-5,-5,-2},{-5,-2,0},{-2,0,2},{0,2,5},{2,5,5}};
    //角速度偏差隶属度函数的相关值
    m_WEFunc={{-5,-5,-3},{-5,-3,-1},{-2,0,2},{1,3,5},{3,5,5}};
    //加速度，角速度微分隶属度函数的相关值,NUM*3
    m_AccWDFunc={{-50,-50,-10},{-50,-10,0},{-10,0,10},{0,10,50},{10,50,50}};

    m_AccErr=0;m_WErr=0;m_AccDif=0;m_WDif=0;

    for(int i=0;i<NUM;i++)
    {
        m_eAccRatio[i]=0;
        m_eWRatio[i]=0;
        m_dAccRatio[i]=0;
        m_dWRatio[i]=0;
    }
}



/* *****************************
 * 三角隶属度函数
 * 输入： x:输入值 a,b,c:三角隶属度函数的三个边界值
 * *******************************/
double FuzzyControl::trimf(double x,int a,int b,int c)
{
    double u=0;
    if(x>=a&&x<=b)
        u=(x-a)/(b-a);
    else if(x>b&&x<=c)
        u=(c-x)/(c-b);
    else
        u=0.0;
    return u;
}

/* ***************************
 * 加速度模糊控制函数实现，因为滤波器部分设计为单个数据滤波，因此模糊控制部分也设计为加速度和角速度模糊控制分开
 * 输入：AccErr加速度偏差  AccDif加速度微分
 * **************************/
double FuzzyControl::realizeAcc(double AccErr,double AccDif)
{
    m_AccErr=AccErr;  //加速度偏差
    m_AccDif=AccDif; //输入加速度微分

    //限幅
    if(m_AccErr<-1*THRE1) m_AccErr=-1*THRE1;
    if(m_AccErr>THRE1) m_AccErr=THRE1;
    if(m_AccDif<-1*THRE2) m_AccDif=-1*THRE2;
    if(m_AccDif>THRE2) m_AccDif=THRE2;

    //计算隶属度
    for(int i=0;i<NUM;i++)
    {
        m_eAccRatio[i]=trimf(m_AccErr,m_AccEFunc[i][0],m_AccEFunc[i][1],m_AccEFunc[i][2]); //加速度偏差隶属度
        m_dAccRatio[i]=trimf(m_AccDif,m_AccWDFunc[i][0],m_AccWDFunc[i][1],m_AccWDFunc[i][2]); //加速度微分隶属度
    }

    //模糊输出
    double AccNum=0; //分子
    double AccDen=0; //分母

    for(int i=0;i<NUM;i++)
    {
        for(int j=0;j<NUM;j++)
        {
            if(m_eAccRatio[i]!=0 && m_dAccRatio[j]!=0)
            {
                AccNum+=(m_eAccRatio[i]*m_dAccRatio[j]*m_rule[i][j]);
                AccDen+=(m_eAccRatio[i]*m_dAccRatio[j]);
            }
        }
    }

    double AccOutput=AccNum/AccDen;

    return AccOutput;
}


/* ***************************
 * 角速度模糊控制函数实现，因为滤波器部分设计为单个数据滤波，因此模糊控制部分也设计为加速度和角速度模糊控制分开
 * 输入：WErr加速度目标值  WDif加速度实际值
 * **************************/
double FuzzyControl::realizeW(double WErr,double WDif)
{
    m_WErr=WErr;  //角速度偏差
    m_WDif=WDif; //输入角速度微分


    if(m_WErr<-1*THRE1) m_WErr=-1*THRE1;
    if(m_WErr>THRE1) m_WErr=THRE1;
    if(m_WDif<-1*THRE2) m_WDif=-1*THRE2;
    if(m_WDif>THRE2) m_WDif=THRE2;

    //计算隶属度
    for(int i=0;i<NUM;i++)
    {
        m_eWRatio[i]=trimf(m_WErr,m_WEFunc[i][0],m_WEFunc[i][1],m_WEFunc[i][2]);  //角速度偏差隶属度
        m_dWRatio[i]=trimf(m_WDif,m_AccWDFunc[i][0],m_AccWDFunc[i][1],m_AccWDFunc[i][2]); //角速度微分隶属度
    }

    //模糊输出
    double WNum=0;
    double WDen=0;
    for(int i=0;i<NUM;i++)
    {
        for(int j=0;j<NUM;j++)
        {
            if(m_eWRatio[i]!=0 && m_dWRatio[j]!=0)
            {
                WNum+=(m_eWRatio[i]*m_dWRatio[j]*m_rule[i][j]);
                WDen+=(m_eWRatio[i]*m_dWRatio[j]);
            }
        }
    }

    double WOutput=WNum/WDen;

    return WOutput;
}




