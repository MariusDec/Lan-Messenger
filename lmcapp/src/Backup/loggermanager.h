#ifndef LOGGERMANAGER_H
#define LOGGERMANAGER_H

#include <QString>


class LoggerManager
{
    int     _retryCount;
    bool    _inProgress;
    QString _message;
    QString _writablePath;

    void writeToFile (const QString &message);

public:
    LoggerManager();
    ~LoggerManager();

    static LoggerManager &getInstance () {
        static LoggerManager manager;
        return manager;
    }

    void writeInfo (const QString &message);
    void writeWarning(const QString &message);
    void writeError (const QString &message);
    void writeSimpleMessage(const QString &message);
};

#endif // LOGGERMANAGER_H
