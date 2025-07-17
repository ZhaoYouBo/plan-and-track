#include "habitmodel.h"

HabitModel::HabitModel(QObject *parent)
    : QStandardItemModel{parent}
{}

Qt::ItemFlags HabitModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    case 0:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    default:
        return QStandardItemModel::flags(index);
    }
}
