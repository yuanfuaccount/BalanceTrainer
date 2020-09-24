#include "gaitsymmetry.h"

GaitSymmetry::GaitSymmetry()
{
    m_gaitSymData.resize(6);
    for(int i=0;i<6;i++)
        m_gaitSymData[i]=0;
    winSize=11;//窗口大小
}

/* *******************************
 * 求自相关系数的函数
 * 输入参数：allData为传感器采集的原始数据，ith为第几维,autoCorrArray用于存储自相关系数的结果
 * *******************************/
void GaitSymmetry::autoCorrelation(QVector<QVector<double>>* allData,int ith,QVector<double>& autoCorrArray)
{
    int num=(*allData).size();
    int start=num/4;
    int end=num*3/4;
    double autoCorr0=0;
    for(int i=start;i<=end;i++)
    {
        if(ith==1)
            autoCorr0+=((*allData)[i][ith]-1)*((*allData)[i][ith]-1);
        else
            autoCorr0+=(*allData)[i][ith]*(*allData)[i][ith];
    }
    autoCorr0/=(end-start+1);
    autoCorrArray.push_back(1);
    double autoCorr1=0;
    for(int k=1;k<end-start;k++)
    {
        autoCorr1=0;
        for(int i=start;i+k<end;i++)
        {
            if(ith==1)
                autoCorr1+=((*allData)[i][ith]-1)*((*allData)[i+k][ith]-1);  //y方向是重力方向，减去初始重力加速度
            else
                autoCorr1+=(*allData)[i][ith]*(*allData)[i+k][ith];
        }
        autoCorr1/=(end-k-start);
        autoCorr1/=autoCorr0;
        autoCorrArray.push_back(autoCorr1);
    }
}

/* *****************************************
 * 腰部传感器数据处理。当传感器数据采集完毕时，开始腰部数据分析
 * 输入参数：串口原始采集数据allData，要写入数据的文件
* ********************************************/
void GaitSymmetry::gaitSymmetryDataProcessSlot(QVector<QVector<double>>* allData,QFile* outfile)
{
    //autoCorrelation(allData,0,m_autoCorrX);
    autoCorrelation(allData,1,m_autoCorrY);
    autoCorrelation(allData,2,m_autoCorrZ);
    //寻找ZRE,ZSY,YRE,YSY
    for(int i=winSize/2;i<m_autoCorrY.size()-winSize/2;i++)
    {
        if(m_gaitSymData[0]!=0 && m_gaitSymData[1]!=0 && m_gaitSymData[3]!=0 && m_gaitSymData[4]!=0)
            break;
        int j1=1;
        for(;j1<=winSize/2;j1++)
        {
            if(m_autoCorrZ[i]<=m_autoCorrZ[i-j1] || m_autoCorrZ[i]<=m_autoCorrZ[i+j1])
                break;
        }
        if(j1>winSize/2 && m_gaitSymData[0]==0) //找到峰值
            m_gaitSymData[0]=m_autoCorrY[i];
        else if(j1>winSize/2 && m_gaitSymData[0]!=0 && m_gaitSymData[1]==0)
            m_gaitSymData[1]=m_autoCorrY[i];

        int j2=1;
        for(;j2<=winSize/2;j2++)
        {
            if(m_autoCorrY[i]<=m_autoCorrY[i-j2] || m_autoCorrY[i]<=m_autoCorrY[i+j2])
                break;
        }
        if(j2>winSize/2 && m_gaitSymData[3]==0)
            m_gaitSymData[3]=m_autoCorrZ[i];
        else if(j2>winSize/2 && m_gaitSymData[3]!=0 &&m_gaitSymData[4]==0)
            m_gaitSymData[4]=m_autoCorrZ[i];
    }
    m_gaitSymData[2]=m_gaitSymData[0]/m_gaitSymData[1];
    m_gaitSymData[5]=m_gaitSymData[3]/m_gaitSymData[4];

    //处理并保存数据
    if(outfile->open(QIODevice::ReadWrite))
        outfile->write("AccX,AccY,AccZ,Wx,Wy,Wz,AngleX,AngleY,AngleZ,\n");
    for(int i=0;i<(*allData).size();i++)
    {
        QString data="";
        data=data+QString::number((*allData)[i][0],'f',4)+",";
        data=data+QString::number((*allData)[i][1],'f',4)+",";
        data=data+QString::number((*allData)[i][2],'f',4)+",";
        data=data+QString::number((*allData)[i][3],'f',4)+",";
        data=data+QString::number((*allData)[i][4],'f',4)+",";
        data=data+QString::number((*allData)[i][5],'f',4)+",";
        data=data+QString::number((*allData)[i][6],'f',4)+",";
        data=data+QString::number((*allData)[i][7],'f',4)+",";
        data=data+QString::number((*allData)[i][8],'f',4)+",\n";

        QByteArray ba=data.toLatin1();
        const char* data1=ba.data();
        outfile->write(data1);
    }
    outfile->close();

    emit gaitSymmetryDataProcessFinished();
}
