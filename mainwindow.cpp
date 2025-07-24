#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addtaskdialog.h"
#include "addhabitdialog.h"
#include "utils.h"
#include "delegates/datedelegate.h"
#include "delegates/habitfrequencydelegate.h"
#include "delegates/taskstatusdelegate.h"
#include "delegates/habitstatusdelegate.h"
#include "delegates/planstatusdelegate.h"
#include "delegates/plannamedelegate.h"

#include <QDate>
#include <QFile>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>
#include <QCoreApplication>
#include <QHeaderView>
#include <QToolTip>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtMath>
#include <limits>
#include <QActionGroup>
#include <QDir>
#include <QFileInfoList>
#include <QSettings>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dbManager("D:/Collection/Sqlite/PlanManage.db")
    , m_tooltip(nullptr)
{
    ui->setupUi(this);

    this->showMaximized();

    this->setWindowTitle("计划管理软件");
    this->setWindowIcon(QIcon(":/assets/resource/icon.ico"));
    initChartTask();
    initChartHabit();
    init();
    createThemeMenu();
    QDir configDir("Config");
    if(!configDir.exists()) {
        QDir().mkdir("Config");
    }
    QSettings settings("Config/config.ini", QSettings::IniFormat);
    QString lastTheme = settings.value("theme").toString();
    bool found = false;
    if (!lastTheme.isEmpty()) {
        changeTheme(lastTheme);

        QList<QAction*> actions = themeGroup->actions();
        for (QAction* action : std::as_const(actions)) {
            if (action->text() == lastTheme) {
                action->setChecked(true);
                found = true;
                break;
            }
        }
    }

    if (!found && !themeGroup->actions().isEmpty()) {
        const QList<QAction*> &actions = themeGroup->actions();
        QAction *firstAction = actions.first();
        firstAction->setChecked(true);
        changeTheme(firstAction->text());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    m_modelTask = new TaskModel(this);
    m_modelHabit = new HabitModel(this);
    m_modelPlan = new PlanModel(this);

    m_modelTask->setHorizontalHeaderLabels({"ID", "任务名称", "创建日期", "截止日期", "完成日期", "完成状态"});
    m_modelHabit->setHorizontalHeaderLabels({"ID", "习惯名称", "创建日期", "习惯频率", "完成状态"});
    m_modelPlan->setHorizontalHeaderLabels({"类型", "计划名称", "完成状态"});

    ui->tableView_task->setModel(m_modelTask);
    ui->tableView_habit->setModel(m_modelHabit);
    ui->tableView_plan->setModel(m_modelPlan);


    ui->tableView_task->setWordWrap(true);
    ui->tableView_habit->setWordWrap(true);
    ui->tableView_plan->setWordWrap(true);

    ui->tableView_task->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_habit->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_plan->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableView_task->setItemDelegateForColumn(3, new DateDelegate(ui->tableView_task));
    ui->tableView_task->setItemDelegateForColumn(5, new TaskStatusDelegate(ui->tableView_task));
    ui->tableView_habit->setItemDelegateForColumn(2, new DateDelegate(ui->tableView_habit));
    ui->tableView_habit->setItemDelegateForColumn(3, new HabitFrequencyDelegate(ui->tableView_habit));
    ui->tableView_habit->setItemDelegateForColumn(4, new HabitStatusDelegate(ui->tableView_habit));
    PlanNameDelegate *planNameDelegate = new PlanNameDelegate(&m_dbManager, ui->tableView_plan, this);
    ui->tableView_plan->setItemDelegateForColumn(1, planNameDelegate);
    ui->tableView_plan->setItemDelegateForColumn(2, new PlanStatusDelegate(ui->tableView_plan));

    connect(m_modelTask, &TaskModel::dataChanged, this, &MainWindow::onTableViewTaskDataChanged);
    connect(m_modelHabit, &HabitModel::dataChanged, this, &MainWindow::onTableViewHabitDataChanged);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::saveData);
    connect(ui->action_addTask, &QAction::triggered, this, &MainWindow::addTask);
    connect(ui->action_addHabit, &QAction::triggered, this, &MainWindow::addHabit);
    connect(ui->action_deletePlan, &QAction::triggered, this, &MainWindow::deletePlan);
    connect(ui->action_addPlan, &QAction::triggered, this, &MainWindow::insertPlan);

    QStringList taskStatuses = Utils::taskStatusList();
    taskStatuses.insert(0, "全部");
    ui->comboBox_task->addItems(taskStatuses);
    QStringList habitStatuses = Utils::habitStatusList();
    habitStatuses.insert(0, "全部");
    ui->comboBox_habit->addItems(habitStatuses);
    ui->comboBox_task->setCurrentText("进行中");
    ui->comboBox_habit->setCurrentText("进行中");

    emit ui->calendarWidget->clicked(QDate::currentDate());

    adjustTableWidth(ui->tableView_plan);
    adjustTableWidth(ui->tableView_task);
    adjustTableWidth(ui->tableView_habit);

    ui->tableView_plan->viewport()->installEventFilter(this);
    ui->tableView_task->viewport()->installEventFilter(this);
    ui->tableView_habit->viewport()->installEventFilter(this);

    ui->tableView_habit->setColumnHidden(0, true);
    ui->tableView_task->setColumnHidden(0, true);
}

