#ifndef HABITFREQUENCYDELEGATE_H
#define HABITFREQUENCYDELEGATE_H

#include <QStyledItemDelegate>

class HabitFrequencyDelegate : public QStyledItemDelegate
{
public:
    explicit HabitFrequencyDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // HABITFREQUENCYDELEGATE_H
