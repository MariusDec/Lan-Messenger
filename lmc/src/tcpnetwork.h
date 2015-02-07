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
#include "crypto.h"

class lmcTcpNetwork : public QObject {
    Q_OBJECT

public:
    lmcTcpNetwork();
    ~lmcTcpNetwork() {}

    void init(int nPort = 0);
    void start();
    void stop();
    void setLocalId(QString* lpszLocalId);
    void addConnection(QString* lpszUserId, QString* lpszAddress);
    void sendMessage(QString* lpszReceiverId, QString* lpszData);
    void sendHandShake(QString *lpszUserId);
    void initSendFile(QString* lpszReceiverId, QString *lpszReceiverName, QString* lpszAddress, QString* lpszData);
    void initReceiveFile(QString* lpszSenderId, QString *lpszSenderName, QString* lpszAddress, QString* lpszData);
    void fileOperation(FileMode mode, QString* lpszUserId, QString* lpszData);
    void settingsChanged();
    void setIPAddress(const QString& szAddress);

signals:
    void newConnection(QString* lpszUserId, QString* lpszAddress);
    void connectionLost(QString* lpszUserId);
    void messageReceived(DatagramHeader* pHeader, QString* lpszData);
    void progressReceived(QString* lpszUserId, QString *lpszUserName, QString* lpszData);

private slots:
    void server_newConnection();
    void socket_readyRead();
    void msgStream_connectionLost(QString* lpszUserId);
    void update(FileMode mode, FileOp op, FileType type, QString* lpszId, QString* lpszUserId, QString *lpszUserName, QString* lpszData);
    void receiveMessage(QString* lpszUserId, QString* lpszAddress, QByteArray& data);

private:
    void addFileSocket(QString* lpszId, QString *lpszUserId, QTcpSocket *pSocket);
    void addMsgSocket(QString* lpszUserId, QTcpSocket* pSocket);
    FileSender* getSender(QString id, QString userId);
    FileReceiver* getReceiver(QString id, QString userId);
    void removeSender(FileSender* pSender);
    void removeReceiver(FileReceiver* pReceiver);

    QTcpServer*				  _server;
    QList<FileSender*>		  sendList;
    QList<FileReceiver*>	  receiveList;
    QMap<QString, MsgStream*> messageMap;
    MsgStream*				  locMsgStream;
    lmcSettings*			  pSettings;
    bool					  isRunning;
    int						  tcpPort;
    QString					  localId;
    QHostAddress			  ipAddress;
};

#endif // TCPNETWORK_H
