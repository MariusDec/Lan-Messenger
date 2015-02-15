/****************************************************************************
**
** This file is part of LAN Messenger.
**
** Copyright (c) 2010 - 2012 Qualia Digital Solutions.
**
** Contact:  qualiatech@gmail.com
**
** LAN Messenger is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** LAN Messenger is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with LAN Messenger.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef STDLOCATION_H
#define STDLOCATION_H

#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDateTime>
#include <QDirIterator>
#include "settings.h"
#include "loggermanager.h"

#define SL_RESOURCE "lmc.rc"
#define SL_LANGDIR "lang"
#define SL_THEMEDIR "themes"
#define SL_AVATARFILE "avatars/default.png"
#define SL_LOGDIR "logs"
#define SL_TEMPCONFIG "lmctmpconf.ini"

#define PORTABLE 0
#define PortableResFolder "resources"

class StdLocation {
public:
  static QString transferHistoryFilePath() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.transferHistoryPath-|- no writable location found"));
      return "";
    }
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    // create the dir if it does not exist
    QDir dir(location);
    if (!dir.exists())
      dir.mkpath(location);

    location.append("/transfers.lst");
    return location;
  }

  static QString historyFile(const QDate &date) {
      lmcSettings settings;

      QString path = settings.value(IDS_HISTORYPATH, IDS_HISTORYPATH_VAL).toString();

      if (path.isEmpty()) {
#if !PORTABLE
          path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
          path = qApp->applicationDirPath() + "/" PortableResFolder;
#endif
          if (!path.endsWith('/'))
              path.append('/');

          path.append(QString("history/conversation %1.xml").arg(date.toString(Qt::ISODate)));
      }

      return path;
  }

  static QStringList historyFiles() {
      lmcSettings settings;

      QStringList historyFiles;

      QString path = settings.value(IDS_HISTORYPATH, IDS_HISTORYPATH_VAL).toString();

      if (path.isEmpty()) {
#if !PORTABLE
          path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
          path = qApp->applicationDirPath() + "/" PortableResFolder;
#endif
          if (!path.endsWith('/'))
              path.append('/');

          path.append(QStringLiteral("history/"));
      }

      QDir dir(path);
      dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

      foreach (const QString &fileName, dir.entryList())
          historyFiles.append(dir.absoluteFilePath(fileName));

      return historyFiles;
  }

  static QString historyFilesDir() {
      lmcSettings settings;

      QString path = settings.value(IDS_HISTORYPATH, IDS_HISTORYPATH_VAL).toString();

      if (path.isEmpty()) {
#if !PORTABLE
          path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
          path = qApp->applicationDirPath() + "/" PortableResFolder;
#endif
          if (!path.endsWith('/'))
              path.append('/');

          path.append(QStringLiteral("history/"));
      }

      return path;
  }

  static QString getFileStoragePath(const QString &sender = QString()) {
    lmcSettings settings;
    QString location = settings.value(IDS_FILESTORAGEPATH, "").toString();

    if (location.isEmpty()) {
#if !PORTABLE
      location =
          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
            "StdLocation.getFileStoragePath-|- no writable location found"));
        return "";
      }
#else
        location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

      location.append(
          (QString("/%1/Received files/").arg (QApplication::applicationName())));
    }

    if (!location.endsWith('/'))
        location.append('/');

    if (!sender.isEmpty()) {
        bool userFolders = settings.value(IDS_STORAGEUSERFOLDER, IDS_STORAGEUSERFOLDER_VAL).toBool();
        if (userFolders)
            location.append(QString("%1/").arg(sender));
    }

    // create the dir if it does not exist
    QDir dir(location);
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString getWritableDataDir(const QString &folderName) {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.getWritableCacheDir-|- no writable location found"));
      return "";
    }

    QDir dir(location.append(QString("/%1/").arg (folderName)));
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString getDataDir(const QString &folderName) {
#if !PORTABLE
    QStringList dataLocations =
        QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
      fileInfo.setFile(location.append(QString("/%1/").arg(folderName)));
      if (fileInfo.exists())
        return location;
    }
#else
      return qApp->applicationDirPath() + "/" PortableResFolder "/" + folderName + "/";
#endif
    return "";
  }

  static QStringList getDataDirs() {
#if !PORTABLE
    QStringList dataLocations =
        QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();
    for (QString &dataLocation : dataLocations)
        dataLocation.append ("/");
#else
      QStringList dataLocations;
      dataLocations.append(QString("%1/%2/").arg(qApp->applicationDirPath(), PortableResFolder));
#endif

    return dataLocations;
  }

  static QString getWritableCacheDir() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.getWritableCacheDir-|- no writable location found"));
      return "";
    }

    QDir dir(location.append(QStringLiteral("/cache/")));
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString getCacheDir() {
#if !PORTABLE
    QStringList dataLocations =
        QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
      fileInfo.setFile(location.append(QStringLiteral("/cache/")));
      if (fileInfo.exists())
        return (location);
    }
#else
      return qApp->applicationDirPath() + "/" PortableResFolder "/cache/";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
                                                "StdLocation.getCacheDir-|- a suitable location could not be found"));

    return "";
  }

  static QString getGenericDocumentsPath() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.getGenericDocumentsPath-|- no writable location found"));
      return "";
    }

    QDir dir(location);
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString getDocumentsPath() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.getDocumentsPath-|- no writable location found"));
      return "";
    }

    QDir dir(location.append(QString("/%1/").arg (QApplication::applicationName())));
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString libDir() {
    return QDir::toNativeSeparators(QDir::currentPath());
  }

  static QString resourceFile() {
    return QDir::toNativeSeparators(
        QDir::current().absoluteFilePath("lmc.rc"));
  }

  static QString translationsPath() {
#if !PORTABLE
    QStringList dataLocations =
        QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
      fileInfo.setFile(location.append("/translations/"));
      if (fileInfo.exists())
        return (location);
    }
#else
      return qApp->applicationDirPath() + "/" PortableResFolder "/translations/";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
        "StdLocation.translationsPath-|- a suitable location could not be found"));

    return "";
  }

  static QString writableGroupsFile() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(QStringLiteral(
          "StdLocation.writableGroupsFile-|- no writable location found"));
      return "";
    }

    QDir dir(location);
    if (!dir.exists())
      dir.mkpath(location);

    return location.append(QStringLiteral("/groups.cfg"));

    LoggerManager::getInstance().writeError(QStringLiteral(
        "StdLocation.writableGroupsFile-|- a suitable location could not be found"));

    return "";
  }

  static QString groupsFile() {
#if !PORTABLE
    QStringList dataLocations =
        QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
      fileInfo.setFile(location.append("/groups.cfg"));
      if (fileInfo.exists())
        return (location);
    }
#else
      return qApp->applicationDirPath() + "/" PortableResFolder "/groups.cfg";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
        "StdLocation.groupsFile-|- an existing groups file could not be found"));

    return "";
  }

  // returns the dir where the log can be written to
  static QString logPath() {
#if !PORTABLE
    QString location =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
      QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
      LoggerManager::getInstance().writeError(
          QStringLiteral("StdLocation.logPath-|- no writable location found"));
      return "";
    }

    // create the dir if it does not exist
    QDir dir(location.append("/logs"));
    if (!dir.exists())
      dir.mkpath(location);

    return location;
  }

  static QString freeLogFile() {
    QString fileName =
        "lmc_" +
        QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()) +
        ".log";
    return (logPath() + "/" + fileName);
  }

  static QString tempConfigFile() {
    QString location =
        QStandardPaths::writableLocation (QStandardPaths::TempLocation);

    if (location.isEmpty ()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
            "StdLocation.tempConfigFile-|- no writable location found"));
        return "";
    }

    QDir dir(location);
    if (!dir.exists ())
        dir.mkpath (location);

    return location.append ("lmc_tmpConf.conf");
  }
};

#endif // STDLOCATION_H
