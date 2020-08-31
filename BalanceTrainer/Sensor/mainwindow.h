#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "serialport.h"
#include "paintchart.h"
#include <QMainWindow>
#include <QChart>

QT_CHARTS_USE_NAMESPACE
QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startTimerSignal();

public slots:

    void beginDataSlot();
    void enddataSlot();

    void loadFileSlot();
    void clearSlot();

    void portOpenedSlot();
    void angleInitedSlot();

private:
    Ui::MainWindow *ui;
    ChartWidget* chartwidget;

    QThread thread1;
    SerialPort* usart1;
    QThread thread2;
    SerialPort* usart2;
    QThread thread3;
    SerialPort* usart3;
    QString time=QDateTime::currentDateTime().toString("MMddhhmm");
};
#endif // MAINWINDOW_H
