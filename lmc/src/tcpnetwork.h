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


#ifndef TCPNETWORK_H
#define TCPNETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "shared.h"
#include "datagram.h"
#include "settings.h"
#include "netstreamer.h"

class lmcTcpNetwork : public QObject {
    Q_OBJECT

public:
    lmcTcpNetwork();
    ~lmcTcpNetwork() {}

    void init(int nPort = 0);
    void start();
    void stop();
    void setLocalId(const QString &_localId);
    void addConnection(const QString &userId, const QString &address);
    void sendMessage(const QString &receiverId, const QString &data);
    void sendHandShake(const QString &userId);
    void initSendFile(const QString &receiverId, const QString &receiverName, const QString &address, const QString &data);
    void initReceiveFile(const QString &senderId, const QString &senderName, const QString &address, const QString &data);
    void fileOperation(FileMode mode, const QString &userId, const QString &data);
    void settingsChanged();
    void setIPAddress(const QString &szAddress);

signals:
    void newConnection(QString userId, QString address);
    void connectionLost(QString userId);
    void messageReceived(DatagramHeader header, QString data);
    void progressReceived(QString userId, QString userName, QString data);

private slots:
    void server_newConnection();
    void socket_readyRead();
    void msgStream_connectionLost(QString userId);
    void update(FileMode mode, FileOp op, FileType type, QString id, QString userId, QString lpszUserName, QString data);
    void receiveMessage(QString userId, QString address, QByteArray data);

private:
    void addFileSocket(const QString &Id, const QString userId, QTcpSocket *socket);
    void addMsgSocket(const QString &userId, QTcpSocket* socket);
    FileSender* getSender(QString id, QString userId);
    FileReceiver* getReceiver(QString id, QString userId);
    void removeSender(FileSender* pSender);
    void removeReceiver(FileReceiver* pReceiver);

    QTcpServer				  _server;
    QList<FileSender*>		  _sendList;
    QList<FileReceiver*>	  _receiveList;
    QMap<QString, MsgStream*> _messageMap;
    MsgStream*				  _localMsgStream = nullptr;
    bool					  _isRunning;
    int						  _tcpPort;
    QString					  _localId;
    QHostAddress			  _ipAddress = QHostAddress::Null;
};

#endif // TCPNETWORK_H
