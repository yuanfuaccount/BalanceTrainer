#include "panel.h"
#include "ui_udpdialog.h"
#include "ui_SpeedMode.h"
#include "ui_PositionMode.h"
#include "ui_TrajectoryPlan.h"
#include <QPushButton>
#include "motorcontrol.h"
#include <QMessageBox>
#include "config.h"
#include <QTimer>
#include "platformcontrol.h"
#include <QDebug>
#include <math.h>
#define PI 3.1415926

/* ***************************************
 * 此文件用于各种UI界面的设计和配置
 * *************************************/


//1. 硬件配置界面
UDPDialog::UDPDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UDPDialog)
{
    ui->setupUi(this);
    //设置默认值

    connect(ui->saveButton,&QPushButton::clicked,this,&QDialog::accept);
    connect(ui->cacelButton,&QPushButton::clicked,this,&QDialog::reject);
}

UDPDialog::~UDPDialog()
{
    delete ui;
}

/* *****************************************
 * 返回参数为：上位机发送/接收端口，MDBox发送接收端口，接收信号的电机IP Group/Node,回传信号的电机IP Group/Node,电动缸行程，一圈脉冲数，电动缸导程
 *
 * ***************************************/
void UDPDialog::getUDDPData(void)
{
    UDPSend sender;
    ushort ChannelCode=0x0001; //写Fn参数并保存

    int PCSendPort=ui->PCSend->value();
    int PCRecvPort=ui->PCRecv->value();
    int MDSendPort=ui->MDSend->value();
    int MDRecvPort=ui->MDRecv->value();
    int SendIPG=ui->SendIPGroup->value();
    int SendIPN=ui->SendIPNode->value();
    int RecvIPG=ui->ReplyIPGroup->value();
    int RecvIPN=ui->ReplyIPNode->value();

    int MotorDisplacement=ui->MotorDis->value();
    int Pulse=ui->PulsePerCycle->value();
    int PitchVal=ui->Pitch->value();
    double Ratio=ui->GearRatio->value();

    I16HostTxPort=static_cast<ushort>(PCSendPort);
    I16HostRxPort=static_cast<ushort>(PCRecvPort);  //接收端口
    I16MboxTxPort=static_cast<ushort>(MDSendPort);
    I16MboxRxPort=static_cast<ushort>(MDRecvPort);

    WhoAcceptIPGroup=static_cast<uint8_t>(SendIPG);
    WhoAcceptIPNode=static_cast<uint8_t>(SendIPN);
    WhoReplyIPGroup=static_cast<uint8_t>(RecvIPG);
    WhoReplyIPNode=static_cast<uint8_t>(RecvIPN);

    AccessDistance =static_cast<float>(MotorDisplacement) ;//电动缸行程
    PitchDistance =static_cast<float>(PitchVal) ;//电动缸导程
    PulsePerCycle =static_cast<uint>(Pulse) ;//一圈脉冲数
    GearRatio = Ratio;//减速比

    sender.WriteRegCmd(ChannelCode,0x0032,static_cast<ushort>(SendIPG));
    sender.WriteRegCmd(ChannelCode,0x0033,static_cast<ushort>(SendIPN));
    sender.WriteRegCmd(ChannelCode,0x0022,static_cast<ushort>(RecvIPG));
    sender.WriteRegCmd(ChannelCode,0x0023,static_cast<ushort>(RecvIPN));

    sender.WriteRegCmd(ChannelCode,0x0034,static_cast<ushort>(PCSendPort));
    sender.WriteRegCmd(ChannelCode,0x0035,static_cast<ushort>(PCRecvPort));
    sender.WriteRegCmd(ChannelCode,0x0024,static_cast<ushort>(MDSendPort));
    sender.WriteRegCmd(ChannelCode,0x0025,static_cast<ushort>(MDRecvPort));

    sender.WriteRegCmd(ChannelCode,0x00C0,static_cast<ushort>(MotorDisplacement*10));  //有效形成设定单位0.1mm
    sender.WriteRegCmd(ChannelCode,0x00C2,static_cast<ushort>(PitchVal*10));    //导程，单位0.1mm
    sender.WriteRegCmd(ChannelCode,0x00C1,static_cast<ushort>(Pulse*Ratio/PitchVal));    //Fn 0c1是电动缸每移动1mm的脉冲数=Pulse*Ratio/PitchVal

    QMessageBox::warning(this,"Notice","参数保存成功");
}


