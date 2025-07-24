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
    void onTableViewTaskDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onTableViewHabitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    void on_comboBox_task_currentIndexChanged(int index);

    void on_comboBox_habit_currentIndexChanged(int index);

    void on_calendarWidget_clicked(const QDate &date);

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
    QChartView *m_chartViewHabit;
    QToolTip *m_tooltip;
    QActionGroup *themeGroup;
    void init();
    void initChartTask();
    void initChartHabit();
    void saveData();
    void adjustTableWidth(QTableView *tableView);
    void createThemeMenu();
    void changeTheme(const QString &themeName);
    void addTask();
    void addHabit();
    void deletePlan();
    void insertPlan();
    void refreshHabitChart(QChart *chart, const QList<HabitData>& habitDataList);
    void updateChartTask(const QDate &date);
    void updateChartHabit();
};
#endif // MAINWINDOW_H
