#include "filter.h"

Filter::Filter()
{

    m_bz.resize(8);
    m_az.resize(8);

    m_bz[secondHighPassAcc]={0.7901,-1.5802,0.7901};
    m_az[secondHighPassAcc]={1, -1.5556,0.6049};

    m_bz[secondLowPassAcc]={0.04,0.08,0.04};
    m_az[secondLowPassAcc]={1,-1.2,0.36};

    m_bz[secondHighPassAng]={0.9070,-1.8141,0.9070};
    m_az[secondHighPassAng]={1.0000,-1.8095,0.8186};

    m_bz[otolish]={0.0693,0.0005,-0.0688};
    m_az[otolish]={1.0000,-1.8407,0.8433};

    m_bz[firstHighPassAcc]={0.9524,-0.9524,0};
    m_az[firstHighPassAcc]={1.0000,-0.9048,0};

    m_bz[firstIntegral]={0.0500,0.0500,0};
    m_az[firstIntegral]={1,-1,0};

    m_bz[secondIntegral]={0.0025,0.0050,0.0025};
    m_az[secondIntegral]={1,-2,1};

    m_bz_semicirulare={0.3297,-0.3297,-0.3297,0.3297};
    m_az_semicirulare={1.0000,-2.3113,1.6374,-0.3260};


    m_input.resize(4);
    m_output.resize(4);

    for(int i=0;i<4;i++)
    {
        m_input[i]=0;
        m_output[i]=0;
    }
}

void Filter::setFilterType(int type)
{
    m_filterType=type;
}

/* ***********************
 * 滤波函数
 * filterType： 0：高通加速度滤波器 1：低通加速度滤波器 2：高通角速度滤波器 3：耳石模型 4：半规管模型
 * ***********************/
double Filter::filter(double input)
{
    m_input[0]=m_input[1];
    m_input[1]=m_input[2];
    m_input[2]=m_input[3];
    m_input[3]=input;

    m_output[0]=m_output[1];
    m_output[1]=m_output[2];
    m_output[2]=m_output[3];

    if(m_filterType==semicirulare)
    {
        m_output[3]=m_bz_semicirulare[0]*m_input[3]+m_bz_semicirulare[1]*m_input[2]+m_bz_semicirulare[2]*m_input[1]+m_bz_semicirulare[3]*m_input[0]
                -m_az_semicirulare[1]*m_output[2]-m_az_semicirulare[2]*m_output[1]-m_az_semicirulare[3]*m_output[0];
    }
    else
    {
        m_output[3]=m_bz[m_filterType][0]*m_input[3]+m_bz[m_filterType][1]*m_input[2]+m_bz[m_filterType][2]*m_input[1]
                -m_az[m_filterType][1]*m_output[2]-m_az[m_filterType][2]*m_output[1];
    }
    return m_output[3];
}

void Filter::reset()
{
    for(int i=0;i<4;i++)
    {
        m_input[i]=0;
        m_output[i]=0;
    }
}


