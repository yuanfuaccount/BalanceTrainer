#include "dialogplot.h"
#include "ui_dialogplot.h"

DialogPlot::DialogPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlot)
{
    ui->setupUi(this);

    m_chart=new QChart();
    m_series=new QSplineSeries();

    m_chartView = new QChartView(m_chart);
    m_chartView->setMinimumSize(600, 600);
    m_chart->addSeries(m_series);
    m_axisX = new QValueAxis;
    m_axisX->setRange(-6,6);
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Left-Right");
    m_axisY = new QValueAxis;
    m_axisY->setRange(-6, 6);
    m_axisY->setTitleText("Front-Back");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    m_chart->legend()->hide();
    m_chart->setTitle("平衡板训练");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chartView);

    connect(&m_timer,&QTimer::timeout,this,&DialogPlot::timeoutslot);

    /*
    if (m_buffer.isEmpty()) {
        m_buffer.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
        {
            m_buffer.append(QPointF(0, 0));
        }
    }
    */

    m_timer.start(100);
}

DialogPlot::~DialogPlot()
{
    delete m_chart;
    delete m_chartView;
    delete ui;
}


void DialogPlot::timeoutslot()
{
    static int count=0;
    m_buffer.push_back(QPointF(g_roll,g_pitch));  //显示平台的倾斜角度
    if(m_buffer.size()>sampleCount)
        m_buffer.pop_front();

    count++;
    if(count>=resolution)
    {
        count=0;
        m_series->replace(m_buffer);
    }
    //update();
}

