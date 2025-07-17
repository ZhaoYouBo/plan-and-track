#include "taskmodel.h"

TaskModel::TaskModel(QObject *parent)
    : QStandardItemModel{parent}
{}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const {
    switch (index.column()) {
    case 0:
    case 2:
    case 4:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    default:
        return QStandardItemModel::flags(index);
    }
}
