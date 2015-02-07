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


#include "tcpnetwork.h"
#include "loggermanager.h"

lmcTcpNetwork::lmcTcpNetwork() {
    sendList.clear();
    receiveList.clear();
    messageMap.clear();
    locMsgStream = NULL;
    ipAddress = QHostAddress::Null;
    _server = new QTcpServer(this);
    connect(_server, &QTcpServer::newConnection, this, &lmcTcpNetwork::server_newConnection);
}

void lmcTcpNetwork::init(int nPort) {
    pSettings = new lmcSettings();
    tcpPort = nPort > 0 ? nPort : pSettings->value(IDS_TCPPORT, IDS_TCPPORT_VAL).toInt();
}

void lmcTcpNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.start started-|- Starting TCP server"));
    isRunning = _server->listen(QHostAddress::AnyIPv4, tcpPort);
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.start ended-|- connection %1").arg(isRunning ? "succeeded" : "failed"));
}

void lmcTcpNetwork::stop() {
    _server->close();
    // Close all open sockets
    if(locMsgStream)
        locMsgStream->stop();
    QMap<QString, MsgStream*>::const_iterator index = messageMap.constBegin();
    while(index != messageMap.constEnd()) {
        MsgStream* pMsgStream = index.value();
        if(pMsgStream)
            pMsgStream->stop();
        index++;
    }
    isRunning = false;
}

void lmcTcpNetwork::setLocalId(QString* lpszLocalId) {
    localId = *lpszLocalId;
}

void lmcTcpNetwork::addConnection(QString* lpszUserId, QString* lpszAddress) {
    if(!isRunning) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.addConnection-|- TCP server not running. Unable to connect"));
        return;
    }

    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addConnection started-|- Connecting to user %1 at %2").arg(*lpszUserId, *lpszAddress));

    MsgStream* msgStream = new MsgStream(localId, *lpszUserId, *lpszAddress, tcpPort);
    connect(msgStream, &MsgStream::connectionLost,
        this, &lmcTcpNetwork::msgStream_connectionLost);
    connect(msgStream, &MsgStream::messageReceived,
        this, &lmcTcpNetwork::receiveMessage);

    //	if connecting to own machine, this stream will be stored in local message stream, else in list
    if(lpszUserId->compare(localId) == 0)
        locMsgStream = msgStream;
    else
        messageMap.insert(*lpszUserId, msgStream);
    msgStream->init();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addConnection ended"));
}

void lmcTcpNetwork::sendMessage(QString* lpszReceiverId, QString* lpszData) {
    if(!isRunning) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.sendMessage-|- TCP server not running. Message not sent"));
        return;
    }

    MsgStream* msgStream;

    if(lpszReceiverId->compare(localId) == 0)
        msgStream = locMsgStream;
    else
        msgStream = messageMap.value(*lpszReceiverId, NULL);

    if(msgStream) {
        QByteArray sendData = lpszData->toUtf8();
        Datagram::addHeader(DT_Message, sendData);
        msgStream->sendMessage(sendData);
        return;
    }

    LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.sendMessage-|- Socket not found. Message sending failed"));
}

void lmcTcpNetwork::sendHandShake(QString* lpszUserId) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.sendHandShake started"));

    MsgStream* msgStream;

    if(lpszUserId->compare(localId) == 0)
        msgStream = locMsgStream;
    else
        msgStream = messageMap.value(*lpszUserId);

    if(msgStream) {
        LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.sendHandShake-|- sending handshake to user %1").arg(*lpszUserId));
        QByteArray data;
        Datagram::addHeader(DT_Handshake, data);
        msgStream->sendMessage(data);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.sendHandShake ended"));
}

