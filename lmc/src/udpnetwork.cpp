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
#include "globals.h"

lmcUdpNetwork::lmcUdpNetwork() {
    _udpReceiver.setParent(this);
    _udpSender.setParent(this);
}

lmcUdpNetwork::~lmcUdpNetwork() {
}

void lmcUdpNetwork::init(int port) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.init started-|- port: %1").arg(port));

    _udpPort = port > 0 ? port : Globals::getInstance().udpPort();
    _multicastAddress = QHostAddress(Globals::getInstance().multicastAddress());

    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.init -|- port: %1, multicast address: %2").arg(QString::number(port), _multicastAddress.toString()));
    int size = _settings.beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        _settings.setArrayIndex(index);
        QHostAddress address = QHostAddress(_settings.value(IDS_BROADCAST).toString());
        if(!_broadcastList.contains(address))
            _broadcastList.append(address);
    }
    _settings.endArray();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.init ended"));
}

void lmcUdpNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.start started"));

    //	start receiving datagrams
    _canReceive = startReceiving();
    _isRunning = true;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.start ended"));
}

void lmcUdpNetwork::stop() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.stop started"));

    disconnect(&_udpReceiver, &QUdpSocket::readyRead, this, &lmcUdpNetwork::processPendingDatagrams);
    if(_udpReceiver.state() == QAbstractSocket::BoundState) {
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.stop-|- Leaving multicast group %1 on interface %2 - started").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName()));

        bool left = _udpReceiver.leaveMulticastGroup(_multicastAddress, _multicastInterface);
        _udpReceiver.close();

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.stop-|- Leaving multicast group  %1 on interface %2 - %3").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName(), left ? "Success" : "Failed"));
    }
    _isRunning = false;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.stop ended"));
}

void lmcUdpNetwork::setLocalId(const QString &localId) {
    _localId = localId;
}

void lmcUdpNetwork::sendBroadcast(const QString &data) {
    if(!_isRunning) {
        LoggerManager::getInstance().writeError(QStringLiteral("lmcUdpNetwork.sendBroadcast -|- UDP server not running. Broadcast not sent"));
        return;
    }

    QByteArray datagram = data.toUtf8();
    sendDatagram(_multicastAddress, datagram);
    for(int index = 0; index < _broadcastList.count(); index++) {
        sendDatagram(_broadcastList.at(index), datagram);
    }
}

void lmcUdpNetwork::settingsChanged() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.settingsChanged ended"));

    QHostAddress address = QHostAddress(Globals::getInstance().multicastAddress());
    if(_multicastAddress != address) {
        if(_udpReceiver.state() == QAbstractSocket::BoundState) {
            LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Leaving multicast group %1 on interface %2 - started").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName()));

            bool left = _udpReceiver.leaveMulticastGroup(_multicastAddress, _multicastInterface);

            LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Leaving multicast group  %1 on interface %2 - %3").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName(), left ? "Success" : "Failed"));
        }
        _multicastAddress = address;
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Joining multicast group %1 on interface %2 - started").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName()));

        bool joined = _udpReceiver.joinMulticastGroup(_multicastAddress, _multicastInterface);

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.settingsChanged-|- Joining multicast group  %1 on interface %2 - %3").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName(), joined ? "Success" : "Failed"));
    }

    _broadcastList.clear();
    _broadcastList.append(_defaultBroadcast);
    int size = _settings.beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        _settings.setArrayIndex(index);
        QHostAddress address = QHostAddress(_settings.value(IDS_BROADCAST).toString());
        if(!_broadcastList.contains(address))
            _broadcastList.append(address);
    }
    _settings.endArray();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.settingsChanged ended"));
}

void lmcUdpNetwork::setMulticastInterface(const QNetworkInterface& networkInterface) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.setMulticastInterface started-|- Interface: %1").arg(networkInterface.humanReadableName()));

    _multicastInterface = networkInterface;

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setMulticastInterface ended"));
}

void lmcUdpNetwork::setIPAddress(const QString& szAddress, const QString& szSubnet) {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.setIPAddress started-|- IP: %1, SubnetMask: %2").arg(szAddress, szSubnet));

    _ipAddress = QHostAddress(szAddress);
    _subnetMask = QHostAddress(szSubnet);
    setDefaultBroadcast();
    if(!_broadcastList.contains(_defaultBroadcast))
        _broadcastList.append(_defaultBroadcast);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setIPAddress ended"));
}

void lmcUdpNetwork::processPendingDatagrams() {
    while(_udpReceiver.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(_udpReceiver.pendingDatagramSize());
        QHostAddress hostAddress;
        _udpReceiver.readDatagram(datagram.data(), datagram.size(), &hostAddress);
        QString address = hostAddress.toString();
        parseDatagram(address, datagram);
    }
}

void lmcUdpNetwork::sendDatagram(const QHostAddress &remoteAddress, const QByteArray& datagram) {
    if(!_isRunning) {
        LoggerManager::getInstance().writeError(QString("lmcUdpNetwork.sendDatagram-|- Sending UDP datagram failed to %1: %2 - network stopped").arg(remoteAddress.toString(), QString::number(_udpPort)));
        return;
    }

    _udpSender.writeDatagram(datagram.data(), datagram.size(), remoteAddress, _udpPort);
}

bool lmcUdpNetwork::startReceiving() {
    LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving started-|- Binding UDP listener to port %1").arg(QString::number(_udpPort)));

    if(_udpReceiver.bind(QHostAddress::AnyIPv4, _udpPort)) {
        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving-|- Joining multicast group %1 on interface %2").arg(_multicastAddress.toString(), _multicastInterface.humanReadableName()));

        bool joined = _udpReceiver.joinMulticastGroup(_multicastAddress, _multicastInterface);
        connect(&_udpReceiver, &QUdpSocket::readyRead, this, &lmcUdpNetwork::processPendingDatagrams);

        LoggerManager::getInstance().writeInfo(QString("lmcUdpNetwork.startReceiving ended-|- Binding UDP listener to port %1: %2").arg(QString::number(_udpPort), joined ? "Success" : "Failed"));
        return true;
    }

    LoggerManager::getInstance().writeError(QString("lmcUdpNetwork.startReceiving-|- Binding UDP listener to port %1 Failed").arg(QString::number(_udpPort)));
    return false;
}

void lmcUdpNetwork::parseDatagram(const QString &address, QByteArray &datagram) {
    DatagramHeader header (DT_Broadcast, QString(), address);
    QString data = QString::fromUtf8(datagram.data(), datagram.length());
    emit broadcastReceived(header, data);
}

void lmcUdpNetwork::setDefaultBroadcast() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast started"));

    if(_ipAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        LoggerManager::getInstance().writeError(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast failed-|- protocol is not IPv4"));
        return;
    }

    //	The network broadcast address is obtained by ORing the host address and
    //	bit inversed subnet mask
    quint32 ipv4 = _ipAddress.toIPv4Address();
    quint32 invMask = ~(_subnetMask.toIPv4Address());
    _defaultBroadcast = QHostAddress((ipv4 | invMask));

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcUdpNetwork.setDefaultBroadcast ended"));
}
