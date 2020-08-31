#include "savedata.h"
#include <QCoreApplication>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

//连接到数据库
static void connectToDatabase()
{
    QString DBPath=QCoreApplication::applicationDirPath()+"//HumanData.db";
    QSqlDatabase db = QSqlDatabase::database(DBPath);
    if(!db.isValid())
    {
        db=QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(DBPath);
    }
    bool OpenSucceed=db.open();
    if(!OpenSucceed)
    {
        qDebug()<<"Create DB error:"<<db.lastError();
    }
}


//创建数据表
static void createTable()
{
    if(!QSqlDatabase::database(QCoreApplication::applicationDirPath()+"//HumanData.db").tables().contains("User"))
    {
        QSqlQuery query;
        if(!query.exec("CREATE TABLE IF NOT EXISTS 'User' ("
                       "'PersonID' INT NOT NULL AUTO_INCREMENT,"
                       "'Name' TEXT NOT NULL,"
                       "'Age' TEXT NOT NULL,"
                       "'Sex' TEXT NOT NULL,"
                       "PRIMARY KEY('PersonID')"
                       ")"))
        {
            qDebug()<<"Create table User failed";
        }
    }

    if(!QSqlDatabase::database(QCoreApplication::applicationDirPath()+"//HumanData.db").tables().contains("Waist"))
    {
        QSqlQuery query;
        if(!query.exec("CREATE TABLE IF NOT EXISTS 'Waist' ("
                       "'PersonID' INT NOT NULL,"
                       "'TIME' TEXT NOT NULL,"
                       "'AccX' TEXT NOT NULL,"
                       "'AccY' TEXT NOT NULL,"
                       "'AccZ' TEXT NOT NULL,"
                       "'WX' TEXT NOT NULL,"
                       "'WY' TEXT NOT NULL,"
                       "'WZ' TEXT NOT NULL,"
                       "'AngleX' TEXT NOT NULL,"
                       "'AngleY' TEXT NOT NULL,"
                       "'AngleZ' TEXT NOT NULL,"
                       "FOREIGN KEY('PersonID') REFERENCES 'User'('PersonID')"
                       ")"))
        {
            qDebug()<<"Create table Waist failed";
        }
    }

    if(!QSqlDatabase::database(QCoreApplication::applicationDirPath()+"//HumanData.db").tables().contains("LeftFoot"))
    {
        QSqlQuery query;
        if(!query.exec("CREATE TABLE IF NOT EXISTS 'LeftFoot' ("
                       "'PersonID' INT NOT NULL,"
                       "'TIME' TEXT NOT NULL,"
                       "'AccX' TEXT NOT NULL,"
                       "'AccY' TEXT NOT NULL,"
                       "'AccZ' TEXT NOT NULL,"
                       "'WX' TEXT NOT NULL,"
                       "'WY' TEXT NOT NULL,"
                       "'WZ' TEXT NOT NULL,"
                       "'AngleX' TEXT NOT NULL,"
                       "'AngleY' TEXT NOT NULL,"
                       "'AngleZ' TEXT NOT NULL,"
                       "FOREIGN KEY('PersonID') REFERENCES 'User'('PersonID')"
                       ")"))
        {
            qDebug()<<"Create table LeftFoot failed";
        }
    }

    if(!QSqlDatabase::database(QCoreApplication::applicationDirPath()+"//HumanData.db").tables().contains("RightFoot"))
    {
        QSqlQuery query;
        if(!query.exec("CREATE TABLE IF NOT EXISTS 'RightFoot' ("
                       "'PersonID' INT NOT NULL,"
                       "'TIME' TEXT NOT NULL,"
                       "'AccX' TEXT NOT NULL,"
                       "'AccY' TEXT NOT NULL,"
                       "'AccZ' TEXT NOT NULL,"
                       "'WX' TEXT NOT NULL,"
                       "'WY' TEXT NOT NULL,"
                       "'WZ' TEXT NOT NULL,"
                       "'AngleX' TEXT NOT NULL,"
                       "'AngleY' TEXT NOT NULL,"
                       "'AngleZ' TEXT NOT NULL,"
                       "FOREIGN KEY('PersonID') REFERENCES 'User'('PersonID')"
                       ")"))
        {
            qDebug()<<"Create table RightFoot failed";
        }
    }
}



SaveData::SaveData(QObject* parent):
    QSqlTableModel((parent))
{
    connectToDatabase();
    createTable();
}

void SaveData::saveToDataBase(QVector<QString> data)
{

}






