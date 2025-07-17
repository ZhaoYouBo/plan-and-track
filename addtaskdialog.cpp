#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    QDate currentDate = QDate::currentDate();
    ui->dateEdit_due->setDate(currentDate);
    ui->dateEdit_due->setCalendarPopup(true);
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

QString AddTaskDialog::getTaskName()
{
    return ui->lineEdit_task->text();
}

QDate AddTaskDialog::getDueDate()
{
    return ui->dateEdit_due->date();
}