void MainWindow::initChartTask()
{
    m_chartViewPlan = new QChartView(this);
    m_chartViewPlan->setRenderHint(QPainter::Antialiasing);

    QChart *chart = new QChart();
    chart->setTitle("任务完成情况");

    QDate startDate = QDate::currentDate().addDays(-13);
    QDate endDate = QDate::currentDate();
    QMap<QDate,double> resultDate = m_dbManager.getPlanNumberByDate(startDate, endDate);

    if (!resultDate.empty())
    {
        QLineSeries * series = new QLineSeries();

        for (auto it = resultDate.begin(); it != resultDate.end(); ++it) {
            QDateTime dateTime(it.key(), QTime(0, 0, 0));
            series->append(dateTime.toMSecsSinceEpoch(), qRound(it.value() * 100));
        }

        series->setPointsVisible(true);
        series->setPointLabelsVisible(false);

        connect(series, &QLineSeries::hovered, this, &MainWindow::onChartHovered);

        QDateTimeAxis *axisX = new QDateTimeAxis();
        axisX->setFormat("MM-dd");
        axisX->setTitleText("日期");
        axisX->setTickCount(16);

        QDateTime startDateTime(startDate, QTime(0, 0, 0));
        QDateTime endDateTime(endDate, QTime(0, 0, 0));
        QDateTime startDateTimeWithMargin = startDateTime.addDays(-1);
        QDateTime endDateTimeWithMargin = endDateTime.addDays(1);
        axisX->setRange(startDateTimeWithMargin, endDateTimeWithMargin);

        QValueAxis *axisY = new QValueAxis();
        axisY->setLabelFormat("%.0f%");
        axisY->setTitleText("完成率");
        axisY->setTickCount(6);
        axisY->setRange(0, 100);

        chart->addSeries(series);
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    else
    {
        chart->addSeries(new QLineSeries());
        chart->setTitle("暂无数据");
    }
    chart->legend()->hide();

    m_chartViewPlan->setChart(chart);

    ui->horizontalLayout->insertWidget(1, m_chartViewPlan);
}

void MainWindow::initChartHabit()
{
    // 创建图表视图
    m_chartViewHabit = new QChartView(this);
    m_chartViewHabit->setRenderHint(QPainter::Antialiasing);

    // 获取数据并创建图表
    QList<HabitData> habitDataList = m_dbManager.getHabitByStatus(1);
    QChart *chart = new QChart();

    // 使用公共方法刷新图表
    refreshHabitChart(chart, habitDataList);

    // 设置图表到视图并添加到布局
    m_chartViewHabit->setChart(chart);
    ui->horizontalLayout_4->insertWidget(1, m_chartViewHabit);
}


void MainWindow::createThemeMenu()
{
    themeGroup = new QActionGroup(this);

    QDir qssDir(":/assets/resources/");
    QStringList nameFilters;
    nameFilters << "*.qss";
    QFileInfoList qssFiles = qssDir.entryInfoList(nameFilters, QDir::Files);

    for (const QFileInfo &fileInfo : std::as_const(qssFiles)) {
        QString theme = fileInfo.baseName();
        QAction *action = new QAction(theme, this);
        action->setCheckable(true);
        themeGroup->addAction(action);
        ui->menu_theme->addAction(action);
        connect(action, &QAction::triggered, this, [this, theme]() {
            this->changeTheme(theme);
        });
    }
}


void MainWindow::changeTheme(const QString &themeName)
{
    QString qssPath = QString(":/assets/resources/%1.qss").arg(themeName);
    QFile file(qssPath);
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);

        QSettings settings("Config/config.ini", QSettings::IniFormat);
        settings.setValue("theme", themeName);
    }
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        if (obj == ui->tableView_plan->viewport())
        {
            adjustTableWidth(ui->tableView_plan);
        }
        else if (obj == ui->tableView_task->viewport())
        {
            adjustTableWidth(ui->tableView_task);
        }
        else if (obj == ui->tableView_habit->viewport())
        {
            adjustTableWidth(ui->tableView_habit);
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::saveData()
{
    QAbstractItemModel* model = ui->tableView_plan->model();
    if (!model) {
        qDebug() << "Model is not set!";
        return;
    }

    QDate selectedDate = ui->calendarWidget->selectedDate();

    int rowCount = model->rowCount();

    m_dbManager.clearDailyPlanByDate(selectedDate);

    for (int row = 1; row <= rowCount; ++row) {
        QString type = model->index(row - 1, 0).data(Qt::DisplayRole).toString();
        int indexId = row;
        QString name = model->index(row - 1, 1).data(Qt::DisplayRole).toString();
        int status = Utils::planStatusFromString(model->index(row - 1, 2).data(Qt::DisplayRole).toString());
        if (type == "习惯")
        {
            int habitId = m_dbManager.getHabitIdByName(name);
            m_dbManager.updateHabitPlan(indexId, name, status, habitId, selectedDate);
        }
        else if (type == "任务")
        {
            int taskId = m_dbManager.getTaskIdByName(name);
            m_dbManager.updateTaskPlan(indexId, name, status, taskId, selectedDate);
        }
        else
        {
            continue;
        }
    }
    on_calendarWidget_clicked(ui->calendarWidget->selectedDate());

    on_comboBox_habit_currentIndexChanged(1);
}

void MainWindow::adjustTableWidth(QTableView *tableView)
{
    if (!tableView || !tableView->model())
        return;

    tableView->resizeColumnsToContents();

    int totalWidth = 0;
    int columnCount = tableView->horizontalHeader()->count();

    for (int i = 0; i < columnCount; ++i)
    {
        totalWidth += tableView->horizontalHeader()->sectionSize(i);
    }

    int viewportWidth = tableView->viewport()->width();

    if (totalWidth > 0 && totalWidth != viewportWidth)
    {
        double factor = static_cast<double>(viewportWidth) / totalWidth;

        for (int i = 0; i < columnCount; ++i)
        {
            int originalWidth = tableView->horizontalHeader()->sectionSize(i);
            int newWidth = static_cast<int>(originalWidth * factor);
            tableView->setColumnWidth(i, newWidth);
        }
    }
    else
    {
        return;
    }

    int adjustedTotalWidth = 0;
    for (int i = 0; i < columnCount; ++i)
    {
        adjustedTotalWidth += tableView->columnWidth(i);
    }

    int delta = viewportWidth - adjustedTotalWidth;
    if (delta != 0 && columnCount > 0)
    {
        int lastColumnIndex = columnCount - 1;
        tableView->setColumnWidth(lastColumnIndex, tableView->columnWidth(lastColumnIndex) + delta);
    }
}


void MainWindow::addTask()
{
    AddTaskDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        TaskData taskData;
        taskData.name = dialog.getTaskName();
        taskData.dueDate = dialog.getDueDate();

        m_dbManager.addTask(taskData);

        on_comboBox_task_currentIndexChanged(ui->comboBox_task->currentIndex());
    }
}

