#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>

namespace Ui {
class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget *parent = nullptr);
    ~AddTaskDialog();

    QString getTaskName();
    QDate getDueDate();

private:
    Ui::AddTaskDialog *ui;
};

#endif // ADDTASKDIALOG_H
