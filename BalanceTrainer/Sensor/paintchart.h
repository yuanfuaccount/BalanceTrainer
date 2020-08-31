#ifndef PAINTCHART_H
#define PAINTCHART_H

#include <QChart>
#include <QChartView>
#include <QColor>
#include <QSplineSeries>
#include <QValueAxis>
#include <QHBoxLayout>

QT_CHARTS_USE_NAMESPACE

class ChartWidget
{
public:
    ChartWidget();
    ~ChartWidget();


    void loadDataFromCSV(QString filename,bool loadAccX=false,bool loadAccY=false,bool loadAccZ=true,bool loadWX=false,bool loadWY=false,bool loadWZ=true,bool loadAngleX=false,bool loadAngleY=false,bool loadAngleZ=true,bool loadQ0=false,bool loadQ1=false,bool loadQ2=false,bool loadQ3=false);
    void addChartData(const QList<qreal>& pointy,const QString dataNme);
    void chartPaint();

public:
    QChart* m_chart;

private:
    QList<QSplineSeries*> m_series;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
    QHBoxLayout* m_layout;
    qreal m_xRange;
    qreal m_xInterval;

    int dataIndex;
};



#endif // PAINTCHART_H
