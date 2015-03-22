#include "loggermanager.h"
#include "shared.h"

#include <thread>
#include <chrono>
#include <fstream>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

#include "stdlocation.h"

#define RETRY_MAX 5
#define RETRY_DELAY 20 // in seconds

LoggerManager::LoggerManager() {

  QString writablePath = StdLocation::getWritableDataDir("logs");

  QString fileName = QString("%1 _ %2 - %3.log").arg(QDate::currentDate().toString(Qt::ISODate), Helper::getHostName(), Helper::getLogonName());
  writablePath.append(fileName);

  _writablePath = writablePath.toStdString();

  writeSimpleMessage("\n\n\t-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
        "-\n"
        "\t         " IDA_TITLE " " IDA_VERSION " application log\n"
        "\t-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

  std::thread watcher([this]() {
      std::chrono::milliseconds interval(100);
      while(true) {
          if (!_pendingMessages.isEmpty()) {
              writeToFile(_pendingMessages.first().toStdString());

              _pendingChangeMutex.lock();
              _pendingMessages.removeAt(0);
              _pendingChangeMutex.unlock();
          } else
              std::this_thread::sleep_for(interval);
      }
  });
  watcher.detach();
}

LoggerManager::~LoggerManager() {}

void LoggerManager::writeInfo(const QString &message) {
  QString msg = QString("<entry>\n\t<message> %1 </message>\n\t<Date> %2 </Date>\n\t<Type> Info </Type>\n</entry>\n").arg(message, QDateTime::currentDateTime ().toString ("yyyy.MM.dd hh:mm:ss (t)"));

  appendMessagesList(msg);
}

void LoggerManager::writeWarning(const QString &message) {
  QString msg = QString("<entry>\n\t<message> %1 </message>\n\t<Date> %2 </Date>\n\t<Type> >> Warning << </Type>\n</entry>\n").arg(message, QDateTime::currentDateTime ().toString ("yyyy.MM.dd hh:mm:ss (t)"));

  appendMessagesList(msg);
}

void LoggerManager::writeError(const QString &message) {
    // TODO find a way to log the caller function
  QString msg =
      QString(
          "<entry>\n\t<message> %1 </message>\n\t<Date> %2 </Date>\n\t<Type> !! Error !! </Type>\n</entry>\n").arg(message, QDateTime::currentDateTime ().toString ("yyyy.MM.dd hh:mm:ss (t)"));

  appendMessagesList(msg);
}

void LoggerManager::writeSimpleMessage(const QString &message) {
  QString msg = message;

  appendMessagesList(msg);
}

void LoggerManager::writeToFile(const std::string &message) {
    std::ofstream writer(_writablePath, std::ios::app | std::ios::out);

    if (writer.is_open ()) {
        writer << message << "\n";
        writer.close ();
    }
}

void LoggerManager::appendMessagesList(const QString &message)
{
    _pendingChangeMutex.lock();
    _pendingMessages.append(message);
    _pendingChangeMutex.unlock();
}
