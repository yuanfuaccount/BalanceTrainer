#include "kinematics.h"

/* *********************************
 * 平台运动学逆解，此部分不包装成类，为直接函数调用
 * *******************************/


/* ******************************************
 * 求旋转矩阵
 * 输入：三个欧拉角Yaw,Pitch,Roll
 * 输出：Matrix3d R
 * *****************************************/
Eigen::Matrix3d RotationMatrix(double Yaw,double Pitch,double Roll)
{
    //首先将角度转化为弧度
    Eigen::Vector3d euler(Yaw*PI/180,Pitch*PI/180,Roll*PI/180);  //Yaw,pitch,roll,分别对应绕Z轴偏航，绕Y轴俯仰，绕X轴滚转
    Eigen::AngleAxisd YawAngle(euler[0],Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd PitchAngle(euler[1],Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd RollAngle(euler[2],Eigen::Vector3d::UnitX());

    Eigen::Matrix3d R;
    R=YawAngle*PitchAngle*RollAngle;
    return R;
}


/* ***********************************
 * 平台位置逆解
 * 欧拉角按Z,Y,X的顺序旋转,用角度表示，在RotationMatrix中转化为弧度
 * 输入参数：3个方向的平移+Z,Y,X的欧拉角
 * 返回值：六根电动缸的长度
 * *********************************/
QVector<double> PositionReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll)
{
    Eigen::Matrix3d R;
    R=RotationMatrix(Yaw,Pitch,Roll);

    Eigen::Vector3d t(PosX,PosY,PosZ);
    Eigen::Vector3d T=t;

    //3X6的矩阵，每一列为上平台的一个坐标
    Eigen::Matrix<double,3,6> P0;
    P0<<PlatformPara::P1_0,PlatformPara::P2_0,PlatformPara::P3_0,PlatformPara::P4_0,PlatformPara::P5_0,PlatformPara::P6_0;
    Eigen::Matrix<double,3,6> P;
    Eigen::Matrix<double,3,6> TT;
    TT<<T,T,T,T,T,T;
    P=R*P0+TT;  //上平台铰点在下平台坐标

    Eigen::Matrix<double,3,6> B0;
    B0<<PlatformPara::B1_0,PlatformPara::B2_0,PlatformPara::B3_0,PlatformPara::B4_0,PlatformPara::B5_0,PlatformPara::B6_0;
    Eigen::Matrix<double,3,6> ScalarL; //每根缸的长度向量
    ScalarL=P-B0;

    //求出六个缸的长度
    QVector<double> L;
    for(int i=0;i<6;i++)
        L.push_back(sqrt(ScalarL.col(i).transpose()*ScalarL.col(i)));
    return L;
}


/* *************************************
 * 速度逆解公式，输入平台现阶段位姿以及末端需要的速度，求解每个电动缸伸缩速度
 * 输入：6维位姿坐标(Posx,PosY,PosZ,Yaw,Pitch,Roll)，6维速度广义坐标(Vx,Vy,Vz,Wx,Wy,Wz)
 * 输出：每个电动缸伸缩速度
 * ************************************/
QVector<double> SpeedReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll,
                             double Vx,double Vy,double Vz,double Wx,double Wy,double Wz)
{
    //1.求出6个上铰点速度矢量Vpi
    Eigen::Matrix3d R;
    R=RotationMatrix(Yaw,Pitch,Roll);

    //上铰点的速度Vpi=V+W×RPi_0
    Eigen::Matrix<double,3,6> P0; //上铰点相对于动坐标系的坐标；
    P0<<PlatformPara::P1_0,PlatformPara::P2_0,PlatformPara::P3_0,PlatformPara::P4_0,PlatformPara::P5_0,PlatformPara::P6_0;

    Eigen::Matrix<double,3,6> VP; //上铰点的速度
    Eigen::Vector3d V(Vx,Vy,Vz); //上平台速度
    Eigen::Vector3d W(Wx,Wy,Wz); //平台角速度
    for(int i=0;i<6;i++)
    {
        VP.col(i)=V+W.cross(R*P0.col(i));  //求出6个上铰点的速度矢量
    }

    //2. 求出6个电动缸单位向量ni
    Eigen::Matrix<double,3,6> P;//上铰点在定坐标系中的坐标
    Eigen::Vector3d t(PosX,PosY,PosZ);
    Eigen::Vector3d T=t+PlatformPara::T0;
    Eigen::Matrix<double,3,6> TT;
    TT<<T,T,T,T,T,T;
    P=TT+R*P0;

    for(int i=0;i<6;i++)
    {
        double l=sqrt(P.col(i).transpose()*P.col(i)); //第i根电动缸长度
        P.col(i)=P.col(i)/l;  //此时求得的P即为6根电动缸的单位向量
    }

    //3. ni.Vpi即为6根电动缸的伸缩速度标量
    QVector<double> ScalarVP;  //速度标量
    for(int i=0;i<6;i++)
    {
        double v=P.col(i).dot(VP.col(i));
        ScalarVP.push_back(v);
    }
    return ScalarVP;
}
