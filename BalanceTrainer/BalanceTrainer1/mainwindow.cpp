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

    connect(ui->actionHardware,&QAction::triggered,[=](){
        UDPDialog UDPdlg;
        int res=UDPdlg.exec();
        if(res==QDialog::Accepted)
        {
            UDPdlg.getUDDPData();  //保存设定参数
        }
    });

    //速度控制模式
    connect(ui->actionspeed,&QAction::triggered,[=](){
        SpeedMode SMDlg;
        SMDlg.setParent(this);
        SMDlg.move((this->width()-SMDlg.width())/2,5);
        SMDlg.exec();
    });

    //位置控制模式
    connect(ui->actionpos,&QAction::triggered,[=](){
        PositionMode PMDlg;
        PMDlg.setParent(this);
        PMDlg.move((this->width()-PMDlg.width())/2,15);
        PMDlg.exec();

    });

    //轨迹规划
    connect(ui->actionpath,&QAction::triggered,[=](){
        class TrajectoryPlan TPDlg;
        TPDlg.setParent(this);
        TPDlg.move((this->width()-TPDlg.width())/2,15);
        TPDlg.exec();
    });



    thread=new GetDataCmdThread(nullptr);
    thread->start();
}


MainWindow::~MainWindow()
{
    thread->exit=true;  //终止线程
    thread->wait();   //等待线程结束
    delete ui;
}






