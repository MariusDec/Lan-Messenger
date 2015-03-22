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
#include "globals.h"

lmcTcpNetwork::lmcTcpNetwork() {
    _localMsgStream = nullptr;
    _server.setParent(this);
    connect(&_server, &QTcpServer::newConnection, this, &lmcTcpNetwork::server_newConnection);
}

void lmcTcpNetwork::init(int nPort) {
    _tcpPort = nPort > 0 ? nPort : Globals::getInstance().tcpPort();
}

void lmcTcpNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.start started-|- Starting TCP server"));
    _isRunning = _server.listen(QHostAddress::AnyIPv4, _tcpPort);
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.start ended-|- connection %1").arg(_isRunning ? "succeeded" : "failed"));
}

void lmcTcpNetwork::stop() {
    _server.close();
    // Close all open sockets
    if(_localMsgStream)
        _localMsgStream->stop();
    QMap<QString, MsgStream*>::const_iterator index = _messageMap.constBegin();
    while(index != _messageMap.constEnd()) {
        MsgStream* pMsgStream = index.value();
        if(pMsgStream)
            pMsgStream->stop();
        index++;
    }
    _isRunning = false;
}

void lmcTcpNetwork::setLocalId(const QString &localId) {
    _localId = localId;
}

void lmcTcpNetwork::addConnection(const QString &userId, const QString &address) {
    if(!_isRunning) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.addConnection-|- TCP server not running. Unable to connect"));
        return;
    }

    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addConnection started-|- Connecting to user %1 at %2").arg(userId, address));

    MsgStream* msgStream = new MsgStream(_localId, userId, address, _tcpPort);
    connect(msgStream, &MsgStream::connectionLost,
        this, &lmcTcpNetwork::msgStream_connectionLost);
    connect(msgStream, &MsgStream::messageReceived,
        this, &lmcTcpNetwork::receiveMessage);

    //	if connecting to own machine, this stream will be stored in local message stream, else in list
    if(userId.compare(_localId) == 0)
        _localMsgStream = msgStream;
    else
        _messageMap.insert(userId, msgStream);
    msgStream->init();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addConnection ended"));
}

void lmcTcpNetwork::sendMessage(const QString &receiverId, const QString &data) {
    if(!_isRunning) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.sendMessage-|- TCP server not running. Message not sent"));
        return;
    }

    MsgStream* msgStream;

    if(!receiverId.compare(_localId))
        msgStream = _localMsgStream;
    else
        msgStream = _messageMap.value(receiverId, NULL);

    if(msgStream) {
        QByteArray sendData = data.toUtf8();
        Datagram::addHeader(DT_Message, sendData);
        msgStream->sendMessage(sendData);
        return;
    }

    LoggerManager::getInstance().writeWarning(QStringLiteral("lmcTcpNetwork.sendMessage-|- Socket not found. Message sending failed"));
}

void lmcTcpNetwork::sendHandShake(const QString &userId) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.sendHandShake started"));

    MsgStream* msgStream;

    if(!userId.compare(_localId))
        msgStream = _localMsgStream;
    else
        msgStream = _messageMap.value(userId);

    if(msgStream) {
        LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.sendHandShake-|- sending handshake to user %1").arg(userId));
        QByteArray data;
        Datagram::addHeader(DT_Handshake, data);
        msgStream->sendMessage(data);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.sendHandShake ended"));
}

void lmcTcpNetwork::initSendFile(const QString &receiverId, const QString &receiverName, const QString &address, const QString &data) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.initSendFile started-|- Receiver id: %1, receiver name: %2 on %3").arg(receiverId, receiverName, address));

    XmlMessage xmlMessage(data);
    int type = Helper::indexOf(FileTypeNames, FT_Max, xmlMessage.data(XN_FILETYPE));

    FileSender* sender = new FileSender(xmlMessage.data(XN_FILEID), _localId, receiverId, receiverName, xmlMessage.data(XN_FILEPATH),
        xmlMessage.data(XN_FILENAME), xmlMessage.data(XN_FILESIZE).toLongLong(), address, _tcpPort, (FileType)type);
    connect(sender, &FileSender::progressUpdated, this, &lmcTcpNetwork::update);
    _sendList.prepend(sender);
    sender->init();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.initSendFile ended"));
}

void lmcTcpNetwork::initReceiveFile(const QString &senderId, const QString &senderName, const QString &address, const QString &data) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.initReceiveFile started-|- Sender id: %1, sender name: %2 on %3").arg(senderId, senderName, address));

    XmlMessage xmlMessage(data);
    int type = Helper::indexOf(FileTypeNames, FT_Max, xmlMessage.data(XN_FILETYPE));

    FileReceiver *receiver = new FileReceiver(xmlMessage.data(XN_FILEID), senderId, senderName, xmlMessage.data(XN_FILEPATH),
        xmlMessage.data(XN_FILENAME), xmlMessage.data(XN_FILESIZE).toLongLong(), address, _tcpPort, (FileType)type);
    connect(receiver, &FileReceiver::progressUpdated, this, &lmcTcpNetwork::update);
    _receiveList.prepend(receiver);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.initReceiveFile ended"));
}

