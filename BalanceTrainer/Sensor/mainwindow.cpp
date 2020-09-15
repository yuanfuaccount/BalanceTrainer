#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialport.h"
#include <QThread>
#include <QtCharts/QtCharts>
#include <QTabWidget>
#include "paintchart.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectSignalAndSlot();
}


MainWindow::~MainWindow()
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
    delete chartwidget;

    delete ui;
}

void MainWindow::angleInitedSlot()
{
    static int time=0;
    time++;
    if(time>=2) ui->label1->setText("校准完成");
}

void MainWindow::portOpenedSlot()
{
    static int time1=0;
    time1++;
    if(time1>=2) ui->label2->setText("串口全打开");
}

void MainWindow::connectSignalAndSlot()
{
    /**********************************
     * 串口相关配置，主要是线程开启时打开串口，关闭时关闭串口
     * 界面上的按钮对串口的控制
     * *******************************/
    //进入程序马上打开串口
    usart1=new SerialPort(time+"左脚.csv","COM8");
    usart1->moveToThread(&thread1);
    connect(&thread1,&QThread::started,usart1,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread1,&QThread::finished,usart1,&SerialPort::closeSerialPortslot); //关闭串口
    connect(ui->btnInitAngle,&QPushButton::clicked,usart1,&SerialPort::setAngleZeroSlot); //初始角度校准
    connect(ui->btnCollectData,&QPushButton::clicked,usart1,&SerialPort::startDataCollectSlot); //开始记录数据
    connect(ui->btnStopColloct,&QPushButton::clicked,usart1,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
    connect(usart1,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart1,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot); //校准完成提示


    usart2=new SerialPort(time+"右脚.csv","COM6");
    usart2->moveToThread(&thread2);
    connect(&thread2,&QThread::started,usart2,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread2,&QThread::finished,usart2,&SerialPort::closeSerialPortslot); //关闭串口
    connect(ui->btnInitAngle,&QPushButton::clicked,usart2,&SerialPort::setAngleZeroSlot); //初始角度校准
    connect(ui->btnCollectData,&QPushButton::clicked,usart2,&SerialPort::startDataCollectSlot); //开始记录数据
    connect(ui->btnStopColloct,&QPushButton::clicked,usart2,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
    connect(usart2,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart2,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);


    usart3=new SerialPort(time+"腰.csv","COM4");
    usart3->moveToThread(&thread3);
    connect(&thread3,&QThread::started,usart3,&SerialPort::openSerialPortSlot); //打开串口
    connect(&thread3,&QThread::finished,usart3,&SerialPort::closeSerialPortslot); //关闭串口
    connect(ui->btnInitAngle,&QPushButton::clicked,usart3,&SerialPort::setAngleZeroSlot); //初始角度校准
    connect(ui->btnCollectData,&QPushButton::clicked,usart3,&SerialPort::startDataCollectSlot); //开始记录数据
    connect(ui->btnStopColloct,&QPushButton::clicked,usart3,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
    connect(usart3,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
    connect(usart3,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);

    thread1.start();
    thread2.start();
//    thread3.start();

    //原始图像加载相关按钮
    chartwidget=new ChartWidget;
    connect(ui->btnLoadImg,&QPushButton::clicked,this,&MainWindow::loadImgSlot);
    connect(ui->btnClearImg,&QPushButton::clicked,this,&MainWindow::clearImgSlot);

    //分析图像加载相关配置
    connect(usart1,&SerialPort::dataProcessFinished,this,&MainWindow::showGaitPhaseTimeSlot);
    connect(usart2,&SerialPort::dataProcessFinished,this,&MainWindow::showGaitPhaseTimeSlot);

    ui->leftFoot->resizeRowsToContents();
    ui->rightFoot->resizeRowsToContents();

}

//加载最新的图像
void MainWindow::loadImgSlot()
{
        QFileDialog* fd=new QFileDialog(this);
        QString fileName = fd->getOpenFileName(this,tr("Open File"),"D:\Files\GitRepository\BalanceTrainer\build-Sensor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug",tr("Excel(*.csv)"));
        if(fileName == "")
              return;
        bool accx=ui->cbAccX1->isChecked();
        bool accy=ui->cbAccY1->isChecked();
        bool accz=ui->cbAccZ1->isChecked();
        bool wx=ui->cbWX1->isChecked();
        bool wy=ui->cbWY1->isChecked();
        bool wz=ui->cbWZ1->isChecked();
        bool anglex=ui->cbAngX1->isChecked();
        bool angley=ui->cbAngY1->isChecked();
        bool anglez=ui->cbAngZ1->isChecked();

        ChartWidget* chartwidget=new ChartWidget;
        chartwidget->loadDataFromCSV(fileName,accx,accy,accz,wx,wy,wz,anglex,angley,anglez);
        chartwidget->chartPaint();
        ui->graphicsView->setChart((chartwidget->m_chart)); //注：此处要把QgraphicsView提升为QChartView,才能使用此函数
        ui->graphicsView->setRenderHint(QPainter::Antialiasing);

}

void MainWindow::clearImgSlot()
{
    chartwidget->chartClear();
    ui->graphicsView->setChart((chartwidget->m_chart));
}

void MainWindow::fillTableAndPie(SerialPort* usart,bool leftFoot)
{

    int rowCount=usart->gaitPhaseTime.size();
    QTableWidget* table;
    if(leftFoot)
        table=ui->leftFoot;
    else
        table=ui->rightFoot;
    table->setRowCount(rowCount);
    for(int i=0;i<rowCount;i++)
    {
        QTableWidgetItem* stepNum=new QTableWidgetItem();
        stepNum->setText(QString::number(i+1));
        QTableWidgetItem* loadingPhase=new QTableWidgetItem();
        loadingPhase->setText(QString::number(usart->gaitPhaseTime[i][2]));
        QTableWidgetItem* stancePhase=new QTableWidgetItem();
        stancePhase->setText(QString::number(usart->gaitPhaseTime[i][3]));
        QTableWidgetItem* leavingPhase=new QTableWidgetItem();
        leavingPhase->setText(QString::number(usart->gaitPhaseTime[i][0]));
        QTableWidgetItem* swingPhase=new QTableWidgetItem();
        swingPhase->setText(QString::number(usart->gaitPhaseTime[i][1]));
        QTableWidgetItem* cycle=new QTableWidgetItem();
        cycle->setText(QString::number(usart->gaitPhaseTime[i][4]));

        table->setItem(i,0,stepNum);
        table->setItem(i,1,loadingPhase);
        table->setItem(i,2,stancePhase);
        table->setItem(i,3,leavingPhase);
        table->setItem(i,4,swingPhase);
        table->setItem(i,5,cycle);
    }

    int colNum=0;
    if(!leftFoot)
        colNum=1;
    for(int i=0;i<7;i++)
    {
        QTableWidgetItem* item=new QTableWidgetItem();
        if(i<5)
            item->setText(QString::number(usart->avgGatiPhaseTime[i]));
        else if(i==5)
            item->setText(QString::number(usart->stepEffecitve));
        else if(i==6)
            item->setText(QString::number(usart->stepTotal));
        ui->gaitPhaseStatistics->setItem(i,colNum,item);
    }
    PieChart* pie=new PieChart();
    if(leftFoot)
    {
        pie->paintPieChart(usart->avgGatiPhaseTime,"步行周期(左)");
        ui->leftFootPie->setChart(pie->m_chart);
    }
    else
    {
        pie->paintPieChart(usart->avgGatiPhaseTime,"步行周期(右)");
        ui->rightFootPie->setChart(pie->m_chart);
    }
}

void MainWindow::showGaitPhaseTimeSlot()
{
    if(!usart1->gaitPhaseTime.empty())
        fillTableAndPie(usart1,true);

    if(!usart2->gaitPhaseTime.empty())
        fillTableAndPie(usart2,false);
}











