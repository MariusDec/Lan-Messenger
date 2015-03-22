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


#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QTimer>
#include "udpnetwork.h"
#include "tcpnetwork.h"
#include "webnetwork.h"
#include "settings.h"
#include "xmlmessage.h"

class lmcNetwork : public QObject {
    Q_OBJECT

public:
    lmcNetwork();
    ~lmcNetwork();

    void init(const XmlMessage &initParams);
    void start();
    void stop();
    QString physicalAddress();
    QString IPAddress();
    void setLocalId(const QString &localId);
    void sendBroadcast(const QString &data);
    void addConnection(const QString &userId, const QString &address);
    void sendMessage(const QString &receiverId, const QString &data);
    void initSendFile(const QString &receiverId, const QString &receiverName, const QString address, const QString &data);
    void initReceiveFile(const QString &senderId, const QString &senderName, const QString &address, const QString &data);
    void fileOperation(FileMode mode, const QString &userId, const QString &data);
    void sendWebMessage(const QString &url);
    void settingsChanged();

    QString	ipAddress;
    QString	subnetMask;
    bool	isConnected = false;
    bool	canReceive = false;

signals:
    void connectionStateChanged();
    void broadcastReceived(DatagramHeader header, QString data);
    void newConnection(QString userId, QString address);
    void connectionLost(QString userId);
    void messageReceived(DatagramHeader header, QString data);
    void progressReceived(QString userId, QString userName, QString data);
    void webMessageReceived(QString data);

private slots:
    void timer_timeout();
    void udp_receiveBroadcast(DatagramHeader header, QString data);
    void tcp_newConnection(QString userId, QString address);
    void tcp_connectionLost(QString userId);
    void tcp_receiveMessage(DatagramHeader header, QString data);
    void tcp_receiveProgress(QString userId, QString userName, QString data);
    void web_receiveMessage(QString data);

private:
    bool getIPAddress(bool getLanAddress = true);
    bool getIPAddress(const QNetworkInterface &networkInterface, QNetworkAddressEntry &addressEntry, bool getLanAddress);
    bool isInterfaceUp(const QNetworkInterface &networkInterface);

    struct NetworkAdapter {
        QString name;
        QString description;
        QString type;
    };

    lmcUdpNetwork			_udpNetwork;
    lmcTcpNetwork			_tcpNetwork;
    lmcWebNetwork			_webNetwork;
    QTimer					_timer;
    QString					_interfaceName;
    QNetworkInterface		_networkInterface;
};

#endif // NETWORK_H
