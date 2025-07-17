#include "datedelegate.h"

#include <QDateEdit>

DateDelegate::DateDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}


QWidget *DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QDateEdit *editor = new QDateEdit(parent);
    editor->setCalendarPopup(true);
    editor->setDisplayFormat("yyyy年MM月dd日");
    return editor;
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    QVariant variantData = index.model()->data(index, Qt::EditRole);
    QDate date;
    if (variantData.canConvert<QString>()) {
        QString dateString = variantData.toString();
        date = QDate::fromString(dateString, "yyyy年MM月dd日");
    }
    else if (variantData.canConvert<QDate>()) {
        date = variantData.toDate();
    }

    if (!date.isValid()) {
        date = QDate::currentDate();
    }

    dateEdit->setDate(date);
}

void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    model->setData(index, dateEdit->date().toString("yyyy年MM月dd日"), Qt::EditRole);
}
