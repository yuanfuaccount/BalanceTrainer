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

void ChartWidget::addChartData(const QList<qreal>& pointy,const QString dataNme,double XInterval=0.02)
{
    int size=pointy.size();
    m_xRange=XInterval*size+1;  //X轴的范围

    QSplineSeries* seri=new QSplineSeries;
    qreal minval=2000;
    qreal maxval=-2000;
    for(int i=0;i<size;i++)
    {
        if(minval>pointy[i]) minval=pointy[i];
        if(maxval<pointy[i]) maxval=pointy[i];
    }
    m_YMax=maxval;
    m_YMin=minval;

    //数据归一化处理
    for(int i=0;i<size;i++)
        seri->append((i+1)*m_xInterval,pointy[i]);
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


void ChartWidget::chartPaint(QString xlabel)
{
    m_axisX->setRange(0,m_xRange);
    //m_axisX->setTickInterval(0.2);
    //m_axisX->setLabelFormat("%2f");
    m_axisX->setGridLineVisible(false); //网格线可见
    m_axisX->setTitleText(xlabel);

    m_axisY->setRange(m_YMin,m_YMax);
    //m_axisY->setLabelFormat("%1f");
    m_axisY->setGridLineVisible(true); //网格线可见
   // m_axisY->setTickInterval(0.2);


    m_chart->addAxis(m_axisX,Qt::AlignBottom);
    m_chart->addAxis(m_axisY,Qt::AlignLeft);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    for(auto iter:m_series)
    {
        m_chart->addSeries(iter);
    }
}

/* **********************************
 * 从CSV表格加载数据，针对原始数据进行图像显示
 * ********************************/
void ChartWidget::loadDataFromCSV(QString filename,bool loadAccX,bool loadAccY,bool loadAccZ,bool loadWX,bool loadWY,bool loadWZ,bool loadAngleX,bool loadAngleY,bool loadAngleZ)
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
        file->close();
    }
}


void ChartWidget::chartClear()
{
    m_chart->removeAllSeries();
}


GaitPhaseExhibition::GaitPhaseExhibition()
{
    m_leftchart=new QChart();

    m_rightchart=new QChart();

}

GaitPhaseExhibition::~GaitPhaseExhibition()
{
    delete  m_leftchart;
    delete m_leftseries;
    delete  m_rightchart;
    delete m_rightseries;
}

void GaitPhaseExhibition::paintPieChart(QVector<double> data,bool leftfoot) //传入的数据顺序分别为脚跟离地期占比，摆动相占比，脚跟着地期占比，完全站立相占比
{
    double ratio3=data[0]*100/data[4];
    double ratio4=data[1]*100/data[4];
    double ratio1=data[2]*100/data[4];
    double ratio2=data[3]*100/data[4];
    //四个时期在饼状图中序号分别为3，4，1，2
    if(leftfoot)
    {
        m_leftseries=new QPieSeries();
        m_leftseries->append("3:"+QString::number(ratio3,'f',1)+'%',data[0])->setColor(colorDatalst[0]);
        m_leftseries->append("4:"+QString::number(ratio4,'f',1)+'%',data[1])->setColor(colorDatalst[1]);
        m_leftseries->append("1:"+QString::number(ratio1,'f',1)+'%',data[2])->setColor(colorDatalst[2]);
        m_leftseries->append("2:"+QString::number(ratio2,'f',1)+'%',data[3])->setColor(colorDatalst[3]);
        //m_series->setLabelsVisible();

        m_leftchart->addSeries(m_leftseries);
        m_leftchart->legend()->hide();
    }
    else
    {
        m_rightseries=new QPieSeries();
        m_rightseries->append("3:"+QString::number(ratio3,'f',1)+'%',data[0])->setColor(colorDatalst[0]);
        m_rightseries->append("4:"+QString::number(ratio4,'f',1)+'%',data[1])->setColor(colorDatalst[1]);
        m_rightseries->append("1:"+QString::number(ratio1,'f',1)+'%',data[2])->setColor(colorDatalst[2]);
        m_rightseries->append("2:"+QString::number(ratio2,'f',1)+'%',data[3])->setColor(colorDatalst[3]);
        //m_series->setLabelsVisible();

        m_rightchart->addSeries(m_rightseries);
        m_rightchart->legend()->hide();
    }


}

