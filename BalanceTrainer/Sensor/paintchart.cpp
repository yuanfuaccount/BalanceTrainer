#include "paintchart.h"
#include <QPainter>
#include <QPainterPath>
#include <QTextStream>


const QList<QColor> colorDatalst={QColor(Qt::red), QColor(Qt::blue), QColor(Qt::green), QColor(Qt::yellow), QColor(Qt::black), QColor(Qt::cyan), QColor(Qt::gray)};

ChartWidget::ChartWidget():
    m_xInterval(0.02),
    dataIndex(0)
{
    m_chart=new QChart;
    m_axisX=new QValueAxis;
    m_axisY=new QValueAxis;
    m_layout=new QHBoxLayout;
}

ChartWidget::~ChartWidget()
{
    delete m_chart;
    delete m_axisX;
    delete m_axisY;
    delete m_layout;
    for(auto iter:m_series)
    {
        delete iter;
    }
}

void ChartWidget::addChartData(const QList<qreal>& pointy,const QString dataNme)
{
    int size=pointy.size();
    m_xRange=m_xInterval*size+1;  //X轴的范围

    QSplineSeries* seri=new QSplineSeries;
    qreal minval=0;
    qreal maxval=0;
    for(int i=0;i<size;i++)
    {
        if(minval>pointy[i]) minval=pointy[i];
        if(maxval<pointy[i]) maxval=pointy[i];
    }
    qreal height=qMax(qAbs(minval),qAbs(maxval));

    //数据归一化处理
    for(int i=0;i<size;i++)
        seri->append((i+1)*m_xInterval,pointy[i]/height);
    if(dataIndex==colorDatalst.size())
        dataIndex=0;
    QPen pen;
    pen.setWidthF(0.7);
    pen.setColor(colorDatalst[dataIndex]);
    seri->setName(dataNme);
    seri->setPen(pen);
    m_series<<seri;
    dataIndex++;
}


void ChartWidget::chartPaint()
{
    m_axisX->setRange(0,m_xRange);
    m_axisX->setTickInterval(0.2);
    //m_axisX->setLabelFormat("%2f");
    m_axisX->setGridLineVisible(false); //网格线可见
    m_axisX->setTitleText("Time/s");

    m_axisY->setRange(-1.2,1.2);
    //m_axisY->setLabelFormat("%1f");
    m_axisY->setGridLineVisible(true); //网格线可见
    m_axisY->setTickInterval(0.2);


    m_chart->addAxis(m_axisX,Qt::AlignBottom);
    m_chart->addAxis(m_axisY,Qt::AlignLeft);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    for(auto iter:m_series)
    {
        m_chart->addSeries(iter);
    }
}


void ChartWidget::loadDataFromCSV(QString filename,bool loadAccX,bool loadAccY,bool loadAccZ,bool loadWX,bool loadWY,bool loadWZ,bool loadAngleX,bool loadAngleY,bool loadAngleZ,bool loadQ0,bool loadQ1,bool loadQ2,bool loadQ3)
{
    QFile* file=new QFile(filename);
    QList<qreal> accx,accy,accz,wx,wy,wz,anglex,angley,anglez,q0,q1,q2,q3;
    if(file->open(QIODevice::ReadOnly))
    {
        QTextStream* stream=new QTextStream(file);
        QStringList datalist=stream->readAll().split('\n');
        for(int i=datalist.count()/4;i<datalist.count()*3/4;i++)
        {
            QStringList row=datalist.at(i).split(',');
            accx<<row[0].toDouble();
            accy<<row[1].toDouble();
            accz<<row[2].toDouble();
            wx<<row[3].toDouble();
            wy<<row[4].toDouble();
            wz<<row[5].toDouble();
            anglex<<row[6].toDouble();
            angley<<row[7].toDouble();
            anglez<<row[8].toDouble();
            q0<<row[9].toDouble();
            q1<<row[10].toDouble();
            q2<<row[11].toDouble();
            q3<<row[12].toDouble();
        }
        if(loadAccX)
            addChartData(accx,"AccX");
        if(loadAccY)
            addChartData(accy,"AccY");
        if(loadAccZ)
            addChartData(accz,"AccZ");
        if(loadWX)
            addChartData(wx,"WX");
        if(loadWY)
            addChartData(wy,"WY");
        if(loadWZ)
            addChartData(wz,"WZ");
        if(loadAngleX)
            addChartData(anglex,"AngleX");
        if(loadAngleY)
            addChartData(angley,"AngleY");
        if(loadAngleZ)
            addChartData(anglez,"AngleZ");
        if(loadQ0)
            addChartData(q0,"Q0");
        if(loadQ1)
            addChartData(q1,"Q1");
        if(loadQ2)
            addChartData(q2,"Q2");
        if(loadQ3)
            addChartData(q3,"Q3");
        file->close();
    }
}




