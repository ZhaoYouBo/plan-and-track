#include "addhabitdialog.h"
#include "ui_addhabitdialog.h"
#include "utils.h"

AddHabitDialog::AddHabitDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddHabitDialog)
{
    ui->setupUi(this);

    ui->comboBox_frequency->addItems(Utils::habitFrequencyList());
}

AddHabitDialog::~AddHabitDialog()
{
    delete ui;
}

QString AddHabitDialog::getHabitName()
{
    return ui->lineEdit_name->text();
}

QString AddHabitDialog::getHabitFrequency()
{
    return ui->comboBox_frequency->currentText();
}
