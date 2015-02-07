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


#include "udpnetwork.h"
#include "loggermanager.h"

lmcUdpNetwork::lmcUdpNetwork() {
    pUdpReceiver = new QUdpSocket(this);
    pUdpSender = new QUdpSocket(this);
    localId = QString::null;
    canReceive = false;
    isRunning = false;
    ipAddress = QHostAddress::AnyIPv4;
    subnetMask = QHostAddress::AnyIPv4;
    defBroadcast = QHostAddress::Broadcast;
    broadcastList.clear();
}

lmcUdpNetwork::~lmcUdpNetwork() {
}

void lmcUdpNetwork::init(int port) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.init started-|- port: %1").arg(port));

    pSettings = new lmcSettings();
    nUdpPort = port > 0 ? port : pSettings->value(IDS_UDPPORT, IDS_UDPPORT_VAL).toInt();
    multicastAddress = QHostAddress(pSettings->value(IDS_MULTICAST, IDS_MULTICAST_VAL).toString());

    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.init -|- port: %1, multicast address: %2").arg(QString::number(port), multicastAddress.toString()));
    int size = pSettings->beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        QHostAddress address = QHostAddress(pSettings->value(IDS_BROADCAST).toString());
        if(!broadcastList.contains(address))
            broadcastList.append(address);
    }
    pSettings->endArray();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.init ended"));
}

void lmcUdpNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.start started"));

    //	start receiving datagrams
    canReceive = startReceiving();
    isRunning = true;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.start ended"));
}

void lmcUdpNetwork::stop() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.stop started"));

    disconnect(pUdpReceiver, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    if(pUdpReceiver->state() == QAbstractSocket::BoundState) {
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.stop-|- Leaving multicast group %1 on interface %2 - started").arg(multicastAddress.toString(), multicastInterface.humanReadableName()));

        bool left = pUdpReceiver->leaveMulticastGroup(multicastAddress, multicastInterface);
        pUdpReceiver->close();

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.stop-|- Leaving multicast group  %1 on interface %2 - %3").arg(multicastAddress.toString(), multicastInterface.humanReadableName(), left ? "Success" : "Failed"));
    }
    isRunning = false;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.stop ended"));
}

void lmcUdpNetwork::setLocalId(QString* lpszLocalId) {
    localId = *lpszLocalId;
}

void lmcUdpNetwork::sendBroadcast(QString* lpszData) {
    if(!isRunning) {
        LoggerManager::getInstance().writeError(QStringLiteral("lmcUdpNetwork.sendBroadcast -|- UDP server not running. Broadcast not sent"));
        return;
    }

    QByteArray datagram = lpszData->toUtf8();
    sendDatagram(multicastAddress, datagram);
    for(int index = 0; index < broadcastList.count(); index++) {
        sendDatagram(broadcastList.at(index), datagram);
    }
}

void lmcUdpNetwork::settingsChanged() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.settingsChanged ended"));

    QHostAddress address = QHostAddress(pSettings->value(IDS_MULTICAST, IDS_MULTICAST_VAL).toString());
    if(multicastAddress != address) {
        if(pUdpReceiver->state() == QAbstractSocket::BoundState) {
            LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Leaving multicast group %1 on interface %2 - started").arg(multicastAddress.toString(), multicastInterface.humanReadableName()));

            bool left = pUdpReceiver->leaveMulticastGroup(multicastAddress, multicastInterface);

            LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Leaving multicast group  %1 on interface %2 - %3").arg(multicastAddress.toString(), multicastInterface.humanReadableName(), left ? "Success" : "Failed"));
        }
        multicastAddress = address;
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Joining multicast group %1 on interface %2 - started").arg(multicastAddress.toString(), multicastInterface.humanReadableName()));

        bool joined = pUdpReceiver->joinMulticastGroup(multicastAddress, multicastInterface);

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Joining multicast group  %1 on interface %2 - %3").arg(multicastAddress.toString(), multicastInterface.humanReadableName(), joined ? "Success" : "Failed"));
    }
    broadcastList.clear();
    broadcastList.append(defBroadcast);
    int size = pSettings->beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        QHostAddress address = QHostAddress(pSettings->value(IDS_BROADCAST).toString());
        if(!broadcastList.contains(address))
            broadcastList.append(address);
    }
    pSettings->endArray();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.settingsChanged ended"));
}

