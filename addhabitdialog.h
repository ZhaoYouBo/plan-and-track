#ifndef ADDHABITDIALOG_H
#define ADDHABITDIALOG_H

#include <QDialog>

namespace Ui {
class AddHabitDialog;
}

class AddHabitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddHabitDialog(QWidget *parent = nullptr);
    ~AddHabitDialog();

    QString getHabitName();
    QString getHabitFrequency();

private:
    Ui::AddHabitDialog *ui;
};

#endif // ADDHABITDIALOG_H
