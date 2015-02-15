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


#ifndef SHARED_H
#define SHARED_H

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QHostInfo>
#include <QRegularExpression>
#include "definitions.h"
#ifdef QWIDGET_H
#include "uidefinitions.h"
#endif

struct User {
    QString id;
    QString name;
    QString address;
    QString version;
    QString status;
    QString group;
    QString note;
    int avatar;
    QString avatarPath;
    uint caps;
    QString hostName;
    int lanIndex;

    User() {}
    User(const QString &id, const QString &version, const QString &address, const QString &name, const QString &status, const QString &group,
         int nAvatar, const QString &note, const QString &avatarPath, const QString &caps, const QString &hostName) : id(id), name(name), address(address), version(version), status(status), group(group), note(note), avatar(nAvatar), avatarPath(avatarPath), caps(caps.toInt()), hostName(hostName) {
        lanIndex = address.mid(address.lastIndexOf('.') + 1).toInt();

        QString userPc = QString("C%1 - ").arg(lanIndex);
        if (!address.endsWith(QString(".1.%1").arg(lanIndex))) {
            QRegularExpressionMatch match = QRegularExpression("^(C(\\d)+)$").match(hostName); // TODO !!! test
            if (match.captured() == hostName) {
                userPc = QString("%1 - ").arg(hostName);
                lanIndex = hostName.mid(1).toInt();
            }
        }

        if (!this->name.startsWith(userPc)) {
            QString match = QRegularExpression("^(C(\\d)+ \\- )$").match(hostName).captured();
            if (this->name.startsWith(match)) {
                this->name.remove(0, match.length());

                this->name.prepend(userPc);
            }
        }
    }
};

struct Group {
    QString id;
    QString name;

    Group() {}
    Group(QString szId) {
        this->id = szId;
    }
    Group(QString szId, QString szName) {
        this->id = szId;
        this->name = szName;
    }

    bool operator == (const Group& v) const { return (this->id.compare(v.id) == 0); }
};

struct DatagramHeader {
    DatagramType type;
    QString userId;
    QString address;

    DatagramHeader(DatagramType dtType, QString szUserId, QString szAddress) {
        type = dtType;
        userId = szUserId;
        address = szAddress;
    }
};

struct MessageHeader {
    MessageType type;
    qint64 id;
    QString userId;
    QString address;

    MessageHeader(MessageType mtType, qint64 nId, QString szUserId) {
        type = mtType;
        id = nId;
        userId = szUserId;
    }
};

class Helper {
public:
    static int indexOf(const QString array[], int size, const QString& value);
    static QString formatSize(qint64 size);
    static QString getUuid();
    static QString getLogonName();
    static QString getHostName();
    static QString getOSName();
    static QString escapeDelimiter(QString* lpszData);
    static QString unescapeDelimiter(QString* lpszData);
    static int compareVersions(const QString& version1, const QString& version2);
    static QString boolToString(bool value);
    static bool stringToBool(const QString& value);
    static bool copyFile(const QString& source, const QString& destination);
};

#endif // SHARED_H
