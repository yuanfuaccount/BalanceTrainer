#include "config.h"
#include <stdlib.h>
#include <Eigen/Core>

/* ******************************
 * 此文件用于全局变量定义，各种配置参数
 * ****************************/


//1.硬件配置参数
//硬件设置相关参数，包括发送和接收上位机和MDBOX的port，IP，电动缸参数
ushort I16HostTxPort=8410; //上位机发送端口，默认8410
ushort I16HostRxPort=8410;  //接收端口
ushort I16MboxTxPort=7408;
ushort I16MboxRxPort=7408;

uint8_t WhoAcceptIPGroup=255;
uint8_t WhoAcceptIPNode=255;
uint8_t WhoReplyIPGroup=255;
uint8_t WhoReplyIPNode=255;

float AccessDistance = 50;//电动缸行程
float PitchDistance = 5;//电动缸导程
uint PulsePerCycle = 10000;//一圈脉冲数
double GearRatio = 1;//减速比

//2.各个电动缸位置,单位是脉冲数
uint xpos;
uint ypos;
uint zpos;
uint upos;
uint vpos;
uint wpos;
QMutex MotorPosMutex;  //电机反馈的位置为共享全局变量，采用互斥锁

//上平台中心坐标，单位mm和角度
double px,py,pz;
double roll,pitch,yaw;
QMutex PlatformPosMutex;

//平台尺寸参数
//平台的尺寸参数
const double PlatformPara::UpperRadius=328.07;
const double PlatformPara::BaseRadius=655.32;

const Eigen::Vector3d PlatformPara::B1_0(652.56,60,0);
const Eigen::Vector3d PlatformPara::B2_0(-274.32,595.14,0);
const Eigen::Vector3d PlatformPara::B3_0(-378.24,535.14,0);
const Eigen::Vector3d PlatformPara::B4_0(-378.24,-535.14,0);
const Eigen::Vector3d PlatformPara::B5_0(-274.32,-595.14,0);
const Eigen::Vector3d PlatformPara::B6_0(652.56,-60,0);

const Eigen::Vector3d PlatformPara::P1_0(213.23,249.33,0);
const Eigen::Vector3d PlatformPara::P2_0(109.31,309.33,0);
const Eigen::Vector3d PlatformPara::P3_0(-322.54,60,0);
const Eigen::Vector3d PlatformPara::P4_0(-322.54,-60,0);
const Eigen::Vector3d PlatformPara::P5_0(109.31,-309.33,0);
const Eigen::Vector3d PlatformPara::P6_0(213.23,-249.33,0);
const Eigen::Vector3d PlatformPara::T0(0,0,100);

