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

#define PORTABLE 0
#define PortableResFolder "resources"

struct StdLocation {
  static QString defaultTransferHistorySavePath();

  static QStringList historyFiles();
  static QString defaultHistorySavePath();
  static QString defaultFileStoragePath(const QString &sender = QString());

  static QString getWritableDataDir(const QString &folderName);
  static QString getDataDir(const QString &folderName);
  static QStringList getDataDirs();
  static QString getWritableCacheDir();
  static QString getCacheDir();
  static QString getGenericDocumentsPath();
  static QString getDocumentsPath();

  static QString translationsPath();

  static QString writableGroupsFile();
  static QString groupsFile();

  static QString logPath();

  static QString tempConfigFile();
};

#endif // STDLOCATION_H