void lmcTcpNetwork::initSendFile(QString* lpszReceiverId, QString *lpszReceiverName, QString* lpszAddress, QString* lpszData) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.initSendFile started-|- Receiver id: %1, receiver name: %2 on %3").arg(*lpszReceiverId, *lpszReceiverName, *lpszAddress));

    XmlMessage xmlMessage(*lpszData);
    int type = Helper::indexOf(FileTypeNames, FT_Max, xmlMessage.data(XN_FILETYPE));

    FileSender* sender = new FileSender(xmlMessage.data(XN_FILEID), localId, *lpszReceiverId, *lpszReceiverName, xmlMessage.data(XN_FILEPATH),
        xmlMessage.data(XN_FILENAME), xmlMessage.data(XN_FILESIZE).toLongLong(), *lpszAddress, tcpPort, (FileType)type);
    connect(sender, &FileSender::progressUpdated, this, &lmcTcpNetwork::update);
    sendList.prepend(sender);
    sender->init();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.initSendFile ended"));
}

void lmcTcpNetwork::initReceiveFile(QString* lpszSenderId, QString *lpszSenderName, QString* lpszAddress, QString* lpszData) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.initReceiveFile started-|- Sender id: %1, sender name: %2 on %3").arg(*lpszSenderId, *lpszSenderName, *lpszAddress));

    XmlMessage xmlMessage(*lpszData);
    int type = Helper::indexOf(FileTypeNames, FT_Max, xmlMessage.data(XN_FILETYPE));

    FileReceiver* receiver = new FileReceiver(xmlMessage.data(XN_FILEID), *lpszSenderId, *lpszSenderName, xmlMessage.data(XN_FILEPATH),
        xmlMessage.data(XN_FILENAME), xmlMessage.data(XN_FILESIZE).toLongLong(), *lpszAddress, tcpPort, (FileType)type);
    connect(receiver, &FileReceiver::progressUpdated, this, &lmcTcpNetwork::update);
    receiveList.prepend(receiver);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.initReceiveFile ended"));
}

void lmcTcpNetwork::fileOperation(FileMode mode, QString* lpszUserId, QString* lpszData) {
    Q_UNUSED(lpszUserId);

    XmlMessage xmlMessage(*lpszData);

    int fileOp = Helper::indexOf(FileOpNames, FO_Max, xmlMessage.data(XN_FILEOP));
    QString id = xmlMessage.data(XN_FILEID);

    if(mode == FM_Send) {
        FileSender* sender = getSender(id, *lpszUserId);
        if(!sender)
            return;

        switch(fileOp) {
        case FO_Cancel:
        case FO_Abort:
            sender->stop();
            removeSender(sender);
            break;
        }
    } else {
        FileReceiver* receiver = getReceiver(id, *lpszUserId);
        if(!receiver)
            return;

        switch(fileOp) {
        case FO_Cancel:
        case FO_Abort:
            receiver->stop();
            removeReceiver(receiver);
            break;
        }
    }
}

void lmcTcpNetwork::settingsChanged() {
}

void lmcTcpNetwork::setIPAddress(const QString& szAddress) {
    ipAddress = QHostAddress(szAddress);
}

void lmcTcpNetwork::server_newConnection() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.server_newConnection started"));

    QTcpSocket* socket = _server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &lmcTcpNetwork::socket_readyRead);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.server_newConnection ended"));
}

void lmcTcpNetwork::socket_readyRead() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.socket_readyRead started"));

    QTcpSocket* socket = (QTcpSocket *)sender();
    disconnect(socket, &QTcpSocket::readyRead, this, &lmcTcpNetwork::socket_readyRead);

    QByteArray buffer = socket->read(64);
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.socket_readyRead-|- buffer read: %1").arg(buffer.data()));

    if(buffer.startsWith("MSG")) {
        //	read user id from socket and assign socket to correct message stream
        QString userId(buffer.mid(3)); // 3 is length of "MSG"
        addMsgSocket(&userId, socket);
    } else if(buffer.startsWith("FILE")) {
        //	read transfer id from socket and assign socket to correct file receiver
        QString id(buffer.mid(4, 32)); // 4 is length of "FILE", 32 is length of File Id
        QString userId(buffer.mid(36));
        addFileSocket(&id, &userId, socket);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.socket_readyRead ended"));
}

void lmcTcpNetwork::msgStream_connectionLost(QString* lpszUserId) {
    emit connectionLost(lpszUserId);
}

