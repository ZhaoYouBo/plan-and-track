#include "planmodel.h"

PlanModel::PlanModel(QObject *parent)
    : QStandardItemModel{parent}
{}

Qt::ItemFlags PlanModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    case 0:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    default:
        return QStandardItemModel::flags(index);
    }
}
