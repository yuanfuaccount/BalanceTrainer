#include "gaitsymmetry.h"

GaitSymmetry::GaitSymmetry()
{
}

/* *******************************
 * 求自相关系数的函数
 * 输入参数：allData为传感器采集的原始数据，ith为第几维,autoCorrArray用于存储自相关系数的结果
 * *******************************/
void GaitSymmetry::autoCorrelation(QVector<QVector<double>>* allData,int ith,QVector<double>& autoCorrArray)
{
    int num=(*allData).size();
    int mid=num/2;
    double autoCorr0=0;
    for(int i=mid;i<num;i++)
        autoCorr0+=(*allData)[i][ith]*(*allData)[i][ith];
    autoCorr0/=(num-mid);
    autoCorrArray.push_back(1);
    double autoCorr1=0;
    for(int k=1;k<num-mid;k++)
    {
        autoCorr1=0;
        for(int i=mid;i+k<num;i++)
            autoCorr1+=(*allData)[i][ith]*(*allData)[i+k][ith];
        autoCorr1/=(num-k-mid);
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
    autoCorrelation(allData,0,m_autoCorrX);
    autoCorrelation(allData,1,m_autoCorrY);
    autoCorrelation(allData,2,m_autoCorrZ);
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
