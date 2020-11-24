#include "realtimeplot.h"

RealTimePlot::RealTimePlot(QWidget* parent):
    QGraphicsView(parent)
{
    m_chart=new QChart();
    m_series=new QSplineSeries();

    //训练曲线
    m_setSeries=new QSplineSeries();
    m_setSeries->append(0,0);
    for(int i=0;i<10;i++)
    {
        m_setSeries->append(0,0);
        m_setSeries->append(5,5);
        m_setSeries->append(8,3);
        m_setSeries->append(10,0);
        m_setSeries->append(7,-7);
        m_setSeries->append(0,-10);
        m_setSeries->append(-7,-7);
        m_setSeries->append(-10,0);
        m_setSeries->append(-5,5);
        m_setSeries->append(0,0);

    }

    m_chartView = new QChartView(m_chart);
    m_chartView->setMinimumSize(800, 600);
    m_chart->addSeries(m_series);
    m_axisX = new QValueAxis;
    m_axisX->setRange(-18,18);
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Samples");
    m_axisY = new QValueAxis;
    m_axisY->setRange(-18, 18);
    m_axisY->setTitleText("Audio level");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    m_chart->legend()->hide();
    m_chart->setTitle("平衡板训练");

    //添加训练曲线
    m_chart->addSeries(m_setSeries);
//    m_setSeries->attachAxis(m_axisX);
//    m_setSeries->attachAxis(m_axisY);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chartView);

    connect(&m_timer,&QTimer::timeout,this,&RealTimePlot::timeoutslot);

    if (m_buffer.isEmpty()) {
        m_buffer.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
        {
            m_buffer.append(QPointF(0, 0));
        }
    }
    m_timer.start(100);
}


RealTimePlot::~RealTimePlot()
{
    delete m_chart;
    delete  m_series;
    delete m_setSeries;
    delete m_chartView;
    delete m_axisX;
    delete m_axisY;
}


void RealTimePlot::timeoutslot()
{
    static int count=0;
    m_buffer.push_back(QPointF(g_roll,g_pitch));  //显示平台的倾斜角度
    m_buffer.pop_front();

    count++;
    if(count>=resolution)
    {
        count=0;
        m_series->replace(m_buffer);
    }
    //update();
}


