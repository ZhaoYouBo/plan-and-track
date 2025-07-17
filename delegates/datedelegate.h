#ifndef DATEDELEGATE_H
#define DATEDELEGATE_H

#include <QStyledItemDelegate>

class DateDelegate : public QStyledItemDelegate
{
public:
    explicit DateDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // DATEDELEGATE_H
