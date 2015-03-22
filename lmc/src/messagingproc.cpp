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

#include "messaging.h"
#include "loggermanager.h"
#include "stdlocation.h"

//	A broadcast is to be sent
void lmcMessaging::sendBroadcast(MessageType type) {
    prepareBroadcast(type);
}

//	A message is to be sent
void lmcMessaging::sendMessage(MessageType type, const QString &userId, XmlMessage &message) {
    QString data = QString::null;

    switch(type) {
    case MT_Group:
        data = message.data(XN_GROUP);
        updateUser(type, userId, data);
        break;
    case MT_Status:
        if (!userId.isEmpty())
            prepareMessage(type, msgId, false, userId, message);
        else
            for(int index = 0; index < userList.count(); index++)
                prepareMessage(type, msgId, false, userList[index].id, message);
        msgId++;
        break;
    case MT_UserName:
    case MT_Note:
    case MT_PublicMessage:
        for(int index = 0; index < userList.count(); index++)
            prepareMessage(type, msgId, false, userList[index].id, message);
        msgId++;
        break;
    case MT_GroupMessage:
        if(!userId.isEmpty())
            prepareMessage(type, msgId, false, userId, message);
        else {
            for(int index = 0; index < userList.count(); index++)
                prepareMessage(type, msgId, false, userList[index].id, message);
        }
        msgId++;
        break;
    case MT_Avatar:
        //	if user id is specified send to that user alone, else send to all
        if(!userId.isEmpty()) {
            prepareMessage(type, msgId, false, userId, message);
        } else {
            XmlMessage msg;
            emit messageReceived(MT_Avatar, localUser->id, message);
            for(int index = 0; index < userList.count(); index++) {
                msg.setContent(message.toString());
                prepareMessage(type, msgId, false, userList[index].id, msg);
            }
        }
        msgId++;
        break;
    case MT_Version:
        sendWebMessage(type);
        break;
    default:
        prepareMessage(type, msgId, false, userId, message);
        msgId++;
        break;
    }
}

