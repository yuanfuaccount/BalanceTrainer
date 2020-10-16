#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <windows.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TabMotionControlInit();
    TabTrajectoryPlanningInit();
    TabWashoutInit();
    TabSensorInit();
    TabImgInit();
    TabGaitPhaseExhibitionInit();
    TabGaitSymmtreyInit();
}

MainWindow::~MainWindow()
{
    m_motionControlThread->exit();
    m_motionControlThread->wait();
    delete m_motioncontrol;
    delete ui;

    if(USELEFTFOOT)
    {
        m_sensorThread1.exit();
        m_sensorThread1.wait();

        delete leftFootSensor;
    }
    if(USERIGHTFOOT)
    {
        m_sensorThread2.exit();
        m_sensorThread2.wait();

        delete rightFootSensor;
    }
    if(USEWAIST)
    {
        m_sensorThread3.exit();
        m_sensorThread3.wait();

        delete waistSensor;
    }

    delete m_chartWidget;
    delete m_gaitPhaseExhibition;

    delete m_ychart;//步态对称性显示部分
    delete m_zchart;
}

/* ************************************
 * 运动控制面板的相关控制函数
 * 运动控制在子线程中运行
 * **********************************/
void MainWindow::TabMotionControlInit()
{
    //当指定父亲为窗口时，创建的对象在窗口销毁时会自动销毁，不需要手动delete
    m_motionControlThread=new QThread(this);
    m_motioncontrol=new MotionControl();

    m_motioncontrol->moveToThread(m_motionControlThread);

    connect(m_motionControlThread,&QThread::started,m_motioncontrol,&MotionControl::threadStartSlot);

    //速度控制模式相关信号和槽
    connect(ui->btnSpeedModeStart,&QPushButton::clicked,this,&MainWindow::startSpeedModeSlot);
    connect(ui->btnSpeedModeStop,&QPushButton::clicked,m_motioncontrol,&MotionControl::stopSpeedAndPosModeSlot);
    connect(this,&MainWindow::startSpeedModeSignal,m_motioncontrol,&MotionControl::startSpeedModeSlot);

    //平台急停，复位，运动到中点，取消急停四个按钮的信号和槽
    connect(ui->btnPlatformReset,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformResetSlot);
    connect(ui->btnPlatformHalt,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformHaltSlot);
    connect(ui->btnHaltCancel,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformCancelHaltSlot);
    connect(ui->btnPlatformToMid,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformToMiddleSlot);

    //位置模式相关信号和槽
    connect(ui->btnPositionModeStart,&QPushButton::clicked,this,&MainWindow::startPosModeSlot);
    connect(ui->btnPositionModeStop,&QPushButton::clicked,m_motioncontrol,&MotionControl::stopSpeedAndPosModeSlot);
    connect(this,&MainWindow::startPosModeSignal,m_motioncontrol,&MotionControl::startPositionModeSlot);

    m_motionControlThread->start();
}


void MainWindow::startSpeedModeSlot()
{
    int x=ui->boxSpeedX->text().toInt();
    int y=ui->boxSpeedY->text().toInt();
    int z=ui->boxSpeedZ->text().toInt();
    int roll=ui->boxSpeedRoll->text().toInt();
    int yaw=ui->boxSpeedYaw->text().toInt();
    int pitch=ui->boxSpeedPitch->text().toInt();
    emit startSpeedModeSignal(x,y,z,roll,yaw,pitch);
}

void MainWindow::startPosModeSlot()
{
    int x=ui->boxPosX->text().toInt();
    int y=ui->boxPosY->text().toInt();
    int z=ui->boxPosZ->text().toInt();
    int roll=ui->boxPosRoll->text().toInt();
    int yaw=ui->boxPosYaw->text().toInt();
    int pitch=ui->boxPosPitch->text().toInt();
    int runTime=ui->boxRunTime->text().toInt();

    if(runTime==0)
    {
        ui->textNotice->insertPlainText("运行时间不能为零!");
        return;
    }
    emit startPosModeSignal(x,y,z,roll,yaw,pitch,runTime);
}


/* ********************************
 * 轨迹规划相关函数
 * 轨迹规划从CSV文件加载轨迹，传入motioncontrol中开始运行
 * *******************************/
void MainWindow::TabTrajectoryPlanningInit()
{
    m_trajectoryPlanning=new TrajectoryPlanning(this);
    ui->btnTrajectoryStart->setEnabled(false); //最开始没有路径轨迹，不能点击此按钮
    connect(ui->btnLoadTrajectory,&QPushButton::clicked,this,&MainWindow::loadTrajectorySlot);
    connect(ui->btnTrajectoryStart,&QPushButton::clicked,m_trajectoryPlanning,&TrajectoryPlanning::startTrajectroyPlanningSlot);
    connect(m_trajectoryPlanning,&TrajectoryPlanning::startTrajectroyPlanningSignal,m_motioncontrol,&MotionControl::startTrajectoryPlanningSlot);
}