void lmcTcpNetwork::fileOperation(FileMode mode, const QString &userId, const QString &data) {
    Q_UNUSED(userId);

    XmlMessage xmlMessage(data);

    int fileOp = Helper::indexOf(FileOpNames, FO_Max, xmlMessage.data(XN_FILEOP));
    QString id = xmlMessage.data(XN_FILEID);

    if(mode == FM_Send) {
        FileSender *sender = getSender(id, userId);
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
        FileReceiver *receiver = getReceiver(id, userId);
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

void lmcTcpNetwork::setIPAddress(const QString &szAddress) {
    _ipAddress = QHostAddress(szAddress);
}

void lmcTcpNetwork::server_newConnection() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.server_newConnection started"));

    QTcpSocket* socket = _server.nextPendingConnection();
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
        addMsgSocket(userId, socket);
    } else if(buffer.startsWith("FILE")) {
        //	read transfer id from socket and assign socket to correct file receiver
        QString id(buffer.mid(4, 32)); // 4 is length of "FILE", 32 is length of File Id
        QString userId(buffer.mid(36));
        addFileSocket(id, userId, socket);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.socket_readyRead ended"));
}

void lmcTcpNetwork::msgStream_connectionLost(QString userId) {
    emit connectionLost(userId);
}

void lmcTcpNetwork::update(FileMode mode, FileOp op, FileType type, QString id, QString userId, QString userName, QString data) {
    XmlMessage xmlMessage;
    xmlMessage.addHeader(XN_FROM, userId);
    xmlMessage.addHeader(XN_TO, _localId);
    xmlMessage.addData(XN_MODE, FileModeNames[mode]);
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[type]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[op]);
    xmlMessage.addData(XN_FILEID, id);

    switch(op) {
    case FO_Complete:
    case FO_Error:
        xmlMessage.addData(XN_FILEPATH, data);
        if(mode == FM_Send)
            removeSender(static_cast<FileSender *>(sender()));
        else
            removeReceiver(static_cast<FileReceiver *>(sender()));
        break;
    case FO_Progress:
        xmlMessage.addData(XN_FILESIZE, data);
        break;
    default:
        break;
    }

    QString message = xmlMessage.toString();
    emit progressReceived(userId, userName, message);
}

void lmcTcpNetwork::receiveMessage(QString userId, QString address, QByteArray datagram) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.receiveMessage started-|- message received from %1 on %2").arg(userId, address));

    DatagramHeader header;
    if(!Datagram::getHeader(datagram, header))
        return;

    header.userId = userId;
    header.address = address;
    QByteArray messageData = Datagram::getData(datagram);
    QString message;

    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.receiveMessage-|- TCP stream type %1 received from user %2 at %3").arg(QString::number(header.type), userId, address));

    switch(header.type) {
    case DT_Handshake:
        emit newConnection(header.userId, header.address);
        break;
    case DT_Message:
        message = QString::fromUtf8(messageData.data(), messageData.length());
        emit messageReceived(header, message);
        break;
    default:
        break;
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.receiveMessage ended"));
}

void lmcTcpNetwork::addFileSocket(const QString &Id, const QString userId, QTcpSocket *socket) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addFileSocket started-|- Accepted connection from user %1").arg(userId));

    FileReceiver* receiver = getReceiver(Id, userId);
    if(receiver)
        receiver->init(socket);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addFileSocket ended"));
}

void lmcTcpNetwork::addMsgSocket(const QString &userId, QTcpSocket *socket) {
    LoggerManager::getInstance().writeInfo(QString("lmcTcpNetwork.addMsgSocket started-|- Accepted connection from user %1").arg(userId));

    QString address = socket->peerAddress().toString();
    MsgStream *msgStream = new MsgStream(_localId, userId, address, _tcpPort);
    connect(msgStream, &MsgStream::connectionLost, this, &lmcTcpNetwork::msgStream_connectionLost);
    connect(msgStream,  &MsgStream::messageReceived, this, &lmcTcpNetwork::receiveMessage);
    _messageMap.insert(userId, msgStream);
    msgStream->init(socket);

    sendHandShake(userId);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcTcpNetwork.addMsgSocket ended"));
}

FileSender* lmcTcpNetwork::getSender(QString id, QString userId) {
    for(int index = 0; index < _sendList.count(); index++)
        if(_sendList[index]->id.compare(id) == 0 && _sendList[index]->peerId.compare(userId) == 0)
            return _sendList[index];

    return NULL;
}

FileReceiver* lmcTcpNetwork::getReceiver(QString id, QString userId) {
    for(int index = 0; index < _receiveList.count(); index++)
        if(_receiveList[index]->id.compare(id) == 0 && _receiveList[index]->peerId.compare(userId) == 0)
            return _receiveList[index];

    return NULL;
}

void lmcTcpNetwork::removeSender(FileSender* pSender) {
    int index = _sendList.indexOf(pSender);
    FileSender* sender = _sendList.takeAt(index);
    sender->deleteLater();  // deleting later is generally safer
}

void lmcTcpNetwork::removeReceiver(FileReceiver* pReceiver) {
    int index = _receiveList.indexOf(pReceiver);
    FileReceiver* receiver = _receiveList.takeAt(index);
    receiver->deleteLater();  // deleting later is generally safer
}