void MainWindow::onTableViewTaskDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!roles.contains(Qt::EditRole)) return;

    int row = topLeft.row();
    int col = topLeft.column();

    const QAbstractItemModel *model = topLeft.model();

    QModelIndex idIndex = model->index(row, 0);
    QString taskIdStr = model->data(idIndex, Qt::DisplayRole).toString();
    bool ok;
    int taskId = taskIdStr.toInt(&ok);

    if (!ok) {
        qDebug() << "Invalid task ID at row" << row;
        return;
    }

    QVariant newValue = model->data(topLeft, Qt::EditRole);

    switch (col) {
    case 1:
        m_dbManager.updateTaskName(taskId, newValue.toString());
        break;
    case 3:
        m_dbManager.updateTaskDueDate(taskId, QDate::fromString(newValue.toString(), "yyyy年MM月dd日"));
        break;
    case 5:
        m_dbManager.updateTaskStatus(taskId, Utils::taskStatusFromString(newValue.toString()));
        break;
    default:
        qDebug() << "Uneditable column modified.";
        break;
    }
    on_comboBox_task_currentIndexChanged(ui->comboBox_task->currentIndex());
    on_calendarWidget_clicked(ui->calendarWidget->selectedDate());
}

void MainWindow::onTableViewHabitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!roles.contains(Qt::EditRole)) return;

    int row = topLeft.row();
    int col = topLeft.column();

    const QAbstractItemModel *model = topLeft.model();

    QModelIndex idIndex = model->index(row, 0);
    QString habitIdStr = model->data(idIndex, Qt::DisplayRole).toString();
    bool ok;
    int habitId = habitIdStr.toInt(&ok);

    if (!ok) {
        qDebug() << "Invalid habit ID at row" << row;
        return;
    }

    QVariant newValue = model->data(topLeft, Qt::EditRole);

    switch (col) {
    case 1:
        m_dbManager.updateHabitName(habitId, newValue.toString());
        break;
    case 2:
        m_dbManager.updateHabitCreatedDate(habitId, QDate::fromString(newValue.toString(), "yyyy年MM月dd日"));
        break;
    case 3:
        m_dbManager.updateHabitFrequency(habitId, newValue.toString());
        break;
    case 4:
        m_dbManager.updateHabitStatus(habitId, Utils::habitStatusFromString(newValue.toString()));
        break;
    default:
        qDebug() << "Uneditable column modified.";
        break;
    }
    on_comboBox_habit_currentIndexChanged(ui->comboBox_task->currentIndex());
    updateChartHabit();
}

