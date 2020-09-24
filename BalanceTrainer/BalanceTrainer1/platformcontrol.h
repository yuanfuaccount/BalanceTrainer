#ifndef PLATFORMCONTROL_H
#define PLATFORMCONTROL_H

#include "motorcontrol.h"
#include <QThread>


//发送读取电机位置的命令的线程的执行函数的定义
class GetDataCmdThread:public QThread
{
    //Q_OBJECT
    /* *********************
     * 注：如果在类中添加Q_OBJECT,在使用到类的位置会报错undefined reference to vtable for GetDataCmdThread
     * 此时注释掉O_OBJECT就行了
     * 原因见https://blog.csdn.net/top_worker/article/details/44994741
     * **********************/
public:
    bool exit=false;

    GetDataCmdThread(QObject* par):QThread(par){}
    void run() override;
signals:
    void RcvSuccess();
};




extern void motorSpeedControl(QVector<double> Speed,QVector<double> BasePos,double time);
extern void PlatformSpeedControl(QVector<double> PlatformSpeed,QVector<uint32_t> BasePos,double time);
extern void PlatFormPositionControl(QVector<double> Position,ushort time);
extern void ReadPathData(QVector<QVector<double>>& PathData);
extern void ExecPath(QVector<double>& PathData);



#endif // PLATFORMCONTROL_H
