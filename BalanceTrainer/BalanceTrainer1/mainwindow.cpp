#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include "panel.h"
#include <windows.h>
#include "platformcontrol.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TabMotionControlInit();
    TabTrajectoryPlanningInit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* ************************************
 * 运动控制面板的相关控制函数
 * 运动控制在子线程中运行
 * **********************************/
void MainWindow::TabMotionControlInit()
{
    //当指定父亲为窗口时，创建的对象在窗口销毁时会自动销毁，不需要手动delete
    m_motionControlThread=new QThread(this);
    m_motioncontrol=new MotionControl(this);

    m_motioncontrol->moveToThread(m_motionControlThread);

    connect(m_motionControlThread,&QThread::started,m_motioncontrol,&MotionControl::threadStartSlot);

    //速度控制模式相关信号和槽
    connect(ui->btnSpeedModeStart,&QPushButton::clicked,this,&MainWindow::startSpeedModeSlot);
    connect(ui->btnSpeedModeStop,&QPushButton::clicked,m_motioncontrol,&MotionControl::stopSpeedAndPosModeSlot);
    connect(this,&MainWindow::startSpeedModeSignal,m_motioncontrol,&MotionControl::startSpeedModeSlot);

    //平台急停，复位，取消急停三个按钮的信号和槽
    connect(ui->btnPlatformReset,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformResetSlot);
    connect(ui->btnPlatformHalt,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformHaltSlot);
    connect(ui->btnHaltCancel,&QPushButton::clicked,m_motioncontrol,&MotionControl::platformCancelHaltSlot);

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

    m_accGrp=new QButtonGroup(this);
    m_accGrp->addButton(ui->rbtnAccStep,0);
    m_accGrp->addButton(ui->rbtnAccSlope,1);

    m_wGrp=new QButtonGroup(this);
    m_wGrp->addButton(ui->rbtnWStep,0);
    m_wGrp->addButton(ui->rbtnWSlope,1);

    //connect();
}






