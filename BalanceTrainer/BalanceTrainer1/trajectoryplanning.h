#ifndef TRAJECTORYPLANNING_H
#define TRAJECTORYPLANNING_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QString>
#include <QTextStream>

class TrajectoryPlanning:public QObject
{
    Q_OBJECT
public:
    TrajectoryPlanning(QObject* parent=nullptr);
    void loadTrajectoryFile(QString fileName);

public slots:
    //因为要将路径数据传输给motioncontrol线程，所以在此增加一个槽函数
    //点击主界面开始运行，触发此槽函数，此槽函数调用startTrajectroyPlanningSignal将m_trajectoryPath传给motioncontrol线程
    void startTrajectroyPlanningSlot();

signals:
    void startTrajectroyPlanningSignal(QVector<QVector<double>>* path);

private:
    QVector<QVector<double>> m_trajectoryPath;
};

#endif // TRAJECTORYPLANNING_H