void MainWindow::on_comboBox_task_currentIndexChanged(int index)
{
    m_modelTask->removeRows(0, m_modelTask->rowCount());

    QList<TaskData> taskDataList = m_dbManager.getTaskByStatus(index);

    for (const TaskData &taskData : std::as_const(taskDataList)) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(taskData.id)));
        items.append(new QStandardItem(taskData.name));
        items.append(new QStandardItem(taskData.createdDate.toString("yyyy年MM月dd日")));
        items.append(new QStandardItem(taskData.dueDate.toString("yyyy年MM月dd日")));
        items.append(new QStandardItem(taskData.completedDate.toString("yyyy年MM月dd日")));
        items.append(new QStandardItem(Utils::taskStatusToString(taskData.status)));

        for (int i = 0; i < items.size(); ++i) {
            if (i == 1) continue;
            items[i]->setTextAlignment(Qt::AlignCenter);
        }

        m_modelTask->appendRow(items);
    }

    PlanNameDelegate* planNameDelegate = qobject_cast<PlanNameDelegate*>(ui->tableView_plan->itemDelegateForColumn(1));
    if (planNameDelegate) {
        planNameDelegate->refreshTaskNames();
    }
    adjustTableWidth(ui->tableView_task);
}


void MainWindow::addHabit()
{
    AddHabitDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        HabitData habitData;
        habitData.name = dialog.getHabitName();
        habitData.target_frequency = dialog.getHabitFrequency();

        m_dbManager.addHabit(habitData);

        on_comboBox_habit_currentIndexChanged(ui->comboBox_habit->currentIndex());
    }
}

