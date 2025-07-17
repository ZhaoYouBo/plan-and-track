#ifndef HABITSTATUSDELEGATE_H
#define HABITSTATUSDELEGATE_H

#include <QStyledItemDelegate>

class HabitStatusDelegate : public QStyledItemDelegate
{
public:
    explicit HabitStatusDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // HABITSTATUSDELEGATE_H
