#include "realtimeplot.h"

RealTimePlot::RealTimePlot(QWidget* parent):
    QGraphicsView(parent)
{
    m_chart=new QChart();
    m_series=new QSplineSeries();

    QChartView *chartView = new QChartView(m_chart);
    chartView->setMinimumSize(800, 600);
    m_chart->addSeries(m_series);
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(-10,10);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Samples");
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-10, 10);
    axisY->setTitleText("Audio level");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);
    m_chart->legend()->hide();
    m_chart->setTitle("Data from the microphone");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartView);

    connect(&m_timer,&QTimer::timeout,this,&RealTimePlot::timeoutslot);

    if (m_buffer.isEmpty()) {
        m_buffer.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
        {
            int x=rand()%10;
            m_buffer.append(QPointF(x, 0));
        }
    }
    m_timer.start(500);
}


void RealTimePlot::timeoutslot()
{
    int start=1990;

        for (int s = 0; s < start; ++s)
        {
            m_buffer[s].setX(m_buffer.at(s + 10).x());
            m_buffer[s].setY(m_buffer.at(s + 10).y());
        }

    for (int s = 1990; s < sampleCount;s++)
    {
       float x=10*cos(0.1*m_time);
       float y=10*sin(0.1*m_time);
        m_buffer[s].setX(x);
        m_buffer[s].setY(y);
        m_time++;

    }

    m_series->replace(m_buffer);
    //update();
}
