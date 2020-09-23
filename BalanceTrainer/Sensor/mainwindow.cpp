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
    if(USE_LEFT_FOOT)
    {
        thread1.exit();
        thread1.wait();

        delete leftFootSensor;
    }
    if(USE_RIGHT_FOOT)
    {
        thread2.exit();
        thread2.wait();

        delete rightFootSensor;
    }
    if(USE_WAIST)
    {
        thread3.exit();
        thread3.wait();

        delete waistSensor;
    }

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
     * 传感器相关配置，主要是线程开启时打开串口，关闭时关闭串口
     * 界面上的按钮对传感器串口的控制
     * *******************************/
    //进入程序马上打开串口
    if(USE_LEFT_FOOT)
    {
        leftFootSensor=new FootSensor(time+"左脚.csv","COM8");
        leftFootSensor->moveToThread(&thread1);
        connect(&thread1,&QThread::started,leftFootSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&thread1,&QThread::finished,leftFootSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(leftFootSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(leftFootSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot); //校准完成提示
        thread1.start();
    }

    if(USE_RIGHT_FOOT)
    {
        rightFootSensor=new FootSensor(time+"右脚.csv","COM6");
        rightFootSensor->moveToThread(&thread2);
        connect(&thread2,&QThread::started,rightFootSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&thread2,&QThread::finished,rightFootSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(rightFootSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(rightFootSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
        thread2.start();
    }

    if(USE_WAIST)
    {
        waistSensor=new WaistSensor(time+"腰.csv","COM7");
        waistSensor->moveToThread(&thread3);
        connect(&thread3,&QThread::started,waistSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&thread3,&QThread::finished,waistSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,waistSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,waistSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,waistSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(waistSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(waistSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
        thread3.start();
    }



    //原始图像加载相关按钮
    chartwidget=new ChartWidget;
    connect(ui->btnLoadImg,&QPushButton::clicked,this,&MainWindow::loadImgSlot);
    connect(ui->btnClearImg,&QPushButton::clicked,this,&MainWindow::clearImgSlot);

    //分析图像加载相关配置
    if(USE_LEFT_FOOT)
    {
        connect(leftFootSensor->detector,&GaitPhaseDetection::gaitPhaseDataProcessFinished,this,&MainWindow::showGaitPhaseTimeSlot);
        ui->leftFoot->resizeRowsToContents();
    }
    if(USE_RIGHT_FOOT)
    {
        connect(rightFootSensor->detector,&GaitPhaseDetection::gaitPhaseDataProcessFinished,this,&MainWindow::showGaitPhaseTimeSlot);
        ui->rightFoot->resizeRowsToContents();
    }

    //自相关函数图像加载
    if(USE_WAIST)
        connect(waistSensor->detector,&GaitSymmetry::gaitSymmetryDataProcessFinished,this,&MainWindow::showGaitSymmetrySlot);

}

//加载最新的图像
void MainWindow::loadImgSlot()
{
        QFileDialog* fd=new QFileDialog(this);
        QString fileName = fd->getOpenFileName(this,tr("Open File"),"D:/Files/GitRepository/BalanceTrainer/build-Sensor-Desktop_Qt_5_12_6_MinGW_64_bit-Debug",tr("Excel(*.csv)"));
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
        chartwidget->chartPaint("time/s");
        ui->graphicsView->setChart((chartwidget->m_chart)); //注：此处要把QgraphicsView提升为QChartView,才能使用此函数
        ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::clearImgSlot()
{
    chartwidget->chartClear();
    ui->graphicsView->setChart((chartwidget->m_chart));
}

void MainWindow::fillTableAndPie(FootSensor* sensor,bool leftFoot)
{
    int rowCount=sensor->detector->gaitPhaseTime.size();
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
        loadingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][2]));
        QTableWidgetItem* stancePhase=new QTableWidgetItem();
        stancePhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][3]));
        QTableWidgetItem* leavingPhase=new QTableWidgetItem();
        leavingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][0]));
        QTableWidgetItem* swingPhase=new QTableWidgetItem();
        swingPhase->setText(QString::number(sensor->detector->gaitPhaseTime[i][1]));
        QTableWidgetItem* cycle=new QTableWidgetItem();
        cycle->setText(QString::number(sensor->detector->gaitPhaseTime[i][4]));

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
            item->setText(QString::number(sensor->detector->avgGatiPhaseTime[i]));
        else if(i==5)
            item->setText(QString::number(sensor->detector->stepEffecitve));
        else if(i==6)
            item->setText(QString::number(sensor->detector->stepTotal));
        ui->gaitPhaseStatistics->setItem(i,colNum,item);
    }
    PieChart* pie=new PieChart();
    if(leftFoot)
    {
        pie->paintPieChart(sensor->detector->avgGatiPhaseTime,"步行周期(左)");
        ui->leftFootPie->setChart(pie->m_chart);
    }
    else
    {
        pie->paintPieChart(sensor->detector->avgGatiPhaseTime,"步行周期(右)");
        ui->rightFootPie->setChart(pie->m_chart);
    }
}

void MainWindow::showGaitPhaseTimeSlot()
{
    if(USE_LEFT_FOOT && !leftFootSensor->detector->gaitPhaseTime.empty())
        fillTableAndPie(leftFootSensor,true);

    if(USE_RIGHT_FOOT && !rightFootSensor->detector->gaitPhaseTime.empty())
        fillTableAndPie(rightFootSensor,false);
}


void MainWindow::showGaitSymmetrySlot()
{
    AutoCorrChart* xchart=new AutoCorrChart();
    AutoCorrChart* ychart=new AutoCorrChart();
    AutoCorrChart* zchart=new AutoCorrChart();
    xchart->paitAutoCorrChart(waistSensor->detector->m_autoCorrX);
    ychart->paitAutoCorrChart(waistSensor->detector->m_autoCorrY);
    zchart->paitAutoCorrChart(waistSensor->detector->m_autoCorrZ);
    ui->ViewAutoCorrX->setChart(xchart->m_chart);
    ui->ViewAutoCorrX->setRenderHint(QPainter::Antialiasing);

    ui->ViewAutoCorrY->setChart(ychart->m_chart);
    ui->ViewAutoCorrY->setRenderHint(QPainter::Antialiasing);

    ui->ViewAutoCorrZ->setChart(zchart->m_chart);
    ui->ViewAutoCorrZ->setRenderHint(QPainter::Antialiasing);
}