void MainWindow::on_comboBox_habit_currentIndexChanged(int index)
{
    m_modelHabit->removeRows(0, m_modelHabit->rowCount());

    QList<HabitData> habitDataList;
    habitDataList = m_dbManager.getHabitByStatus(index);

    for (const HabitData &habitData : std::as_const(habitDataList)) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(habitData.id)));
        items.append(new QStandardItem(habitData.name));
        items.append(new QStandardItem(habitData.createdDate.toString("yyyy年MM月dd日")));
        items.append(new QStandardItem(habitData.target_frequency));
        items.append(new QStandardItem(Utils::habitStatusToString(habitData.status)));

        for (int i = 0; i < items.size(); ++i) {
            if (i == 1) continue;
            items[i]->setTextAlignment(Qt::AlignCenter);
        }

        m_modelHabit->appendRow(items);
    }

    adjustTableWidth(ui->tableView_habit);
}


void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
    updateChartTask(date);
    updateChartHabit();
    m_modelPlan->removeRows(0, m_modelPlan->rowCount());

    QList<PlanData> planDataList;
    planDataList = m_dbManager.getPlanByDate(date);
    bool needAdd = true;

    for (const PlanData &plan : std::as_const(planDataList))
    {
        if (plan.type == "习惯") needAdd = false;
        QList<QStandardItem*> items;
        items.append(new QStandardItem(plan.type));
        items.append(new QStandardItem(plan.name));
        items.append(new QStandardItem(Utils::planStatusToString(plan.status)));

        items[0]->setTextAlignment(Qt::AlignCenter);

        m_modelPlan->appendRow(items);
    }

    if (!needAdd)
    {
        return;
    }

    QList<HabitData> habitDataList;
    habitDataList = m_dbManager.getHabitByStatus(1);

    for (const HabitData &habit : std::as_const(habitDataList))
    {
        if (habit.createdDate > date)
            continue;

        QString frequency = habit.target_frequency;
        bool shouldAdd = false;

        if (frequency == "每日一次")
        {
            shouldAdd = true;
        }
        else if (frequency.startsWith("每二日一次"))
        {
            int daysDiff = habit.createdDate.daysTo(date);
            if (daysDiff % 2 == 0)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每三日一次"))
        {
            int daysDiff = habit.createdDate.daysTo(date);
            if (daysDiff % 3 == 0)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周一"))
        {
            if (date.dayOfWeek() == 1)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周二"))
        {
            if (date.dayOfWeek() == 2)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周三"))
        {
            if (date.dayOfWeek() == 3)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周四"))
        {
            if (date.dayOfWeek() == 4)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周五"))
        {
            if (date.dayOfWeek() == 5)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周六"))
        {
            if (date.dayOfWeek() == 6)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周周日"))
        {
            if (date.dayOfWeek() == 7)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周工作日"))
        {
            if (date.dayOfWeek() >= 1 && date.dayOfWeek() <= 5)
                shouldAdd = true;
        }
        else if (frequency.startsWith("每周休息日"))
        {
            if (date.dayOfWeek() == 6 || date.dayOfWeek() == 7)
                shouldAdd = true;
        }

        if (shouldAdd)
        {
            QList<QStandardItem*> items;
            items.append(new QStandardItem("习惯"));
            items.append(new QStandardItem(habit.name));
            items.append(new QStandardItem(Utils::planStatusToString(habit.status)));
            items[0]->setTextAlignment(Qt::AlignCenter);

            m_modelPlan->appendRow(items);
        }
    }
}


void MainWindow::deletePlan()
{
    QTableView *tableView = ui->tableView_plan;
    QAbstractItemModel *model = tableView->model();
    QItemSelectionModel *selectionModel = tableView->selectionModel();

    if (!model || !selectionModel) {
        qDebug() << "Model or selection model is null!";
        return;
    }

    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.isEmpty()) {
        qDebug() << "No row selected!";
        return;
    }

    for (int i = selectedRows.size() - 1; i >= 0; --i) {
        int row = selectedRows[i].row();
        model->removeRow(row);
    }
}


void MainWindow::insertPlan()
{
    QTableView *tableView = ui->tableView_plan;
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(tableView->model());

    if (!model) {
        qDebug() << "Model is not a QStandardItemModel!";
        return;
    }

    QList<QStandardItem*> items;
    items.append(new QStandardItem("任务"));
    items.append(new QStandardItem(""));
    items.append(new QStandardItem("进行中"));

    model->appendRow(items);
}

void MainWindow::refreshHabitChart(QChart *chart, const QList<HabitData> &habitDataList)
{
    // 清除现有的系列和坐标轴
    chart->removeAllSeries();

    // 删除所有现有坐标轴
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
        chart->removeAxis(axis);
        delete axis;
    }

    if (!habitDataList.empty())
    {
        // 创建数据集
        QBarSet *set1 = new QBarSet("总次数");
        QBarSet *set2 = new QBarSet("连续次数");

        set1->setLabelColor(Qt::black);
        set2->setLabelColor(Qt::black);

        QStringList categories;
        int maxValue = 0;

        // 填充数据
        for (const HabitData &habit : habitDataList)
        {
            int maxTimes = m_dbManager.getHabitMaxTimes(habit);
            int allTimes = m_dbManager.getHabitTimes(habit);

            *set1 << allTimes;
            *set2 << maxTimes;
            categories << habit.name;

            maxValue = qMax(maxValue, qMax(maxTimes, allTimes));
        }

        // 创建系列
        QBarSeries *series = new QBarSeries();
        series->append(set1);
        series->append(set2);
        series->setLabelsVisible(true);
        series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

        chart->addSeries(series);

        // 创建坐标轴
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        axisX->setTitleText("习惯名称");

        // 计算纵坐标上限值
        int upperBound;
        if (maxValue == 0) {
            // 如果maxValue是0，则上限设为10（因为0是整十数，0+10=10）
            upperBound = 10;
        } else if (maxValue % 10 == 0) {
            // 如果maxValue是整十数，则上限为maxValue + 10
            upperBound = maxValue + 10;
        } else {
            // 如果maxValue不是整十数，则向上取整到最近的整十数
            upperBound = (maxValue / 10 + 1) * 10;
        }

        QValueAxis *axisY = new QValueAxis();
        axisY->setLabelFormat("%.0f");
        axisY->setTitleText("次数");
        axisY->setTickCount(6);
        axisY->setRange(0, upperBound);  // 使用计算得到的上限值

        // 添加坐标轴并关联系列
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        // 设置图表样式
        chart->setTitle("习惯完成情况");
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
    }
    else
    {
        // 空数据处理
        QBarSeries *emptySeries = new QBarSeries();
        QBarSet *emptySet = new QBarSet("无数据");
        emptySeries->append(emptySet);
        chart->addSeries(emptySeries);

        chart->setTitle("暂无数据");
        chart->legend()->setVisible(false);
    }
}

void MainWindow::updateChartTask(const QDate &date)
{
    QChart *chartPlan = m_chartViewPlan->chart();
    chartPlan->removeAllSeries();

    QDate startDate = date.addDays(-13);
    QMap<QDate, double> resultDate = m_dbManager.getPlanNumberByDate(startDate, date);

    QDateTimeAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    foreach (QAbstractAxis* ax, chartPlan->axes(Qt::Horizontal)) {
        axisX = qobject_cast<QDateTimeAxis*>(ax);
    }
    foreach (QAbstractAxis* ay, chartPlan->axes(Qt::Vertical)) {
        axisY = qobject_cast<QValueAxis*>(ay);
    }

    if (!axisX) {
        axisX = new QDateTimeAxis();
        chartPlan->addAxis(axisX, Qt::AlignBottom);
    }
    if (!axisY) {
        axisY = new QValueAxis();
        chartPlan->addAxis(axisY, Qt::AlignLeft);
    }

    axisX->setFormat("MM-dd");
    axisX->setTitleText("日期");
    axisX->setTickCount(16);
    axisX->setRange(QDateTime(date.addDays(-14), QTime(0,0,0)), QDateTime(date.addDays(1), QTime(0,0,0)));

    axisY->setLabelFormat("%.0f%");
    axisY->setTitleText("完成率");
    axisY->setTickCount(6);
    axisY->setRange(0, 100);

    chartPlan->removeAllSeries();

    if (!resultDate.empty()) {
        QLineSeries *series = new QLineSeries();
        for (auto it = resultDate.begin(); it != resultDate.end(); ++it) {
            series->append(QDateTime(it.key(), QTime(0,0,0)).toMSecsSinceEpoch(), qRound(it.value() * 100));
        }

        series->setPointsVisible(true);
        series->setPointLabelsVisible(false);

        connect(series, &QLineSeries::hovered, this, &MainWindow::onChartHovered);

        chartPlan->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        chartPlan->setTitle("任务完成情况");
    } else {
        chartPlan->addSeries(new QLineSeries());
        chartPlan->setTitle("暂无数据");
    }

    chartPlan->legend()->hide();
}

void MainWindow::updateChartHabit()
{
    // 获取现有图表和最新数据
    QChart *chart = m_chartViewHabit->chart();
    QList<HabitData> habitDataList = m_dbManager.getHabitByStatus(1);

    // 使用公共方法刷新图表
    refreshHabitChart(chart, habitDataList);
}

void MainWindow::onChartHovered(const QPointF &point, bool state)
{
    if (state) {
        QChart *chart = m_chartViewPlan->chart();
        if (!chart || chart->series().isEmpty()) {
            return;
        }

        QList<QAbstractSeries*> seriesList = chart->series();
        QLineSeries *series = qobject_cast<QLineSeries*>(seriesList.first());
        if (!series) {
            return;
        }

        QPointF closestPoint;
        double minDistance = std::numeric_limits<double>::max();
        bool foundClosest = false;

        for (int i = 0; i < series->count(); ++i) {
            QPointF dataPoint = series->at(i);
            double distance = qSqrt(qPow(point.x() - dataPoint.x(), 2) + qPow(point.y() - dataPoint.y(), 2));

            if (distance < 0.5 * 24 * 60 * 60 * 1000) {
                if (distance < minDistance) {
                    minDistance = distance;
                    closestPoint = dataPoint;
                    foundClosest = true;
                }
            }
        }

        if (foundClosest) {
            QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(closestPoint.x());
            QString dateStr = dateTime.toString("yyyy年MM月dd日");
            QString completionRate = QString::number(closestPoint.y(), 'f', 1) + "%";

            QString tooltipText = QString("日期: %1\n完成率: %2").arg(dateStr, completionRate);

            QPoint globalPos = m_chartViewPlan->mapToGlobal(m_chartViewPlan->mapFromScene(
                m_chartViewPlan->chart()->mapToPosition(closestPoint)));

            QToolTip::showText(globalPos, tooltipText, m_chartViewPlan);
        }
    } else {
        QToolTip::hideText();
    }
}


void MainWindow::on_comboBox_type_currentTextChanged(const QString &arg1)
{
    QDate date = ui->calendarWidget->selectedDate();
    on_calendarWidget_clicked(date);
}

