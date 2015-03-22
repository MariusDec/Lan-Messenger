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

#include "message.h"
#include "loggermanager.h"

QString Message::addHeader(MessageType type, qint64 id, const QString &localId, const QString &peerId, XmlMessage &message) {
    // remove time stamp from message
    message.removeHeader(XN_TIME);

    message.addHeader(XN_FROM, localId);
    if(!peerId.isEmpty())
        message.addHeader(XN_TO, peerId);
    message.addHeader(XN_MESSAGEID, QString::number(id));
    message.addHeader(XN_TYPE, MessageTypeNames[type]);

    return message.toString();
}

void Message::removeHeader(XmlMessage &message) {
    message.removeHeader(XN_TIME);
    message.removeHeader(XN_FROM);
    message.removeHeader(XN_TO);
    message.removeHeader(XN_MESSAGEID);
    message.removeHeader(XN_TYPE);
}

bool Message::getHeader(const QString &message, MessageHeader &header, XmlMessage &xmlMessage) {
    xmlMessage.setContent(message);
    if(!xmlMessage.isValid())
        return false;

    // add time stamp to message
    xmlMessage.addHeader(XN_TIME, QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()));

    int type = Helper::indexOf(MessageTypeNames, MT_Max, xmlMessage.header(XN_TYPE));
    if(type < 0)
        return false;

    header.init((MessageType)type,
                    xmlMessage.header(XN_MESSAGEID).toLongLong(),
                    xmlMessage.header(XN_FROM));
    return true;
}
