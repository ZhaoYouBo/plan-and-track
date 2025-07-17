#include "plannamedelegate.h"

#include <QComboBox>

PlanNameDelegate::PlanNameDelegate(Database* dbManager, QTableView *tableView, QObject *parent)
    : QStyledItemDelegate{parent}
    , m_dbManager(dbManager)
    , m_tableView(tableView)
{
    refreshTaskNames();
}

QWidget *PlanNameDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QModelIndex typeIndex = index.sibling(index.row(), 0);
    QString type = typeIndex.data(Qt::DisplayRole).toString();

    if (type == "习惯") {
        return nullptr;
    }
    else {
        QComboBox *editor = new QComboBox(parent);
        editor->addItems(m_taskNames);
        return editor;
    }
}

void PlanNameDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int idx = comboBox->findText(value);
    if (idx >= 0) {
        comboBox->setCurrentIndex(idx);
    }
}

void PlanNameDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void PlanNameDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void PlanNameDelegate::refreshTaskNames()
{
    m_taskNames.clear();
    QList<TaskData> tasks = m_dbManager->getTaskByStatus(1);
    for (const TaskData &task : std::as_const(tasks)) {
        m_taskNames << task.name;
    }
}
