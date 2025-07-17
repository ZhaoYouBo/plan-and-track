#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"
#include "models/habitmodel.h"
#include "models/taskmodel.h"
#include "models/planmodel.h"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <QChartView>
#include <QToolTip>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void on_pushButton_add_task_clicked();
    void onTableViewTaskDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onTableViewHabitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    void on_comboBox_task_currentIndexChanged(int index);

    void on_pushButton_add_habit_clicked();

    void on_comboBox_habit_currentIndexChanged(int index);

    void on_calendarWidget_clicked(const QDate &date);

    void on_pushButton_savePlan_clicked();

    void on_pushButton_saveSummary_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_insert_clicked();

private slots:
    void onChartHovered(const QPointF &point, bool state);

    void on_comboBox_type_currentTextChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    Database m_dbManager;
    TaskModel* m_modelTask;
    HabitModel* m_modelHabit;
    PlanModel* m_modelPlan;
    QChartView *m_chartViewPlan;
    QToolTip *m_tooltip;
    QActionGroup *themeGroup; // 主题分组，便于同步菜单选中项
    void init();
    void initChart();
    void saveData();
    void adjustTableWidth(QTableView *tableView);
    void createThemeMenu();
    void changeTheme(const QString &themeName);
};
#endif // MAINWINDOW_H
