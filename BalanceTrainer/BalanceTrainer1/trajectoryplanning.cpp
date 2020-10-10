#include "trajectoryplanning.h"
#include <QDebug>

TrajectoryPlanning::TrajectoryPlanning(QObject* parent)
{
    this->setParent(parent);
}

void TrajectoryPlanning::loadTrajectoryFile(QString fileName)
{
    if(!m_trajectoryPath.empty())
        m_trajectoryPath.clear();
    QFile* file=new QFile(fileName,this);
    if(file->open(QIODevice::ReadOnly))
    {
        QTextStream* stream=new QTextStream(file);
        QStringList datalist=stream->readAll().split('\n');
        for(int i=0;i<datalist.count()-1;i++)
        {
            QStringList row=datalist.at(i).split(',');
            m_trajectoryPath.push_back({row[0].toDouble(),row[1].toDouble(),row[2].toDouble(),row[3].toDouble(),row[4].toDouble(),row[5].toDouble()});
        }
        file->close();
    }
}

void TrajectoryPlanning::startTrajectroyPlanningSlot()
{
    emit startTrajectroyPlanningSignal(&m_trajectoryPath);
}


