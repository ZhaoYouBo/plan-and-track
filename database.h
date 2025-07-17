#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QDate>

struct TaskData {
    int id; // Primary key
    QString name; // Task name
    QDate createdDate; // Created date
    QDate dueDate; // Due date
    QDate completedDate; // Completed date
    int status; // Task status
};

struct HabitData {
    int id; // Primary key
    QString name; // Habit name
    QDate createdDate; // Created date
    QString target_frequency; // Habit Frequency
    int status; // Habit status
};

struct PlanData {
    int id; // Primary key
    QString type;
    QString name; // Plan name
    QString target_frequency; // Habit Frequency
    int status; // Habit status
};

struct ReviewData {
    QString reflection;
    QString summary;
};
class Database
{
public:
    Database(const QString& dbName);

    QList<TaskData> getTaskByStatus(int status);
    QList<HabitData> getHabitByStatus(int status);
    QList<PlanData> getPlanByDate(const QDate& date);
    QMap<QDate,double> getPlanNumberByDate(const QDate& startDate, const QDate& endDate);
    ReviewData getReviewByDate(const QString& type, const QDate& startDate, const QDate& endDate);
    QList<ReviewData> getReviewByType(const QString& type, const QDate& startDate, const QDate& endDate);
    void addTask(TaskData data);
    void addHabit(HabitData data);
    void updateTaskName(int id, const QString& name);
    void updateTaskDueDate(int id, const QDate& date);
    void updateTaskStatus(int id, int status);
    void updateHabitName(int id, const QString& name);
    void updateHabitCreatedDate(int id, const QDate& date);
    void updateHabitFrequency(int id, QString frequency);
    void updateHabitStatus(int id, int status);
    int getHabitIdByName(QString name);
    int getTaskIdByName(QString name);
    void updateHabitPlan(int index, QString name, int status, int habitId, QDate date);
    void updateTaskPlan(int index, QString name, int status, int taskId, QDate date);
    void updateReview(const QString& reflection, const QString& summary, const QDate& date, const QString& type);
    void updateHabitStatusByTimes(const HabitData &habit);
    int getHabitTimes(const HabitData &habit);
    int getHabitMaxTimes(const HabitData &habit);
    void clearDailyPlanByDate(const QDate& date);

private:
    QSqlDatabase m_db;

    /**
     * @brief createTables Creates core database tables if they don't exist
     */
    void createTables();
};

#endif // DATABASE_H
