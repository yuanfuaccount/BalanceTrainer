#ifndef PAINTCHART_H
#define PAINTCHART_H

#include <QChart>
#include <QChartView>
#include <QColor>
#include <QSplineSeries>
#include <QValueAxis>
#include <QHBoxLayout>
#include <QGraphicsLayout>
#include <QPieSeries>
#include <QTableWidget>
#include "imusensor.h"

QT_CHARTS_USE_NAMESPACE

class ChartWidget
{
public:
    ChartWidget();
    ~ChartWidget();


    void loadDataFromCSV(QString filename,bool loadAccX=false,bool loadAccY=false,bool loadAccZ=true,bool loadWX=false,bool loadWY=false,bool loadWZ=true,bool loadAngleX=false,bool loadAngleY=false,bool loadAngleZ=true);
    void addChartData(const QList<qreal>& pointy,const QString dataNme,double XInterval);
    void chartPaint(QString xlabel);
    void chartClear(); //清除图像

public:
    QChart* m_chart;

private:
    QList<QSplineSeries*> m_series;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
    QHBoxLayout* m_layout;
    qreal m_xRange;
    qreal m_YMin;
    qreal m_YMax;
    qreal m_xInterval;

    int dataIndex;
};

//将步态
class GaitPhaseExhibition
{
public:
    GaitPhaseExhibition();
    ~GaitPhaseExhibition();
    void paintPieChart(QVector<double> data,bool leftfoot);
    void fillTableAndChart(FootSensor* sensor,bool leftFoot,QTableWidget* phaseTable,QTableWidget* statisticsTable,QChartView* pie);
    QChart* m_leftchart;
    QChart* m_rightchart;
private:

    QPieSeries* m_leftseries;
    QPieSeries* m_rightseries;
};




//自相关函数绘图
class AutoCorrChart
{
public:
    AutoCorrChart();
    ~AutoCorrChart();
    void paitAutoCorrChart(QVector<double> data);
    QChart* m_chart;
private:
    QSplineSeries* m_seri;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
    QHBoxLayout* m_layout;
};



#endif // PAINTCHART_H
