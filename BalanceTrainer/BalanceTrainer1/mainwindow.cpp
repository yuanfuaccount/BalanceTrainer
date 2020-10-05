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
}

MainWindow::~MainWindow()
{
    delete ui;
}

//运动控制面板的相关控制函数
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







