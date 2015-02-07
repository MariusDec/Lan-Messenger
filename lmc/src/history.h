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

#ifndef HISTORY_H
#define HISTORY_H

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QList>
#include "chathelper.h"
#include "settings.h"

#define HC_FILENAME "messenger.db"
#define HC_HDRSIZE 28
#define HC_VERSION 1
#define HC_DBMARKER "DB"
#define HC_IDMARKER "ID"
#define HC_DTMARKER "DT"

struct DBHeader {
  QString marker;
  int version;
  qint16 headerSize;
  int count;
  qint64 first;
  qint64 last;

  DBHeader() {
    this->marker = HC_DBMARKER;
    this->headerSize = HC_HDRSIZE;
    this->version = HC_VERSION;
  }

  DBHeader(QString szMarker, qint16 nHeaderSize, int nVersion, int nCount,
           qint64 nFirst, qint64 nLast) {
    this->marker = szMarker;
    this->headerSize = nHeaderSize;
    this->version = nVersion;
    this->count = nCount;
    this->first = nFirst;
    this->last = nLast;
  }
};

struct MsgInfo {
  QString name;
  QString id;
  QDateTime date;
  qint64 tstamp;
  QString fileName;

  MsgInfo() {}

  MsgInfo(const QString &name, const QString &id, const QDateTime &date,
          const qint64 &dateInt, const QString &fileName)
      : name(name), id(id), date(date), tstamp(dateInt), fileName(fileName) {}
};

class History {
public:
  static QString historyFile(const QDate &date);
  static QString historyFilesDir();
  static void save(const QString &user, const QString &userId, const QDateTime &timeStamp, const QList<QString> &peersList, QList<SingleMessage> &messages, bool isBroadcast = false);
  static QString getUserMessageHistory(const QString &userId, const QDate &date);
  static QList<MsgInfo> getList();
  static QList<MsgInfo> getConversationInfo(const QString &fileName);
  static QString getMessage(const QString &fileName, const QString &criteria, bool useID);
  static bool validateXml(const QString &fileName);

private:
  static QString getHtmlFromMessages(const QList<SingleMessage> messageLog);
  static void decodeMessage(QString &html);
  static void processMessageText(QString &message);
};

#endif // HISTORY_H