void lmcUdpNetwork::setMulticastInterface(const QNetworkInterface& networkInterface) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.setMulticastInterface started-|- Interface: %1").arg(networkInterface.humanReadableName()));

    multicastInterface = networkInterface;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setMulticastInterface ended"));
}

void lmcUdpNetwork::setIPAddress(const QString& szAddress, const QString& szSubnet) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.setIPAddress started-|- IP: %1, SubnetMask: %2").arg(szAddress, szSubnet));

    ipAddress = QHostAddress(szAddress);
    subnetMask = QHostAddress(szSubnet);
    setDefaultBroadcast();
    if(!broadcastList.contains(defBroadcast))
        broadcastList.append(defBroadcast);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setIPAddress ended"));
}

void lmcUdpNetwork::processPendingDatagrams() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.processPendingDatagrams started"));

    while(pUdpReceiver->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(pUdpReceiver->pendingDatagramSize());
        QHostAddress address;
        pUdpReceiver->readDatagram(datagram.data(), datagram.size(), &address);
        QString szAddress = address.toString();
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.processPendingDatagrams-|- address: %1").arg(szAddress));
        parseDatagram(&szAddress, datagram);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.processPendingDatagrams ended"));
}

void lmcUdpNetwork::sendDatagram(QHostAddress remoteAddress, QByteArray& datagram) {
    if(!isRunning) {
        LoggerManager::getInstance().writeError(QString("lmcUdpNetwork.sendDatagram-|- Sending UDP datagram failed to %1: %2 - network stopped").arg(remoteAddress.toString(), QString::number(nUdpPort)));
        return;
    }

    pUdpSender->writeDatagram(datagram.data(), datagram.size(), remoteAddress, nUdpPort);
}

bool lmcUdpNetwork::startReceiving() {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving started-|- Binding UDP listener to port %1").arg(QString::number(nUdpPort)));

    if(pUdpReceiver->bind(QHostAddress::AnyIPv4, nUdpPort)) {
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving-|- Joining multicast group %1 on interface %2").arg(multicastAddress.toString(), multicastInterface.humanReadableName()));

        bool joined = pUdpReceiver->joinMulticastGroup(multicastAddress, multicastInterface);
        connect(pUdpReceiver, &QUdpSocket::readyRead, this, &lmcUdpNetwork::processPendingDatagrams);

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving ended-|- Binding UDP listener to port %1: %2").arg(QString::number(nUdpPort), joined ? "Success" : "Failed"));
        return true;
    }

    LoggerManager::getInstance().writeError(QString("lmcUdpNetwork.startReceiving-|- Binding UDP listener to port %1 Failed").arg(QString::number(nUdpPort)));
    return false;
}

void lmcUdpNetwork::parseDatagram(QString* lpszAddress, QByteArray& baDatagram) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.parseDatagram started-|- UDP datagram received from %1").arg(*lpszAddress));

    DatagramHeader* pHeader = new DatagramHeader(DT_Broadcast, QString(), *lpszAddress);
    QString szData = QString::fromUtf8(baDatagram.data(), baDatagram.length());
    emit broadcastReceived(pHeader, &szData);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.parseDatagram ended"));
}

void lmcUdpNetwork::setDefaultBroadcast() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast started"));

    if(ipAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        LoggerManager::getInstance().writeError(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast failed-|- protocol is not IPv4"));
        return;
    }

    //	The network broadcast address is obtained by ORing the host address and
    //	bit inversed subnet mask
    quint32 ipv4 = ipAddress.toIPv4Address();
    quint32 invMask = ~(subnetMask.toIPv4Address());
    defBroadcast = QHostAddress((ipv4 | invMask));

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast ended"));
}
