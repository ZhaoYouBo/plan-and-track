#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QStandardItemModel>

class TaskModel : public QStandardItemModel
{
public:
    explicit TaskModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // TASKMODEL_H
