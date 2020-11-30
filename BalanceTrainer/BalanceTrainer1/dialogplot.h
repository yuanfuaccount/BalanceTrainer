#ifndef DIALOGPLOT_H
#define DIALOGPLOT_H

#include <QDialog>
#include <QtCharts>

#include "config.h"



QT_CHARTS_USE_NAMESPACE

namespace Ui {
class DialogPlot;
}

class DialogPlot : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlot(QWidget *parent = nullptr);
    ~DialogPlot();

    static const int sampleCount=2000; //总共采样点个数
    static const int resolution=1; //每多少个点更新一次数据

public slots:
    void timeoutslot();

private:
    Ui::DialogPlot *ui;

    //画图需要的数据
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

#endif // DIALOGPLOT_H
