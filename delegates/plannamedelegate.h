#ifndef PLANNAMEDELEGATE_H
#define PLANNAMEDELEGATE_H

#include "../database.h"

#include <QStyledItemDelegate>
#include <QTableView>

class PlanNameDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PlanNameDelegate(Database *dbManager, QTableView *tableView, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void refreshTaskNames();

private:
    Database* m_dbManager;
    QTableView *m_tableView;
    mutable QStringList m_taskNames;
};

#endif // PLANNAMEDELEGATE_H
