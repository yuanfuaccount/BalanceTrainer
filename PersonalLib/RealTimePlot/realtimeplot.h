#ifndef REALTIMEPLOT_H
#define REALTIMEPLOT_H

#include <QtCharts>

QT_CHARTS_USE_NAMESPACE


class RealTimePlot:public QGraphicsView
{
    Q_OBJECT
public:
    RealTimePlot(QWidget* parent=nullptr);

    static const int sampleCount=2000;

public slots:
    void timeoutslot();

private:
    QVector<QPointF> m_buffer;
    float m_time;

    QTimer m_timer;

    QChart *m_chart;
    QSplineSeries *m_series ;
};

#endif // REALTIMEPLOT_H