//2. 速度模式面板
SpeedMode::SpeedMode(QWidget *parent) :
    QDialog(parent),
    SM(new Ui::SpeedMode)
{
    SM->setupUi(this);
    //设置默认值

    tim=new QTimer();
    //uint xbasepos,ybasepos,zbasepos,ubasepos,vbasepos,wbasepos;

    connect(SM->close,&QPushButton::clicked,this,&QDialog::reject);  //关闭界面


    connect(SM->start,&QPushButton::clicked,this,[=](){
        PlatformSpeed.push_back(SM->speedx->value());
        PlatformSpeed.push_back(SM->speedy->value());
        PlatformSpeed.push_back(SM->speedz->value());


        PlatformSpeed.push_back(SM->speedroll->value()*PI/180);
        PlatformSpeed.push_back(SM->speedpitch->value()*PI/180);
        PlatformSpeed.push_back(SM->speedyaw->value()*PI/180);

        MotorPosMutex.lock();
        BasePos.push_back(xpos);
        BasePos.push_back(ypos);
        BasePos.push_back(zpos);
        BasePos.push_back(upos);
        BasePos.push_back(vpos);
        BasePos.push_back(wpos);
        MotorPosMutex.unlock();

        time=0;

        tim->start(100);
    });

    connect(SM->stop,&QPushButton::clicked,this,[=](){
        tim->stop();
    });

    //时钟中断的函数
    connect(tim,&QTimer::timeout,this,[=](){
        time+=0.1;  //0.1是时间增量

        PlatformSpeedControl(PlatformSpeed,BasePos,time);
    });

}


SpeedMode::~SpeedMode()
{
    delete SM;
}

//3. 位置控制模式面板
PositionMode::PositionMode(QWidget *parent):
    QDialog(parent),
    PM(new Ui::PositionMode)
{
    PM->setupUi(this);
    connect(PM->close,&QPushButton::clicked,this,&QDialog::reject);
    connect(PM->start,&QPushButton::clicked,this,[=]()
    {
        position.push_back(PM->posx->value());
        position.push_back(PM->posy->value());
        position.push_back(PM->posz->value());
        position.push_back(PM->roll->value()*PI/180);
        position.push_back(PM->pitch->value()*PI/180);
        position.push_back(PM->yaw->value()*PI/180);

        ushort time=10; //先暂时规定10s完成位移运动
        PlatFormPositionControl(position,time);
    });

}

PositionMode::~PositionMode()
{
    delete PM;
}

TrajectoryPlan::TrajectoryPlan(QWidget* parent):
    QDialog(parent),
    TP(new Ui::TrajectoryPlan)
{
    TP->setupUi(this);
//    connect(TP->Path1,&QRadioButton::toggled,this,[=](){
//        if(pathnum!=1)
//        {
//            datanum=0;
//            pathnum=1;
//            Path.clear();
//            ReadPathData(Path);
//        }
//    });
//    connect(TP->Path2,&QRadioButton::toggled,this,[=](){
//        if(pathnum!=2)
//        {
//            datanum=0;
//            pathnum=2;
//            Path.clear();
//            ReadPathData(Path);
//        }
//    });
//    connect(TP->Path3,&QRadioButton::toggled,this,[=](){
//        if(pathnum!=3)
//        {
//            datanum=0;
//            pathnum=3;
//            Path.clear();
//            ReadPathData(Path);
//        }
//    });

//    connect(TP->start,&QPushButton::clicked,this,[=](){
//        tim=new QTimer();
//        tim->start(100);
//    });
//    connect(tim,&QTimer::timeout,this,[=](){
//        ExecPath(Path[datanum]);
//        datanum++;
//        if(datanum==Path.size())
//            tim->stop();
//    });

}

TrajectoryPlan::~TrajectoryPlan()
{
    delete TP;
}

