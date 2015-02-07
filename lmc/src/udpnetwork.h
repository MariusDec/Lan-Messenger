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


#ifndef UDPNETWORK_H
#define UDPNETWORK_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QList>
#include "shared.h"
#include "datagram.h"
#include "settings.h"

class lmcUdpNetwork : public QObject {
    Q_OBJECT

public:
    lmcUdpNetwork();
    ~lmcUdpNetwork();

    void init(int port = 0);
    void start();
    void stop();
    void setLocalId(QString* lpszLocalId);
    void sendBroadcast(QString* lpszData);
    void settingsChanged();
    void setMulticastInterface(const QNetworkInterface& networkInterface);
    void setIPAddress(const QString& szAddress, const QString& szSubnet);

    bool isConnected;
    bool canReceive;

signals:
    void broadcastReceived(DatagramHeader* pHeader, QString* lpszData);
    void connectionStateChanged();

private slots:
    void processPendingDatagrams();

private:
    void sendDatagram(QHostAddress remoteAddress, QByteArray& baDatagram);
    bool startReceiving();
    void parseDatagram(QString* lpszAddress, QByteArray& baDatagram);
    void setDefaultBroadcast();

    lmcSettings*		pSettings;
    QUdpSocket*			pUdpReceiver;
    QUdpSocket*			pUdpSender;

    bool				isRunning;
    int					nUdpPort;
    QHostAddress		multicastAddress;
    QString				localId;
    QNetworkInterface	multicastInterface;
    QHostAddress		ipAddress;
    QHostAddress		subnetMask;
    QList<QHostAddress>	broadcastList;
    QHostAddress		defBroadcast;
};

#endif // UDPNETWORK_H
