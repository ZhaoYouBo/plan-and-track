#include "database.h"
#include <QSqlError>
#include <QSqlQuery>

Database::Database(const QString &dbName) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);
    if (!m_db.open()) {
        qDebug() << "Error: " << m_db.lastError().text();
    } else {
        qDebug() << "Database opened";
    }

    createTables();
}

void Database::createTables()
{
    QSqlQuery query;

    query.exec(
        "CREATE TABLE IF NOT EXISTS task ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "created_date DATE DEFAULT CURRENT_DATE, "
        "due_date DATE, "
        "completed_date DATE, "
        "status INTEGER DEFAULT 0"
        ")"
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS habits ("
        "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "created_date DATE DEFAULT CURRENT_DATE, "
        "target_frequency TEXT, "
        "status INTEGER DEFAULT 0); "
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS daily_plan ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "task_id INTEGER, "
        "habit_id INTEGER, "
        "plan_date DATE NOT NULL, "
        "plan_name TEXT, "
        "index_id INTEGER, "
        "status INTEGER DEFAULT 0, "
        "FOREIGN KEY (task_id) REFERENCES task(id) ON DELETE CASCADE, "
        "FOREIGN KEY (habit_id) REFERENCES habits(id) ON DELETE CASCADE, "
        "CHECK ( (task_id IS NOT NULL AND habit_id IS NULL) OR (task_id IS NULL AND habit_id IS NOT NULL) )"
        ")"
        );

    query.exec(
        "CREATE TABLE IF NOT EXISTS daily_review ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "type TEXT, "
        "review_date DATE NOT NULL, "
        "period_start DATE, "
        "period_end DATE, "
        "reflection TEXT, "
        "summary TEXT"
        ")"
        );
}

QList<TaskData> Database::getTaskByStatus(int status)
{
    QList<TaskData> taskDataList;
    QSqlQuery query;

    if (status == 0) {
        if (!query.exec("SELECT id, name, created_date, due_date, completed_date, status "
                        "FROM task"))
        {
            return taskDataList;
        }
    } else {
        status = status - 1;
        query.prepare("SELECT id, name, created_date, due_date, completed_date, status "
                      "FROM task "
                      "WHERE status = ?");
        query.addBindValue(status);
        if (!query.exec())
        {
            return taskDataList;
        }
    }

    while (query.next()) {
        TaskData taskData;
        taskData.id = query.value(0).toInt();
        taskData.name = query.value(1).toString();
        taskData.createdDate = query.value(2).toDate();
        taskData.dueDate = query.value(3).toDate();
        taskData.completedDate = query.value(4).toDate();
        taskData.status = query.value(5).toInt();
        taskDataList.append(taskData);
    }

    return taskDataList;
}

QList<HabitData> Database::getHabitByStatus(int status)
{
    QList<HabitData> habitDataList;
    QSqlQuery query;

    if (status == 0) {
        if (!query.exec("SELECT id, name, created_date, target_frequency, status "
                        "FROM habits"))
        {
            return habitDataList;
        }
    } else {
        status = status - 1;
        query.prepare("SELECT id, name, created_date, target_frequency, status "
                      "FROM habits "
                      "WHERE status = ?");
        query.addBindValue(status);
        if (!query.exec()) {
            return habitDataList;
        }
    }

    while(query.next()) {
        HabitData habitData;
        habitData.id = query.value(0).toInt();
        habitData.name = query.value(1).toString();
        habitData.createdDate = query.value(2).toDate();
        habitData.target_frequency = query.value(3).toString();
        habitData.status = query.value(4).toInt();

        habitDataList.append(habitData);
    }

    return habitDataList;
}

QList<PlanData> Database::getPlanByDate(const QDate &date)
{
    QList<PlanData> planDataList;

    QSqlQuery query("SELECT task_id, habit_id, plan_name, status "
                    "FROM daily_plan "
                    "WHERE plan_date = ?;");
    query.addBindValue(date);

    if (!query.exec()) {
        return planDataList;
    }

    while(query.next()) {
        PlanData planData;
        planData.name = query.value(2).toString();
        planData.status = query.value(3).toInt();

        if (!query.value(0).isNull()) {
            planData.type = "任务";
        } else if (!query.value(1).isNull()) {
            planData.type = "习惯";
        } else {
            continue;
        }

        planDataList.append(planData);
    }

    return planDataList;
}

QMap<QDate, double> Database::getPlanNumberByDate(const QDate &startDate, const QDate &endDate)
{
    QMap<QDate, double> resultData;

    QSqlQuery query;
    query.prepare("SELECT plan_date, COUNT(*), SUM(CASE WHEN status = 1 THEN 1 ELSE 0 END) "
                  "FROM daily_plan "
                  "WHERE plan_date BETWEEN :start AND :end "
                  "GROUP BY plan_date");
    query.bindValue(":start", startDate);
    query.bindValue(":end", endDate);

    if (!query.exec()) {
        return resultData;
    }

    while (query.next())
    {
        QDate date = query.value(0).toDate();
        int total = query.value(1).toInt();
        int completed = query.value(2).toInt();

        double ratio = (total > 0) ? static_cast<double>(completed) / total : 0.0;
        resultData.insert(date, ratio);
    }

    return resultData;
}

ReviewData Database::getReviewByDate(const QString& type, const QDate& startDate, const QDate& endDate)
{
    ReviewData reviewData;

    QSqlQuery query("SELECT reflection, summary "
                    "FROM daily_review "
                    "WHERE type = ? and period_start = ? and period_end = ?;");
    query.addBindValue(type);
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (!query.exec() || !query.next()) {
        return reviewData;
    }

    reviewData.reflection = query.value(0).toString();
    reviewData.summary = query.value(1).toString();

    return reviewData;
}

QList<ReviewData> Database::getReviewByType(const QString &type, const QDate &startDate, const QDate &endDate)
{
    QList<ReviewData> reviewData;
    QString searchType;
    QSqlQuery query;

    if(type == "日总结") {
        return reviewData;
    }
    else if (type == "周总结") {
        searchType = "日总结";
        query.prepare("SELECT reflection, summary "
                      "FROM daily_review "
                      "WHERE type = ? and period_start >= ? and period_end <= ?;");
        query.addBindValue(searchType);
        query.addBindValue(startDate);
        query.addBindValue(endDate);
    }
    else if (type == "月总结") {
        searchType = "周总结";
        query.prepare("SELECT reflection, summary "
                      "FROM daily_review "
                      "WHERE type = ? and period_start >= ? and period_end <= ?;");
        query.addBindValue(searchType);
        query.addBindValue(startDate);
        query.addBindValue(endDate);
    }
    else if (type == "年中总结") {
        searchType = "月总结";
        query.prepare("SELECT reflection, summary "
                      "FROM daily_review "
                      "WHERE type = ? and period_start >= ? and period_end <= ?;");
        query.addBindValue(searchType);
        query.addBindValue(startDate);
        query.addBindValue(endDate);
    }
    else if (type == "年终总结") {
        searchType = "年中总结";
        query.prepare("SELECT reflection, summary "
                      "FROM daily_review "
                      "WHERE (type = ? and period_start >= ? and period_end <= ?) "
                      "or (type = '月总结' and period_start >= ? and period_end <= ?);");
        query.addBindValue(searchType);
        query.addBindValue(startDate);
        query.addBindValue(endDate);
        query.addBindValue(QDate(endDate.year(), 7, 1));
        query.addBindValue(QDate(endDate.year(), 12, 31));
    }

    if (!query.exec()) {
        return reviewData;
    }

    while (query.next()) {
        ReviewData data;
        data.reflection = query.value(0).toString();
        data.summary = query.value(1).toString();
        reviewData.append(data);
    }

    return reviewData;
}

void Database::addTask(TaskData data)
{
    QSqlQuery query;
    query.prepare("INSERT INTO task (name, due_date) "
                  "VALUES (?, ?);");
    query.addBindValue(data.name);
    query.addBindValue(data.dueDate);
    if (query.exec()) {
    }
    else {
    }
}

void Database::addHabit(HabitData data)
{
    QSqlQuery query;
    query.prepare("INSERT INTO habits (name, target_frequency) "
                  "VALUES (?, ?);");
    query.addBindValue(data.name);
    query.addBindValue(data.target_frequency);
    if (query.exec()) {
    }
    else {
    }
}

void Database::updateTaskName(int id, const QString &name)
{
    QSqlQuery query;
    query.prepare("UPDATE task "
                  "SET name = ? "
                  "WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

void Database::updateTaskDueDate(int id, const QDate &date)
{
    QSqlQuery query;
    query.prepare("UPDATE task "
                  "SET due_date = ? "
                  "WHERE id = ?");
    query.addBindValue(date);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

void Database::updateTaskStatus(int id, int status)
{
    QSqlQuery query;
    switch (status) {
    case 0:
    case 2:
    case 4:
        query.prepare("UPDATE task "
                      "SET status = ?, completed_date = '' "
                      "WHERE id = ?");
        query.addBindValue(status);
        break;
    default:
        query.prepare("UPDATE task "
                      "SET status = ?, completed_date = ? "
                      "WHERE id = ?");
        query.addBindValue(status);
        query.addBindValue(QDate::currentDate());
        break;
    }
    query.addBindValue(id);

    if (!query.exec()) {
        return;
    }

    QSqlQuery planQuery;
    if (status == 1 || status == 3) {
        planQuery.prepare("UPDATE daily_plan "
                          "SET status = 1 "
                          "WHERE plan_date = ? AND task_id = ?");
        planQuery.addBindValue(QDate::currentDate());
        planQuery.addBindValue(id);
    } else if (status == 2 || status == 4) {
        planQuery.prepare("UPDATE daily_plan "
                          "SET status = 2 "
                          "WHERE plan_date = ? AND task_id = ?");
        planQuery.addBindValue(QDate::currentDate());
        planQuery.addBindValue(id);
    }

    if (!planQuery.exec()) {
    }
}

void Database::updateHabitName(int id, const QString &name)
{
    QSqlQuery query;
    query.prepare("UPDATE habits "
                  "SET name = ? "
                  "WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

void Database::updateHabitCreatedDate(int id, const QDate &date)
{
    QSqlQuery query;
    query.prepare("UPDATE habits "
                  "SET created_date = ? "
                  "WHERE id = ?");
    query.addBindValue(date);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

void Database::updateHabitFrequency(int id, QString frequency)
{
    QSqlQuery query;
    query.prepare("UPDATE habits "
                  "SET target_frequency = ? "
                  "WHERE id = ?");
    query.addBindValue(frequency);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

void Database::updateHabitStatus(int id, int status)
{
    QSqlQuery query;
    query.prepare("UPDATE habits "
                  "SET status = ? "
                  "WHERE id = ?");
    query.addBindValue(status);
    query.addBindValue(id);
    if (!query.exec()) {
    }
}

int Database::getHabitIdByName(QString name)
{
    int habitId = 0;

    QSqlQuery query("SELECT id "
                    "FROM habits "
                    "WHERE name = ?;");
    query.addBindValue(name);

    if (!query.exec() || !query.next()) {
        return habitId;
    }

    habitId = query.value(0).toInt();

    return habitId;
}

int Database::getTaskIdByName(QString name)
{
    int taskId = 0;

    QSqlQuery query("SELECT id "
                    "FROM task "
                    "WHERE name = ?;");
    query.addBindValue(name);

    if (!query.exec() || !query.next())
    {
        return taskId;
    }

    taskId = query.value(0).toInt();

    return taskId;
}

void Database::updateHabitPlan(int index, QString name, int status, int habitId, QDate date)
{
    QSqlQuery query;
    query.prepare("INSERT INTO daily_plan (habit_id, plan_date, plan_name, index_id, status) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(habitId);
    query.addBindValue(date);
    query.addBindValue(name);
    query.addBindValue(index);
    query.addBindValue(status);

    if (!query.exec())
    {
    }
}

void Database::updateTaskPlan(int index, QString name, int status, int taskId, QDate date)
{
    QSqlQuery query;
    query.prepare("INSERT INTO daily_plan (task_id, plan_date, plan_name, index_id, status) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(taskId);
    query.addBindValue(date);
    query.addBindValue(name);
    query.addBindValue(index);
    query.addBindValue(status);

    if (!query.exec())
    {
    }
}

void Database::updateReview(const QString& reflection, const QString& summary, const QDate& date, const QString& type)
{
    QDate startPeriodDate = date;
    QDate endPeriodDate = date;

    if (type == "周总结")
    {
        startPeriodDate = date.addDays(-date.dayOfWeek() + 1);
        endPeriodDate = startPeriodDate.addDays(6);
    }
    else if (type == "月总结")
    {
        startPeriodDate = date.addDays(-date.day() + 1);
        endPeriodDate = startPeriodDate.addMonths(1).addDays(-1);
    }
    else if (type == "年中总结")
    {
        startPeriodDate = QDate(date.year(), 1, 1);
        endPeriodDate = QDate(date.year(), 6, 30);
    }
    else if (type == "年终总结")
    {
        startPeriodDate = QDate(date.year(), 1, 1);
        endPeriodDate = QDate(date.year(), 12, 31);
    }
    QSqlQuery query;
    query.prepare("UPDATE daily_review "
                  "SET reflection = ?, summary = ?, review_date = ? "
                  "WHERE type = ? and period_start = ? and period_end = ?");
    query.addBindValue(reflection);
    query.addBindValue(summary);
    query.addBindValue(date);
    query.addBindValue(type);
    query.addBindValue(startPeriodDate);
    query.addBindValue(endPeriodDate);
    if (!query.exec())
    {
        qDebug() << "更新总结失败:" << query.lastError().text();
        return;
    }

    if (query.numRowsAffected() == 0)
    {
        query.prepare("INSERT INTO daily_review (review_date, reflection, summary, type, period_start, period_end) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(date);
        query.addBindValue(reflection);
        query.addBindValue(summary);
        query.addBindValue(type);
        query.addBindValue(startPeriodDate);
        query.addBindValue(endPeriodDate);

        if (!query.exec())
        {
            qDebug() << "插入总结失败:" << query.lastError().text();
        }
    }
}

void Database::updateHabitStatusByTimes(const HabitData &habit)
{
    QSqlQuery habitQuery;
    bool shouldComplete = false;
    int maxTimes;
    maxTimes = getHabitMaxTimes(habit);
    if (maxTimes >= 30) {
        shouldComplete = true;
    }
    if (shouldComplete)
    {
        habitQuery.prepare("UPDATE habits "
                           "SET status = 1 "
                           "WHERE name = ? and status = 0");
        habitQuery.addBindValue(habit.name);
        habitQuery.exec();
    }
}

int Database::getHabitTimes(const HabitData &habit)
{
    QSqlQuery query;
    int allStreak = 0;
    query.prepare("SELECT COUNT(*) "
                  "FROM daily_plan "
                  "WHERE plan_name = ? and status = 1 ");
    query.addBindValue(habit.name);
    if (!query.exec() || !query.next()) {
        return allStreak;
    }

    allStreak = query.value(0).toInt();
    return allStreak;
}

int Database::getHabitMaxTimes(const HabitData &habit)
{
    QSqlQuery planQuery;
    int maxStreak = 0;
    if (habit.target_frequency == "每日一次")
    {
        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? and status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                if (lastDate.isValid() && lastDate.addDays(1) == date) {
                    currentStreak++;
                } else {
                    currentStreak = 1;
                }
                if (currentStreak > maxStreak) {
                    maxStreak = currentStreak;
                }
                lastDate = date;
            }
        }
    }
    else if (habit.target_frequency.startsWith("每二日一次"))
    {
        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? AND status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                if (lastDate.isValid() && lastDate.addDays(2) == date) {
                    currentStreak++;
                } else {
                    currentStreak = 1;
                }
                if (currentStreak > maxStreak) {
                    maxStreak = currentStreak;
                }
                lastDate = date;
            }
        }
    }
    else if (habit.target_frequency.startsWith("每三日一次"))
    {
        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? AND status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                if (lastDate.isValid() && lastDate.addDays(3) == date) {
                    currentStreak++;
                } else {
                    currentStreak = 1;
                }
                if (currentStreak > maxStreak) {
                    maxStreak = currentStreak;
                }
                lastDate = date;
            }
        }

    }
    else if (habit.target_frequency.startsWith("每周周"))
    {
        QString weekDayStr = habit.target_frequency.mid(2, 2);
        int targetDayOfWeek = 1;
        if (weekDayStr == "周一") targetDayOfWeek = 1;
        else if (weekDayStr == "周二") targetDayOfWeek = 2;
        else if (weekDayStr == "周三") targetDayOfWeek = 3;
        else if (weekDayStr == "周四") targetDayOfWeek = 4;
        else if (weekDayStr == "周五") targetDayOfWeek = 5;
        else if (weekDayStr == "周六") targetDayOfWeek = 6;
        else if (weekDayStr == "周日") targetDayOfWeek = 7;

        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? AND status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                if (date.dayOfWeek() != targetDayOfWeek) continue;
                if (lastDate.isValid() && lastDate.addDays(7) == date) {
                    currentStreak++;
                } else {
                    currentStreak = 1;
                }
                if (currentStreak > maxStreak) {
                    maxStreak = currentStreak;
                }
                lastDate = date;
            }
        }
    }
    else if (habit.target_frequency.startsWith("每周工作日"))
    {
        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? AND status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int lastDayOfWeek = 0;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                int dayOfWeek = date.dayOfWeek();
                if (dayOfWeek >= 1 && dayOfWeek <= 5) {
                    if (lastDate.isValid()) {
                        int daysDiff = lastDate.daysTo(date);
                        if ((lastDate.dayOfWeek() == 5 && dayOfWeek == 1 && daysDiff == 3) ||
                            (lastDate.dayOfWeek() != 5 && daysDiff == 1)) {
                            currentStreak++;
                        } else {
                            currentStreak = 1;
                        }
                    } else {
                        currentStreak = 1;
                    }
                    if (currentStreak > maxStreak) {
                        maxStreak = currentStreak;
                    }
                    lastDate = date;
                }
            }
        }
    }
    else if (habit.target_frequency.startsWith("每周休息日"))
    {
        planQuery.prepare("SELECT plan_date "
                          "FROM daily_plan "
                          "WHERE plan_name = ? AND status = 1 "
                          "ORDER BY plan_date ASC");
        planQuery.addBindValue(habit.name);
        if (planQuery.exec()) {
            QDate lastDate;
            int currentStreak = 0;
            while (planQuery.next()) {
                QDate date = planQuery.value(0).toDate();
                int dayOfWeek = date.dayOfWeek();
                if (lastDate.isValid()) {
                    if ((dayOfWeek == 7 && lastDate.addDays(1) == date) ||
                        (dayOfWeek == 6 && lastDate.addDays(6) == date)) {
                        currentStreak++;
                    } else {
                        currentStreak = 1;
                    }
                    if (currentStreak > maxStreak) {
                        maxStreak = currentStreak;
                    }
                    lastDate = date;
                }
            }
        }
    }

    return maxStreak;
}

void Database::clearDailyPlanByDate(const QDate &date)
{
    QSqlQuery planQuery;
    planQuery.prepare("DELETE FROM daily_plan "
                      "WHERE plan_date = ?");
    planQuery.addBindValue(date);
    planQuery.exec();
}
