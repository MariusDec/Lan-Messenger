﻿/****************************************************************************
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


#include <QDataStream>
#include "datagram.h"
#include "loggermanager.h"

void Datagram::addHeader(DatagramType type, QByteArray& baData) {
    QByteArray datagramType = DatagramTypeNames[type].toLocal8Bit();
    baData.insert(0, datagramType);
}

bool Datagram::getHeader(const QByteArray &datagram, DatagramHeader &header) {
    QString data(datagram);
    LoggerManager::getInstance().writeInfo(QString("Datagram.getHeader started-|- datagram: %1").arg(data));

    QString datagramType(datagram.mid(0, 6));	// first 6 bytes represent datagram type
    int type = Helper::indexOf(DatagramTypeNames, DT_Max, datagramType);
    if(type < 0) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("Datagram.getHeader ended-|- type: -1"));
        return false;
    }

    header.init((DatagramType)type,
                    QString(),
                    QString());

    LoggerManager::getInstance().writeWarning(QString("Datagram.getHeader ended-|- type: %1").arg(type));

    return true;
}

QByteArray Datagram::getData(QByteArray& baDatagram) {
    if(baDatagram.length() > 6)
        return baDatagram.mid(6);

    return QByteArray();
}