void MainWindow::loadTrajectorySlot()
{
    QFileDialog* fd=new QFileDialog(this);
    QString fileName = fd->getOpenFileName(this,tr("Open File"),"D:/Files/GitRepository/BalanceTrainer/BalanceTrainer1",tr("Excel(*.csv)"));
    if(fileName == "")
          return;
    m_trajectoryPlanning->loadTrajectoryFile(fileName);
    ui->btnTrajectoryStart->setEnabled(true);
}


/* **************************************
 * 体感仿真相关函数
 * 由于体感仿真模块washout需要实时改变输入加速度和角速度并检测运行时间,因此将其作为motioncontrol的一个私有成员进行初始化
 * 在motioncontrol的定时器中先利用Washout::getWashOut获取输出位移，然后进行位置控制
 * 因此，此处将washout面板相关槽函数都归类为体感仿真模块而不是motioncontrol模块
 * *************************************/
void MainWindow::TabWashoutInit()
{

    m_modeGrp=new QButtonGroup(this);
    m_modeGrp->addButton(ui->rbtnX,0);
    m_modeGrp->addButton(ui->rbtnY,1);
    m_modeGrp->addButton(ui->rbtnYaw,2);
    m_modeGrp->addButton(ui->rbtnJolt,3);




    connect(ui->btnStartWashout,&QPushButton::clicked,this,&MainWindow::startWashoutSlot);
    connect(this,&MainWindow::startWashoutSignal,m_motioncontrol,&MotionControl::startWashoutSlot);
}


void MainWindow::startWashoutSlot()
{
    QVector<QVector<double>> value(4,QVector<double>(3,0));
    value[0][0]=ui->sbAccX_XMode->text().toDouble();
    value[0][1]=ui->sbTime_XMode->text().toDouble();
    value[0][2]=ui->sbSlopeTime_XMode->text().toDouble();

    value[1][0]=ui->sbAccY_YMode->text().toDouble();
    value[1][1]=ui->sbTime_YMode->text().toDouble();
    value[1][2]=ui->sbSlopeTime_YMode->text().toDouble();

    value[2][0]=ui->sbWZ_ZMode->text().toDouble();
    value[2][1]=ui->sbTime_ZMode->text().toDouble();
    value[2][2]=ui->sbSlopeTime_ZMode->text().toDouble();

    if(ui->cbZ->isChecked())
        value[3][0]=1;
    if(ui->cbRoll->isChecked())
        value[3][1]=1;
    if(ui->cbPitch->isChecked())
        value[3][2]=1;


    int mode=m_modeGrp->checkedId();

    emit startWashoutSignal(value[mode][0],value[mode][1],value[mode][2],mode);
}


/* ***********************************
 * 传感器数据采集部分，主要是三个线程，每个线程单独运行一个传感器数据采集函数
 * **********************************/
