#include "habitfrequencydelegate.h"
#include "../utils.h"
#include <QComboBox>

HabitFrequencyDelegate::HabitFrequencyDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}


QWidget *HabitFrequencyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(Utils::habitFrequencyList());
    return editor;
}

void HabitFrequencyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value);
    if (idx >= 0) {
        comboBox->setCurrentIndex(idx);
    }
}

void HabitFrequencyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void HabitFrequencyDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