void GaitPhaseExhibition::fillTableAndChart(FootSensor* sensor,bool leftFoot,QTableWidget* phaseTable,QTableWidget* statisticsTable,QChartView* pie)
{
    //清除上次显示的所有数据
    allDataclear(phaseTable,statisticsTable,pie);

    //填充原始数据表和统计数据表
    int rowCount=sensor->detector->gaitPhaseTime.size();
    phaseTable->setRowCount(rowCount);
    for(int i=0;i<rowCount;i++)
    {
        QTableWidgetItem* stepNum=new QTableWidgetItem();
        stepNum->setText(QString::number(i+1));
        QTableWidgetItem* loadingPhase=new QTableWidgetItem();
        loadingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][2]));
        QTableWidgetItem* stancePhase=new QTableWidgetItem();
        stancePhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][3]));
        QTableWidgetItem* leavingPhase=new QTableWidgetItem();
        leavingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][0]));
        QTableWidgetItem* swingPhase=new QTableWidgetItem();
        swingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][1]));
        QTableWidgetItem* cycle=new QTableWidgetItem();
        cycle->setText(QString::number(sensor->detector->gaitPhaseTime[i][4]));

        phaseTable->setItem(i,0,stepNum);
        phaseTable->setItem(i,1,loadingPhase);
        phaseTable->setItem(i,2,stancePhase);
        phaseTable->setItem(i,3,leavingPhase);
        phaseTable->setItem(i,4,swingPhase);
        phaseTable->setItem(i,5,cycle);
    }

    int rowNum=0;
    if(!leftFoot)
        rowNum=1;
    for(int i=0;i<7;i++)
    {
        QTableWidgetItem* item=new QTableWidgetItem();
        if(i<5)
            item->setText(QString::number(sensor->detector->avgGatiPhaseTime[i],'f',2));
        else if(i==5)
            item->setText(QString::number(sensor->detector->stepEffecitve));
        else if(i==6)
            item->setText(QString::number(sensor->detector->stepTotal));
        statisticsTable->setItem(rowNum,i,item);
    }

    //画饼状图
    paintPieChart(sensor->detector->avgGatiPhaseTime,leftFoot);
    if(leftFoot)
        pie->setChart(m_leftchart);

    else
        pie->setChart(m_rightchart);

}

void GaitPhaseExhibition::allDataclear(QTableWidget* phaseTable,QTableWidget* statisticsTable,QChartView* pie)
{
    phaseTable->clearContents();
    statisticsTable->clearContents();
    m_leftchart->removeAllSeries();  //会删除对应的series指针
    m_rightchart->removeAllSeries();
    pie->setChart(m_rightchart);
}



AutoCorrChart::AutoCorrChart()
{
    m_chart=new QChart();
    m_axisX=new QValueAxis;
    m_axisY=new QValueAxis;
    m_layout=new QHBoxLayout;
}

AutoCorrChart::~AutoCorrChart()
{
    delete m_chart;
    delete m_seri;
    delete m_axisX;
    delete m_axisY;
    delete m_layout;
}

void AutoCorrChart::paitAutoCorrChart(QVector<double> data)
{
    allDataClear();

    m_seri=new QSplineSeries();
    for(int i=0;i<data.size();i++)
        *m_seri<<QPointF(i,data[i]);
    QPen pen;
    pen.setWidthF(0.7);
    m_seri->setPen(pen);
    m_seri->setPointLabelsVisible(false);

    m_axisX->setRange(0,data.size()+1);
    m_axisX->setGridLineVisible(false); //网格线可见
    m_axisX->setTickType(QValueAxis::TickType::TicksDynamic);
    m_axisX->setTickInterval(10);
    m_axisX->setLabelFormat("%d");

    m_axisY->setRange(-1.2,1.2);
    m_axisY->setGridLineVisible(true); //网格线可见
    m_axisY->setTickCount(7);
    m_axisY->setLabelFormat("%.1f");


    m_chart->addSeries(m_seri);
    m_chart->setAxisX(m_axisX,m_seri);
    m_chart->setAxisY(m_axisY,m_seri);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);


    m_chart->setMargins(QMargins(0,0,0,0));
    m_chart->legend()->setVisible(false);
    m_chart->layout()->setContentsMargins(0,0,0,0);
}

void AutoCorrChart::allDataClear()
{
    m_chart->removeAllSeries();
}


