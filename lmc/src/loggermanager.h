#ifndef LOGGERMANAGER_H
#define LOGGERMANAGER_H

#include <QStringList>
#include <mutex>


class LoggerManager
{
    std::mutex    _pendingChangeMutex;
    QString       _message;
    std::string   _writablePath;
    QStringList   _pendingMessages;

    void writeToFile (const std::string &message);
    void appendMessagesList (const QString &message);

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
