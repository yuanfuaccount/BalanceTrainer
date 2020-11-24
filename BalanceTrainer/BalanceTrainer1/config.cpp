#include "config.h"
#include <stdlib.h>
#include <Eigen/Core>

/* ******************************
 * 此文件用于全局变量定义，各种配置参数
 * ****************************/

const double PI=3.1415;
const double g=9.8;

//1.硬件配置参数
//硬件设置相关参数，包括发送和接收上位机和MDBOX的port，IP，电动缸参数
const ushort I16HostTxPort=8410; //上位机发送端口，默认8410
const ushort I16HostRxPort=8410;  //接收端口
const ushort I16MboxTxPort=7408;
const ushort I16MboxRxPort=7408;

//const QString TargetIP="127.0.0.1";
const QString TargetIP="192.168.15.255"; //广播IP地址

const double cmdInterval=0.1; //100ms指令时间间隔

const uint8_t WhoAcceptIPGroup=255;
const uint8_t WhoAcceptIPNode=255;
const uint8_t WhoReplyIPGroup=255;
const uint8_t WhoReplyIPNode=255;

const double AccessDistance = 250;//电动缸行程
const double PitchDistance = 10;//电动缸导程
const uint PulsePerCycle = 10000;//一圈脉冲数
const double GearRatio = 1.5;//减速比
const uint MaxAccessPul=345000; //最大脉冲数


//腰部传感器的角度数据，需要此数据实现传感器和平台的互动
double waistAngX=0;
double waistAngY=0;


//2.各个电动缸位置,单位是脉冲数
std::atomic_uint g_xpul(0);
std::atomic_uint g_ypul(0);
std::atomic_uint g_zpul(0);
std::atomic_uint g_upul(0);
std::atomic_uint g_vpul(0);
std::atomic_uint g_wpul(0);


//上平台中心坐标，单位mm和角度
std::atomic<double> g_px(0),g_py(0),g_pz(408); //g_pz跟PlatformPara::T0的第三个参数必须保持一致，代表初始时上平台中心Z坐标
std::atomic<double> g_roll(0),g_pitch(0),g_yaw(0); //角度制，在外部的角度统一角度制，在RotationMatrix函数中统一转换


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
const Eigen::Vector3d PlatformPara::T0(0,0,408); //第三个参数必须跟g_pz保持一致，代表初始时上平台中心Z坐标

