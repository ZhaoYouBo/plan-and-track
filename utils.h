#ifndef UTILS_H
#define UTILS_H
#include <QStringList>
#include <QColor>

class Utils
{
public:
    Utils();

public:
    static QString taskStatusToString(int status);
    static QString habitStatusToString(int status);
    static QString planStatusToString(int status);
    static QStringList taskStatusList();
    static QStringList habitStatusList();
    static QStringList habitFrequencyList();
    static QStringList planStatusList();
    static int taskStatusFromString(const QString &str);
    static int habitStatusFromString(const QString &str);
    static int planStatusFromString(const QString &str);

    static const QMap<QString, QColor>& statusColorMap();
};

#endif // UTILS_H
