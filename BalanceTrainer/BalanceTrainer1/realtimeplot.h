#ifndef REALTIMEPLOT_H
#define REALTIMEPLOT_H

#include <QtCharts>


#include "config.h"

QT_CHARTS_USE_NAMESPACE


class RealTimePlot:public QGraphicsView
{
    Q_OBJECT
public:
    RealTimePlot(QWidget* parent=nullptr);
    ~RealTimePlot();

    static const int sampleCount=2000; //总共采样点个数
    static const int resolution=1; //每五个点更新一次图像

public slots:
    void timeoutslot();

private:
    QList<QPointF> m_buffer;
    float m_time;

    QTimer m_timer;

    QChart *m_chart;
    QSplineSeries *m_series ;
    QSplineSeries *m_setSeries;

    QChartView *m_chartView;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
};

#endif // REALTIMEPLOT_H
