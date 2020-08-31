#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialport.h"
#include <QThread>
#include <QtCharts/QtCharts>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //进入程序马上打开串口
    usart1=new SerialPort(time+"左脚.csv","COM8");
    usart1->moveToThread(&thread1);
    connect(&thread1,&QThread::started,usart1,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread1,&QThread::finished,usart1,&SerialPort::closeSerialPortslot); //关闭串口
    connect(this,&MainWindow::startTimerSignal,usart1,&SerialPort::startTimerSlot); //打开定时器，记录数据
    connect(usart1,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart1,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
    connect(ui->btnInitAngle,&QPushButton::clicked,usart1,&SerialPort::setAngleZeroSlot); //初始角度校准


    usart2=new SerialPort(time+"右脚.csv","COM6");
    usart2->moveToThread(&thread2);
    connect(&thread2,&QThread::started,usart2,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread2,&QThread::finished,usart2,&SerialPort::closeSerialPortslot); //关闭串口
    connect(this,&MainWindow::startTimerSignal,usart2,&SerialPort::startTimerSlot);
    connect(usart2,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart2,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
    connect(ui->btnInitAngle,&QPushButton::clicked,usart2,&SerialPort::setAngleZeroSlot); //初始角度校准


    usart3=new SerialPort(time+"腰.csv","COM4");
    usart3->moveToThread(&thread3);
    connect(&thread3,&QThread::started,usart3,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread3,&QThread::finished,usart3,&SerialPort::closeSerialPortslot); //关闭串口
    connect(this,&MainWindow::startTimerSignal,usart3,&SerialPort::startTimerSlot);
    connect(usart3,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart3,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
    connect(ui->btnInitAngle,&QPushButton::clicked,usart3,&SerialPort::setAngleZeroSlot); //初始角度校准


    thread1.start();
    thread2.start();
    thread3.start();

    connect(ui->btnCollectData,&QPushButton::clicked,this,&MainWindow::beginDataSlot);
    connect(ui->btnStopColloct,&QPushButton::clicked,this,&MainWindow::enddataSlot);
    connect(ui->btnLoad,&QPushButton::clicked,this,&MainWindow::loadFileSlot);
    connect(ui->btnClear,&QPushButton::clicked,this,&MainWindow::clearSlot);

}


MainWindow::~MainWindow()
{
    delete ui;
    delete chartwidget;
}

void MainWindow::angleInitedSlot()
{
    static int time=0;
    time++;
    if(time>=3) ui->label1->setText("校准完成");
}

void MainWindow::portOpenedSlot()
{
    static int time1=0;
    time1++;
    if(time1>=3) ui->label2->setText("串口全打开");
}


void MainWindow::beginDataSlot()
{
    emit startTimerSignal();
}

void MainWindow::enddataSlot()
{
    thread1.exit();
    thread1.wait();
    thread2.exit();
    thread2.wait();
    thread3.exit();
    thread3.wait();
    delete usart1;
    delete usart2;
    delete usart3;
}



void MainWindow::loadFileSlot()
{
    QFileDialog* fd=new QFileDialog(this);
    QString fileName = fd->getOpenFileName(this,tr("Open File"),"C:/Users/15705/Desktop/C++/build-Sensor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug",tr("Excel(*.csv)"));
    if(fileName == "")
          return;

    bool accx=ui->accx->isChecked();
    bool accy=ui->accy->isChecked();
    bool accz=ui->accz->isChecked();
    bool wx=ui->wx->isChecked();
    bool wy=ui->wy->isChecked();
    bool wz=ui->wz->isChecked();
    bool anglex=ui->anglex->isChecked();
    bool angley=ui->angley->isChecked();
    bool anglez=ui->anglez->isChecked();
    bool q0=ui->q0->isChecked();
    bool q1=ui->q1->isChecked();
    bool q2=ui->q2->isChecked();
    bool q3=ui->q3->isChecked();

    chartwidget=new ChartWidget;
    chartwidget->loadDataFromCSV(fileName,accx,accy,accz,wx,wy,wz,anglex,angley,anglez,q0,q1,q2,q3);
    chartwidget->chartPaint();
    ui->graphicsView->setChart((chartwidget->m_chart));
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::clearSlot()
{
    //ui->graphicsView->clearFocus();
}



