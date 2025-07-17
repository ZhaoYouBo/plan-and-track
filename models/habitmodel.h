#ifndef HABITMODEL_H
#define HABITMODEL_H

#include <QStandardItemModel>

class HabitModel : public QStandardItemModel
{
public:
    explicit HabitModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // HABITMODEL_H
