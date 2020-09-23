#ifndef GAITSYMMETRY_H
#define GAITSYMMETRY_H
#include <QObject>
#include <QVector>
#include <deque>
#include <QFile>

using namespace std;

class GaitSymmetry:public QObject
{
    Q_OBJECT
public:
    GaitSymmetry();
    void autoCorrelation(QVector<QVector<double>>* allData,int ith,QVector<double>& autoCorrArray);

    QVector<double> m_autoCorrX; //存储自相关系数
    QVector<double> m_autoCorrY; //存储自相关系数
    QVector<double> m_autoCorrZ; //存储自相关系数

public slots:
    void gaitSymmetryDataProcessSlot(QVector<QVector<double>>* allData,QFile* outfile);

signals:
    void gaitSymmetryDataProcessFinished();
};




#endif // GAITSYMMETRY_H
