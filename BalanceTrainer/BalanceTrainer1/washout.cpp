#include "washout.h"

WashOut::WashOut(QObject* parent)
{
    if(parent!=nullptr)
        this->setParent(parent);

    for(int i=0;i<3;i++)
    {
        m_lowPassWFilter[i].setFilterType(secondLowPassAcc); //此滤波器用来对角速度微分进行过滤，参数与加速度低通一致
        m_firstHighPassAccFilter[i].setFilterType(firstHighPassAcc);
        m_firstIntegral[i].setFilterType(firstIntegral);
        m_secondIntegral[i].setFilterType(secondIntegral);
        for(int j=0;j<2;j++)
        {
            m_otolishFilter[i][j].setFilterType(otolish);
            m_highPassAccFilter[i][j].setFilterType(secondHighPassAcc);
            m_lowPassAccFilter[i][j].setFilterType(secondLowPassAcc);
            m_semicirulareFilter[i][j].setFilterType(semicirulare);
            m_highPassWFilter[i][j].setFilterType(secondHighPassAng);
        }
    }
}

const double WashOut::INTERVAL=0.1;  //时间间隔0.1s



/* *****************************
 * 洗出算法,输入角速度为角度制
 * 输出为位置增量:deltaX,deltaY,deltaZ,deltaRoll,deltaPitch,deltaYaw,角度均为角度制
 * *****************************/
QVector<double> WashOut::getWashOut(QVector<double> input)
{
    QVector<double> out;
    input[3]=input[3]*PI/180;input[4]=input[4]*PI/180;input[5]=input[5]*PI/180;

    Eigen::Matrix3d Ls=RotationMatrix(g_yaw,g_pitch,g_roll);
    Eigen::Matrix3d Ts;
    Ts<<1,sin(g_roll)*tan(g_pitch),cos(g_roll)*tan(g_pitch),0,cos(g_roll),-sin(g_roll),0,sin(g_roll)/cos(g_pitch),cos(g_roll)/cos(g_pitch);

    Eigen::Vector3d Acc(input[0],input[1],input[2]);
    Eigen::Vector3d W(input[3],input[4],input[5]);

    Acc=Ls*Acc; //变换到世界坐标系
    W=Ts*W; //坐标变换到世界坐标系


    /***************************
     * 加速度洗出算法部分
     * *************************/
    for(int i=0;i<3;i++)
    {
        m_desiredOutput[i]=m_otolishFilter[i][0].filter(Acc[i]); //耳石滤波器1，用于目标输入滤波
        m_highPassOutput[i]=m_highPassAccFilter[i][0].filter(Acc[i]); //高通加速度滤波器1，用于经典输出滤波
        m_highPassOutput[i]=m_firstHighPassAccFilter[i].filter(m_highPassOutput[i]);

        double AccDif=(Acc[i]-m_lastInput[i])/INTERVAL;
        AccDif=m_lowPassAccFilter[i][0].filter(AccDif);  //低通加速度滤波器1，用于加速度微分滤波
        m_fuzzyOutput[i]=m_fuzzy.realizeAcc(m_desiredOutput[i]-m_senseOutput[i],AccDif);
        double FuzzyOut1=m_highPassAccFilter[i][1].filter(m_fuzzyOutput[i]); //高通加速度滤波器2，用于模糊输出滤波
        m_highPassOutput[i]+=FuzzyOut1;  //经过模糊输出高通滤波的部分直接与经典滤波高通部分相加，此部分二次积分即为末端位姿
        //模糊输出另一部分，加到低通滤波器的输出上
        double FuzzyOut2=(m_fuzzyOutput[i]-FuzzyOut1)/9.8;
        m_lowPassAccOutput[i]=m_lowPassAccFilter[i][1].filter(Acc[i]); //低通加速度滤波器2，用于加速度低通滤波
        m_lowPassAccOutput[i]=asin(m_lowPassAccOutput[i]/9.8); //倾斜协调通道
        m_lowPassAccOutput[i]+=FuzzyOut2; //跟模糊补偿第二部分相加

        //倾斜协调速度限幅,在Z方向没有倾斜协调
        if(i<2)
        {
            if((m_lowPassAccOutput[i]-m_lastLowPassAccOutput[i])/INTERVAL>LIMITRATE[i])
                m_lowPassAccOutput[i]=m_lastLowPassAccOutput[i]+LIMITRATE[i]*INTERVAL;
            else if((m_lowPassAccOutput[i]-m_lastLowPassAccOutput[i])/INTERVAL<-1*LIMITRATE[i])
                m_lowPassAccOutput[i]=m_lastLowPassAccOutput[i]-LIMITRATE[i]*INTERVAL; //倾斜协调部分输出，将其加入到最终角度
        }

    }

    /* ****************************
     * 角速度洗出算法部分
     * *****************************/
    for(int i=0;i<3;i++)
    {
        m_desiredOutput[i+3]=m_semicirulareFilter[i][0].filter(W[i]);  //半规管滤波器1，用于理论输出
        m_highPassOutput[i+3]=m_highPassWFilter[i][0].filter(W[i]); //高通角速度滤波器1，用于高通角速度滤波
        double WDif=(W[i]-m_lastInput[i+3])/INTERVAL;
        WDif=m_lowPassWFilter[i].filter(WDif); //低通角速度滤波，用于对角速度微分滤波
        m_fuzzyOutput[i+3]=m_fuzzy.realizeW(m_desiredOutput[i+3]-m_senseOutput[i+3],WDif);
        m_fuzzyOutput[i+3]=m_highPassWFilter[i][1].filter(m_fuzzyOutput[i+3]); //高通角速度滤波器2，用于角速度模糊输出滤波
    }

    /* ***************************
     * 将倾斜协调通道和角速度通道整合,此部分经过最终半规管滤波就能得到最终输出角速度
     * ***************************/
    //wx与accy的倾斜协调通道及x的模糊角速度输出相加，此部分一次积分得到绕X的角位移，即ROLL
    m_highPassOutput[3]=m_highPassOutput[3]+(m_lowPassAccOutput[1]-m_lastLowPassAccOutput[1])/INTERVAL+m_fuzzyOutput[3];
    //wy与accx倾斜协调通道及y的模糊角速度输出相加此部分一次积分得到绕Y的角位移，即PITCH
    m_highPassOutput[4]=m_highPassOutput[1]+(m_lowPassAccOutput[0]-m_lastLowPassAccOutput[0])/INTERVAL+m_fuzzyOutput[4];
    //wz与z的模糊角速度输出相加，此部分一次积分得到Z的角位移，即YAW
    m_highPassOutput[5]=m_highPassOutput[5]+m_fuzzyOutput[5];


    /* ***************************
     * 最终输出部分
     * ***************************/
    for(int i=0;i<3;i++)
    {
        m_senseOutput[i]=m_highPassOutput[i]+9.8*sin(m_lowPassAccOutput[i]); //高通加速度+低通加速度得到和加速度
        m_senseOutput[i]=m_otolishFilter[i][1].filter(m_senseOutput[i]); //耳石滤波器2，最终加速度输出

        m_posOutput[i]=m_secondIntegral[i].filter(m_highPassOutput[i]);
    }
    for(int i=3;i<6;i++)
    {
        m_senseOutput[i]=m_semicirulareFilter[i][1].filter(m_highPassOutput[i]); //半规管滤波器2，最终加速度输出
        m_posOutput[i]=m_firstIntegral[i-3].filter(m_highPassOutput[i]);
    }

    //输出数据
    for(int i=0;i<6;i++)
    {
        out.push_back(m_posOutput[i]-m_lastPosOutput[i]);
    }

    //记录过去的值
    for(int i=0;i<6;i++)
    {
        out.push_back(m_senseOutput[i]);
        if(i<3) m_lastInput[i]=Acc[i];
        else m_lastInput[i]=W[i-3];

        m_lastPosOutput[i]=m_posOutput[i];

        if(i<3)
            m_lastLowPassAccOutput[i]=m_lowPassAccOutput[i];

    }


    return out;
}

