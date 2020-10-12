#ifndef KINEMATICS_H
#define KINEMATICS_H

//此文件主要包含机构运动学逆解
#include <QVector>

#include "Eigen/Eigen/Core"
#include "Eigen/Eigen/Geometry"
#include "config.h"

extern Eigen::Matrix3d RotationMatrix(double Yaw,double Pitch,double Roll);
extern QVector<double> PositionReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll);
extern QVector<double> SpeedReverse(double PosX,double PosY,double PosZ,double Yaw,double Pitch,double Roll,
                                    double Vx,double Vy,double Vz,double Wx,double Wy,double Wz);


#endif // KINEMATICSS_H