void lmcMessaging::sendWebMessage(MessageType type) {
    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.sendWebMessage started-|- Sending web message type %1").arg(QString::number(type)));

    QString szUrl;

    switch(type) {
    case MT_Version:
        szUrl = QString(IDA_DOMAIN"/webservice.php?q=version&p=" IDA_PLATFORM);
        pNetwork->sendWebMessage(szUrl);
        break;
    default:
        break;
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.sendWebMessage ended"));
}

//	A broadcast has been received
void lmcMessaging::receiveBroadcast(const DatagramHeader &header, const QString &data) {
    MessageHeader messageHeader;
    XmlMessage message;
    if(!Message::getHeader(data, messageHeader, message)) {
        LoggerManager::getInstance().writeWarning(QString("lmcMessaging.receiveWebMessage started -|- Broadcast header parse failed. Header: %1").arg(data));
        return;
    }
    messageHeader.address = header.address;
    processBroadcast(messageHeader);
}

//	A message has been received
void lmcMessaging::receiveMessage(const DatagramHeader &header, const QString &data) {
    MessageHeader messageHeader;
    XmlMessage message;
    if(!Message::getHeader(data, messageHeader, message)) {
        LoggerManager::getInstance().writeWarning(QString("lmcMessaging.receiveMessage -|- Message header parse failed. Header: %1").arg(data));
        return;
    }
    messageHeader.address = header.address;
    processMessage(messageHeader, message);
}

//	A web message has been received
void lmcMessaging::receiveWebMessage(const QString &data) {
    MessageHeader messageHeader;
    XmlMessage message;
    if(!Message::getHeader(data, messageHeader, message)) {
        LoggerManager::getInstance().writeWarning(QString("lmcMessaging.receiveWebMessage started -|- Web message header parse failed. Header: %1").arg(data));
        return;
    }

    processWebMessage(messageHeader, message);
}

//	Handshake procedure has been completed
void lmcMessaging::newConnection(QString userId, QString address) {
    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.newConnection started -|- Connection completed with user %1 at %2").arg(userId, address));
    sendUserData(MT_UserData, QO_Get, userId);
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.newConnection ended"));
}

void lmcMessaging::connectionLost(QString userId) {
    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.connectionLost started -|- Connection to user %1 lost").arg(userId));
    removeUser(userId);
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.connectionLost ended"));
}

void lmcMessaging::sendUserData(MessageType type, QueryOp op, const QString &userId) {
    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.sendUserData started -|- Sending local user details to user %1 at %2").arg(userId));

    XmlMessage xmlMessage;
    xmlMessage.addData(XN_USERID, localUser->id);
    xmlMessage.addData(XN_NAME, localUser->name);
    xmlMessage.addData(XN_ADDRESS, localUser->address);
    xmlMessage.addData(XN_VERSION, localUser->version);
    xmlMessage.addData(XN_STATUS, localUser->status);
    xmlMessage.addData(XN_NOTE, localUser->note);
    xmlMessage.addData(XN_USERCAPS, QString::number(localUser->caps));
    xmlMessage.addData(XN_HOST, localUser->hostName);
    xmlMessage.addData(XN_QUERYOP, QueryOpNames[op]);

    QString messageToSend = Message::addHeader(type, msgId, localUser->id, userId, xmlMessage);
    pNetwork->sendMessage(userId, messageToSend);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.sendUserData ended"));
}

void lmcMessaging::prepareBroadcast(MessageType type) {
    if(!isConnected()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.prepareBroadcast -|- Not connected. Broadcast not sent"));
        return;
    }
    if(localUser->id.isNull()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.prepareBroadcast -|- Local user not initialized. Broadcast not sent"));
        return;
    }

    XmlMessage message;
    QString messageToSend = Message::addHeader(type, msgId, localUser->id, QString::null, message);
    pNetwork->sendBroadcast(messageToSend);
}

//	This method converts a Message from ui layer to a Datagram that can be passed to network layer
void lmcMessaging::prepareMessage(MessageType type, qint64 msgId, bool retry, const QString &userId, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareMessage started"));

    if(!isConnected()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.prepareMessage -|- Not connected. Message not sent"));
        return;
    }
    if(localUser->id.isNull()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.prepareMessage -|- Local user not initialized. Message not sent"));
        return;
    }
    if(userId.isEmpty()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.prepareMessage -|- No receiver user specified. Message not sent"));
        return;
    }

    User *receiver = getUser(userId);

    switch(type) {
    case MT_Status:
        if (!message.dataExists(XN_STATUS))
            message.addData(XN_STATUS, localUser->status);
        break;
    case MT_UserName:
        if (!message.dataExists(XN_NAME))
            message.addData(XN_NAME, localUser->name);
        break;
    case MT_Note:
        if (!message.dataExists(XN_NOTE))
            message.addData(XN_NOTE, localUser->note);
        break;
    case MT_Message:
        if(!receiver) {
            emit messageReceived(MT_Failed, userId, message);
            break;
        }
        //	add message to pending list
        if(!retry)
            addPendingMsg(msgId, MT_Message, userId, message);
        break;
    case MT_GroupMessage:
    case MT_PublicMessage:
    case MT_Broadcast:
    case MT_InstantMessage:
    case MT_Acknowledge:
        break;
    case MT_Query:
        //	if its a 'get' query add message to pending list
        if(message.data(XN_QUERYOP) == QueryOpNames[QO_Get] && !retry)
            addPendingMsg(msgId, MT_Query, userId, message); // sendUserData(MessageType type, QueryOp op, const QString &userId)
        else if(message.data(XN_QUERYOP) == QueryOpNames[QO_Result])
            getUserInfo(message);
        break;
    case MT_ChatState:
        break;
    case MT_File:
    case MT_Avatar:
        prepareFile(userId, message);
        break;
    case MT_Folder:
        prepareFolder(userId, message);
        break;
    default:
        break;
    }

    if(!receiver) {
        LoggerManager::getInstance().writeWarning(QString("lmcMessaging.prepareMessage -|- Recipient %1 not found. Message not sent").arg(userId));
        return;
    }

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.prepareMessage -|- Sending message type %1 (%2) to user %3 at %4").arg(QString::number(type), MessageTypeNames[type], receiver->id, receiver->address));

    Message::removeHeader(message);
    QString messageToSend = Message::addHeader(type, msgId, localUser->id, userId, message);
    pNetwork->sendMessage(receiver->id, messageToSend);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareMessage ended"));
}

//	This method converts a Datagram from network layer to a Message that can be passed to ui layer
void lmcMessaging::processBroadcast(const MessageHeader &header) {
    //	do not process broadcasts from local user unless loopback is specified in command line
    if(!loopback && header.userId.compare(localUser->id) == 0)
        return;

    switch(header.type) {
    case MT_Announce:
        if(!getUser(header.userId))
            pNetwork->addConnection(header.userId, header.address);
        break;
    case MT_Depart:
        removeUser(header.userId);
        break;
    default:
        break;
    }
}

void lmcMessaging::processMessage(const MessageHeader &header, XmlMessage &message) {
    QString msgId;
    QString data = QString::null;
    XmlMessage reply;

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.processMessage started -|- Processing message type %1 (%2) from user %3").arg(QString::number(header.type), ((header.type >= 0) ? MessageTypeNames[header.type] : ""), header.userId));

    switch(header.type) {
    case MT_UserData:
        if(message.data(XN_QUERYOP) == QueryOpNames[QO_Get])
            sendUserData(header.type, QO_Result, header.userId);
        //	add the user only after sending back user data, this way both parties will have added each other
        addUser(message.data(XN_USERID), message.data(XN_VERSION), message.data(XN_ADDRESS),
            message.data(XN_NAME), message.data(XN_STATUS), message.data(XN_AVATAR), message.data(XN_NOTE),
            message.data(XN_USERCAPS), message.data(XN_HOST));
        break;
    case MT_Broadcast:
    case MT_InstantMessage:
        emit messageReceived(header.type, header.userId, message);
        break;
    case MT_Status:
        data = message.data(XN_STATUS);
        updateUser(header.type, header.userId, data);
        break;
    case MT_UserName:
        data = message.data(XN_NAME);
        updateUser(header.type, header.userId, data);
        break;
    case MT_Note:
        data = message.data(XN_NOTE);
        updateUser(header.type, header.userId, data);
        break;
    case MT_Message:
        //	add message to received message list
        if(addReceivedMsg(header.id, header.userId)) {
            emit messageReceived(header.type, header.userId, message);
        }

        //	send an acknowledgement
        msgId = QString::number(header.id);
        reply.addData(XN_MESSAGEID, msgId);
        sendMessage(MT_Acknowledge, header.userId, reply);
        break;
    case MT_GroupMessage:
    case MT_UserList:
        emit messageReceived(header.type, header.userId, message);
        break;
    case MT_PublicMessage:
        emit messageReceived(header.type, header.userId, message);
        break;
    case MT_Query:
        //	send a reply cum acknowledgement if its a 'get' query
        if(message.data(XN_QUERYOP) == QueryOpNames[QO_Get]) {
            msgId = QString::number(header.id);
            reply.addData(XN_MESSAGEID, msgId);
            reply.addData(XN_QUERYOP, QueryOpNames[QO_Result]);
            sendMessage(header.type, header.userId, reply);
        } else if(message.data(XN_QUERYOP) == QueryOpNames[QO_Result]) {
            msgId = message.data(XN_MESSAGEID);
            removePendingMsg(msgId.toLongLong());

            //  Add the path to the user's avatar image stored locally
            data = "avt_" + header.userId + ".png";
            data = QDir(StdLocation::getCacheDir()).absoluteFilePath(data);

            reply.setContent(message.toString());
            reply.addData(XN_AVATAR, data);
            emit messageReceived(header.type, header.userId, reply);
        }
        break;
    case MT_ChatState:
        emit messageReceived(header.type, header.userId, message);
        break;
    case MT_Acknowledge:
        //	remove message from pending list
        msgId = message.data(XN_MESSAGEID);
        removePendingMsg(msgId.toLongLong());
        break;
    case MT_File:
    case MT_Avatar:
        processFile(header, message);
        break;
    case MT_Folder:
        processFolder(header, message);
        break;
    default:
        break;
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.processMessage ended"));
}

void lmcMessaging::processWebMessage(const MessageHeader &header, const XmlMessage &message) {
    switch(header.type) {
    case MT_Version:
        emit messageReceived(header.type, QString::null, message);
        break;
    case MT_WebFailed:
        emit messageReceived(header.type, QString::null, message);
        break;
    default:
        break;
    }
}
