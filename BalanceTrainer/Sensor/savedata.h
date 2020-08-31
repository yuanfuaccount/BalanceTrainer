#ifndef SAVEDATA_H
#define SAVEDATA_H
#include <QSqlTableModel>
#include <QSqlDatabase>


class SaveData: public QSqlTableModel
{
    Q_OBJECT
public:
    SaveData(QObject* parent);
    void saveToDataBase(QVector<QString> data);
};

#endif // SAVEDATA_H
