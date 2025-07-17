#include "habitstatusdelegate.h"
#include "..\utils.h"

#include <QComboBox>
#include <QPainter>

HabitStatusDelegate::HabitStatusDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

QWidget *HabitStatusDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(Utils::habitStatusList());
    return editor;
}

void HabitStatusDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value);
    if (idx >= 0) {
        comboBox->setCurrentIndex(idx);
    }
}

void HabitStatusDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void HabitStatusDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void HabitStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QMap<QString, QColor>& colorMap = Utils::statusColorMap();
    QString status = index.data(Qt::DisplayRole).toString();
    QColor bgColor = colorMap.value(status, option.palette.base().color());

    painter->fillRect(option.rect, bgColor);
    painter->setPen(Qt::black);
    painter->drawText(option.rect, Qt::AlignCenter, status);
}
