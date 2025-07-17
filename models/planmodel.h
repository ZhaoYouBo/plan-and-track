#ifndef PLANMODEL_H
#define PLANMODEL_H

#include <QStandardItemModel>

class PlanModel : public QStandardItemModel
{
public:
    explicit PlanModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // PLANMODEL_H