void WashOut::reset()
{
    for(int i=0;i<6;i++)
    {
        m_senseOutput[i]=0;
        m_desiredOutput[i]=0;
        m_fuzzyOutput[i]=0; //模糊控制器输出
        m_lastInput[i]=0; //上一次的输入

        m_posOutput[i]=0; //位移输出
        m_lastPosOutput[i]=0; //上一次位移输出
        m_highPassOutput[i]=0;//高通滤波输出

        if(i<3)
        {
            m_lowPassAccOutput[i]=0; //低通加速度滤波输出
            m_lastLowPassAccOutput[i]=0; //上一次低通加速度滤波通道输出，即倾斜协调输出
        }
    }

    //滤波器的历史记录也需要重置
    for(int i=0;i<3;i++)
    {
        m_lowPassWFilter[i].reset();
        for(int j=0;j<2;j++)
        {
            m_otolishFilter[i][j].reset();
            m_highPassAccFilter[i][j].reset();
            m_lowPassAccFilter[i][j].reset();
            m_semicirulareFilter[i][j].reset();
            m_highPassWFilter[i][j].reset();
        }
    }
}

//计算当前实际需要输入的加速度和角速度
QVector<double> WashOut::calAccW(double AccX,double AccY,double AccZ,double WX,double WY,double WZ,double AccTime,double WTime,double AccSlopeTime,double WSlopeTime,double runtime)
{
    QVector<double> res(6,0);
    if(runtime<AccSlopeTime)
    {
        res[0]=runtime*AccX/AccSlopeTime;
        res[1]=runtime*AccY/AccSlopeTime;
        res[2]=runtime*AccZ/AccSlopeTime;
    }
    else if(runtime>=AccSlopeTime && runtime<AccSlopeTime+AccTime)
    {
        res[0]=AccX;res[1]=AccY;res[2]=AccZ;
    }
    else
    {
        res[0]=0;res[1]=0;res[2]=0;
    }
    if(runtime<WSlopeTime)
    {
        res[3]=runtime*WX/WSlopeTime;
        res[4]=runtime*WY/WSlopeTime;
        res[5]=runtime*WZ/WSlopeTime;
    }
    else if(runtime>=WSlopeTime && runtime<WSlopeTime+WTime)
    {
        res[3]=WX;res[4]=WY;res[5]=WZ;
    }
    else
    {
        res[3]=0;res[4]=0;res[5]=0;
    }
    return res;
}
