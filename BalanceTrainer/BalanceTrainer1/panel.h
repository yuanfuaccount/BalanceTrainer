#ifndef UDPDIALOG_H
#define UDPDIALOG_H

#include <QDialog>
#include <vector>

namespace Ui {
class UDPDialog;
class SpeedMode;
class PositionMode;
class TrajectoryPlan;
}

class UDPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UDPDialog(QWidget *parent = nullptr);
    ~UDPDialog();

public slots:
    void getUDDPData();

private:
    Ui::UDPDialog *ui;

};



class SpeedMode : public QDialog
{
    Q_OBJECT

public:
    explicit SpeedMode(QWidget *parent = nullptr);
    ~SpeedMode();


private:
    QTimer* tim;
    Ui::SpeedMode *SM;
    QVector<double> PlatformSpeed;
    QVector<uint32_t> BasePos;
    double time;
};

class PositionMode:public QDialog
{
    Q_OBJECT
public:
    explicit PositionMode(QWidget *parent=nullptr);
    ~PositionMode();

private:
    Ui::PositionMode *PM;
    QVector<double> position;
};

class TrajectoryPlan:public QDialog
{
    Q_OBJECT
public:
    explicit TrajectoryPlan(QWidget* parent=nullptr);
    ~TrajectoryPlan();

private:
    Ui::TrajectoryPlan* TP;
    QTimer* tim;
    QVector<QVector<double>> Path;
    int pathnum;
    int datanum;
};

#endif // UDPDIALOG_H