void MainWindow::TabSensorInit()
{
    //进入程序马上打开串口
    if(USELEFTFOOT)
    {
        leftFootSensor=new FootSensor(time+"左脚.csv","COM8");
        leftFootSensor->moveToThread(&m_sensorThread1);
        leftFootSensor->moveToThread(&m_sensorThread1);
        connect(&m_sensorThread1,&QThread::started,leftFootSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&m_sensorThread1,&QThread::finished,leftFootSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,leftFootSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(leftFootSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(leftFootSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot); //校准完成提示
        m_sensorThread1.start();
    }

    if(USERIGHTFOOT)
    {
        rightFootSensor=new FootSensor(time+"右脚.csv","COM6");
        rightFootSensor->moveToThread(&m_sensorThread2);
        connect(&m_sensorThread2,&QThread::started,rightFootSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&m_sensorThread2,&QThread::finished,rightFootSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,rightFootSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(rightFootSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(rightFootSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
        m_sensorThread2.start();
    }

    if(USEWAIST)
    {
        waistSensor=new WaistSensor(time+"腰.csv","COM7");
        waistSensor->moveToThread(&m_sensorThread3);
        connect(&m_sensorThread3,&QThread::started,waistSensor->usart,&SerialPort::openSerialPortSlot); //打开串口
        connect(&m_sensorThread3,&QThread::finished,waistSensor->usart,&SerialPort::closeSerialPortslot); //关闭串口
        connect(ui->btnInitAngle,&QPushButton::clicked,waistSensor->usart,&SerialPort::setAngleZeroSlot); //初始角度校准
        connect(ui->btnCollectData,&QPushButton::clicked,waistSensor->usart,&SerialPort::startDataCollectSlot); //开始记录数据
        connect(ui->btnStopColloct,&QPushButton::clicked,waistSensor->usart,&SerialPort::endDataCollectSlot); //停止记录数据，开始处理数据
        connect(waistSensor->usart,&SerialPort::portOpenedSignal,this,&MainWindow::portOpenedSlot); //当所有串口都打开时提示
        connect(waistSensor->usart,&SerialPort::initAgnleFinishedSignal,this,&MainWindow::angleInitedSlot);
        m_sensorThread3.start();
    }


}

void MainWindow::portOpenedSlot()
{
    static int count=0;
    count++;
    if(count>=3)
        ui->labelIsPortOpend->setText("串口全打开");
}

void MainWindow::angleInitedSlot()
{
    static int count=0;
    count++;
    if(count>=3)
        ui->labelIsAngInited->setText("角度初始化完成");
}

/* **************************
 * 原始图象的加载部分
 * ***************************/
void MainWindow::TabImgInit()
{
    m_chartWidget=new ChartWidget();
    connect(ui->btnLoadImg,&QPushButton::clicked,this,&MainWindow::loadImgSlot);
    connect(ui->btnClearImg,&QPushButton::clicked,this,&MainWindow::clearImgSlot);
}


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

        m_chartWidget->loadDataFromCSV(fileName,accx,accy,accz,wx,wy,wz,anglex,angley,anglez);
        m_chartWidget->chartPaint("time/s");
        ui->graphicsView->setChart((m_chartWidget->m_chart)); //注：此处要把QgraphicsView提升为QChartView,才能使用此函数
        ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::clearImgSlot()
{
    m_chartWidget->chartClear();
    ui->graphicsView->setChart((m_chartWidget->m_chart));
}


/* ************************
 * 步态时相数据显示界面
 * ************************/
void MainWindow::TabGaitPhaseExhibitionInit()
{
    m_gaitPhaseExhibition=new GaitPhaseExhibition();
    if(USELEFTFOOT)
    {
        connect(leftFootSensor->detector,&GaitPhaseDetection::gaitPhaseDataProcessFinished,this,&MainWindow::showLeftGaitPhaseTimeSlot);
        ui->leftFoot->resizeRowsToContents();
    }
    if(USERIGHTFOOT)
    {
        connect(rightFootSensor->detector,&GaitPhaseDetection::gaitPhaseDataProcessFinished,this,&MainWindow::showRightGaitPhaseTimeSlot);
        ui->rightFoot->resizeRowsToContents();
    }
}

void MainWindow::showLeftGaitPhaseTimeSlot()
{
    if(USELEFTFOOT && !leftFootSensor->detector->gaitPhaseTime.empty())
        m_gaitPhaseExhibition->fillTableAndChart(leftFootSensor,true,ui->leftFoot,ui->gaitPhaseStatistics,ui->leftFootPie);
}

void MainWindow::showRightGaitPhaseTimeSlot()
{
    if(USERIGHTFOOT && !rightFootSensor->detector->gaitPhaseTime.empty())
        m_gaitPhaseExhibition->fillTableAndChart(rightFootSensor,false,ui->rightFoot,ui->gaitPhaseStatistics,ui->rightFootPie);
}


/* *****************************
 * 步态对称性显示面板
 * *******************************/
void MainWindow::TabGaitSymmtreyInit()
{
    m_ychart=new AutoCorrChart();
    m_zchart=new AutoCorrChart();
    if(USEWAIST)
        connect(waistSensor->detector,&GaitSymmetry::gaitSymmetryDataProcessFinished,this,&MainWindow::showGaitSymmetrySlot);
}

void MainWindow::showGaitSymmetrySlot()
{
    //xchart->paitAutoCorrChart(waistSensor->detector->m_autoCorrX);
    m_ychart->paitAutoCorrChart(waistSensor->detector->m_autoCorrY);
    m_zchart->paitAutoCorrChart(waistSensor->detector->m_autoCorrZ);
    //ui->ViewAutoCorrX->setChart(xchart->m_chart);
    //ui->ViewAutoCorrX->setRenderHint(QPainter::Antialiasing);

    ui->ViewAutoCorrY->setChart(m_ychart->m_chart);
    ui->ViewAutoCorrY->setRenderHint(QPainter::Antialiasing);

    ui->ViewAutoCorrZ->setChart(m_zchart->m_chart);
    ui->ViewAutoCorrZ->setRenderHint(QPainter::Antialiasing);

    for(int i=0;i<6;i++)
    {
        QTableWidgetItem* item=new QTableWidgetItem();
        item->setText(QString::number(waistSensor->detector->m_gaitSymData[i],'f',2));
        ui->tableGaitSymData->setItem(0,i,item);
    }
}







