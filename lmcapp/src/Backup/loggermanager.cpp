#include "loggermanager.h"

#include <thread>
#include <chrono>
#include <fstream>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

#define RETRY_MAX 5
#define RETRY_DELAY 20 // in seconds

LoggerManager::LoggerManager() : _inProgress(false)  {
  _writablePath =
      QStandardPaths::writableLocation(QStandardPaths::DataLocation);

  QDir dir(_writablePath);
  if (!dir.exists())
    dir.mkpath(_writablePath);
}

LoggerManager::~LoggerManager() {}

void LoggerManager::writeInfo(const QString &message) {
  QString msg = QString("%1 - <message> %2 </message>\n\t<Type> Info </Type>").arg(QDateTime::currentDateTimeUtc ().toString ("yyyy.MM.dd hh:mm:ss"), message);

  writeToFile(msg);
}

void LoggerManager::writeWarning(const QString &message) {
  QString msg = QString("%1 - <message> %2 </message>\n\t<Type> >> Warning << </Type>").arg(QDateTime::currentDateTimeUtc ().toString ("yyyy.MM.dd hh:mm:ss"), message);

  writeToFile(msg);
}

void LoggerManager::writeError(const QString &message) {
  QString msg =
      QString(
          "%1 - <message> %2 </message>\n\t<Type> !! Error !! </Type>").arg(QDateTime::currentDateTimeUtc ().toString ("yyyy.MM.dd hh:mm:ss"), message);

  writeToFile(msg);
}

void LoggerManager::writeSimpleMessage(const QString &message) {
  QString msg = message;

  writeToFile(msg);
}

void LoggerManager::writeToFile(const QString &message) {
    std::thread writeThread([this, message]() {
        bool retryCount = 0;
        while (retryCount < RETRY_MAX)
            if (!_inProgress) {
                _inProgress = true;
                std::ofstream writer((_writablePath + "/Application Log.log").toStdString (), std::ios::app | std::ios::out);

                if (writer.is_open ()) {
                    writer << message.toStdString () << "\n";
                    writer.close ();
                }
                _inProgress = false;
                return;
            } else {
                std::chrono::seconds sleepTime(RETRY_DELAY);
                ++retryCount;
                std::this_thread::sleep_for(sleepTime);
            }
    });

    writeThread.detach ();
}