void lmcTcpNetwork::update(FileMode mode, FileOp op, FileType type, QString* lpszId, QString* lpszUserId, QString *lpszUserName, QString* lpszData) {
    XmlMessage xmlMessage;
    xmlMessage.addHeader(XN_FROM, *lpszUserId);
    xmlMessage.addHeader(XN_TO, localId);
    xmlMessage.addData(XN_MODE, FileModeNames[mode]);
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[type]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[op]);
    xmlMessage.addData(XN_FILEID, *lpszId);

    switch(op) {
    case FO_Complete:
    case FO_Error:
        xmlMessage.addData(XN_FILEPATH, *lpszData);
        if(mode == FM_Send)
            removeSender(static_cast<FileSender*>(sender()));
        else
            removeReceiver(static_cast<FileReceiver*>(sender()));
        break;
    case FO_Progress:
        xmlMessage.addData(XN_FILESIZE, *lpszData);
        break;
    default:
        break;
    }

    QString szMessage = xmlMessage.toString();
    emit progressReceived(lpszUserId, lpszUserName, &szMessage);
}

void lmcTcpNetwork::receiveMessage(QString* lpszUserId, QString* lpszAddress, QByteArray& datagram) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.receiveMessage started-|- message received from %1 on %2").arg(*lpszUserId, *lpszAddress));

    DatagramHeader* pHeader = NULL;
    if(!Datagram::getHeader(datagram, &pHeader))
        return;

    pHeader->userId = *lpszUserId;
    pHeader->address = *lpszAddress;
    QByteArray messageData = Datagram::getData(datagram);
    QString szMessage;

    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.receiveMessage-|- TCP stream type %1 received from user %2 at %3").arg(QString::number(pHeader->type), *lpszUserId, *lpszAddress));

    switch(pHeader->type) {
    case DT_Handshake:
        emit newConnection(&pHeader->userId, &pHeader->address);
        break;
    case DT_Message:
        szMessage = QString::fromUtf8(messageData.data(), messageData.length());
        emit messageReceived(pHeader, &szMessage);
        break;
    default:
        break;
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.receiveMessage ended"));
}

void lmcTcpNetwork::addFileSocket(QString* lpszId, QString* lpszUserId, QTcpSocket* pSocket) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addFileSocket started-|- Accepted connection from user %1").arg(*lpszUserId));

    FileReceiver* receiver = getReceiver(*lpszId, *lpszUserId);
    if(receiver)
        receiver->init(pSocket);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addFileSocket ended"));
}

void lmcTcpNetwork::addMsgSocket(QString* lpszUserId, QTcpSocket* pSocket) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addMsgSocket started-|- Accepted connection from user %1").arg(*lpszUserId));

    QString address = pSocket->peerAddress().toString();
    MsgStream* msgStream = new MsgStream(localId, *lpszUserId, address, tcpPort);
    connect(msgStream, &MsgStream::connectionLost, this, &lmcTcpNetwork::msgStream_connectionLost);
    connect(msgStream,  &MsgStream::messageReceived, this, &lmcTcpNetwork::receiveMessage);
    messageMap.insert(*lpszUserId, msgStream);
    msgStream->init(pSocket);

    sendHandShake(lpszUserId);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addMsgSocket ended"));
}

FileSender* lmcTcpNetwork::getSender(QString id, QString userId) {
    for(int index = 0; index < sendList.count(); index++)
        if(sendList[index]->id.compare(id) == 0 && sendList[index]->peerId.compare(userId) == 0)
            return sendList[index];

    return NULL;
}

FileReceiver* lmcTcpNetwork::getReceiver(QString id, QString userId) {
    for(int index = 0; index < receiveList.count(); index++)
        if(receiveList[index]->id.compare(id) == 0 && receiveList[index]->peerId.compare(userId) == 0)
            return receiveList[index];

    return NULL;
}

void lmcTcpNetwork::removeSender(FileSender* pSender) {
    int index = sendList.indexOf(pSender);
    FileSender* sender = sendList.takeAt(index);
    sender->deleteLater();  // deleting later is generally safer
}

void lmcTcpNetwork::removeReceiver(FileReceiver* pReceiver) {
    int index = receiveList.indexOf(pReceiver);
    FileReceiver* receiver = receiveList.takeAt(index);
    receiver->deleteLater();  // deleting later is generally safer
}
