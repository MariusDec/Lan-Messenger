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

#include "network.h"
#include "loggermanager.h"
#include "globals.h"

lmcNetwork::lmcNetwork() {
    _udpNetwork.setParent(this);
    _tcpNetwork.setParent(this);
    _webNetwork.setParent(this);
    _timer.setParent(this);
    connect(&_udpNetwork, &lmcUdpNetwork::broadcastReceived,
        this, &lmcNetwork::udp_receiveBroadcast);
    connect(&_tcpNetwork, &lmcTcpNetwork::newConnection,
        this, &lmcNetwork::tcp_newConnection);
    connect(&_tcpNetwork, &lmcTcpNetwork::connectionLost,
        this, &lmcNetwork::tcp_connectionLost);
    connect(&_tcpNetwork, &lmcTcpNetwork::messageReceived,
        this, &lmcNetwork::tcp_receiveMessage);
    connect(&_tcpNetwork, &lmcTcpNetwork::progressReceived,
        this, &lmcNetwork::tcp_receiveProgress);
    connect(&_webNetwork, &lmcWebNetwork::messageReceived,
        this, &lmcNetwork::web_receiveMessage);
}

lmcNetwork::~lmcNetwork() {
}

void lmcNetwork::init(const XmlMessage &initParams) {
    LoggerManager::getInstance().writeInfo(QString("lmcNetwork.init started-|- init parameters: %1").arg(initParams.toString()));

    isConnected = getIPAddress();

    LoggerManager::getInstance().writeInfo(QString("lmcNetwork.init -|- Network interface selected: %1\n\tIP address obtained: %2\n\tSubnet mask obtained: %3\n\tConnection status: %4").arg((_networkInterface.isValid() ? _networkInterface.humanReadableName() : "None"), (ipAddress.isEmpty() ? "NULL" : ipAddress), (subnetMask.isEmpty() ? "NULL" : subnetMask), (isConnected ? "OK" : "Fail")));

    int port = initParams.data(XN_PORT).toInt();

    _udpNetwork.init(port);
    _tcpNetwork.init(port);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.init ended"));
}

void lmcNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.start started"));

    connect(&_timer, &QTimer::timeout, this, &lmcNetwork::timer_timeout);
    _timer.start(2000);

    if(isConnected) {
        _udpNetwork.setMulticastInterface(_networkInterface);
        _udpNetwork.setIPAddress(ipAddress, subnetMask);
        _udpNetwork.start();
        _tcpNetwork.setIPAddress(ipAddress);
        _tcpNetwork.start();
        canReceive = _udpNetwork.canReceive();
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.start ended"));
}

void lmcNetwork::stop() {
     LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.stop started"));

    _timer.stop();

    _udpNetwork.stop();
    _tcpNetwork.stop();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.stop ended"));
}

QString lmcNetwork::physicalAddress() {
    if(_networkInterface.isValid())
        return _networkInterface.hardwareAddress();

    return QString::null;
}

QString lmcNetwork::IPAddress() {
    return ipAddress;
}

void lmcNetwork::setLocalId(const QString &localId) {
    _udpNetwork.setLocalId(localId);
    _tcpNetwork.setLocalId(localId);
}

void lmcNetwork::sendBroadcast(const QString &data) {
    _udpNetwork.sendBroadcast(data);
}

void lmcNetwork::addConnection(const QString &userId, const QString &address) {
    _tcpNetwork.addConnection(userId, address);
}

void lmcNetwork::sendMessage(const QString &receiverId, const QString &data) {
    _tcpNetwork.sendMessage(receiverId, data);
}

void lmcNetwork::initSendFile(const QString &receiverId, const QString &receiverName, const QString address, const QString &data) {
    _tcpNetwork.initSendFile(receiverId, receiverName, address, data);
}

void lmcNetwork::initReceiveFile(const QString &senderId, const QString &senderName, const QString &address, const QString &data) {
    _tcpNetwork.initReceiveFile(senderId, senderName, address, data);
}

void lmcNetwork::fileOperation(FileMode mode, const QString &userId, const QString &data) {
    _tcpNetwork.fileOperation(mode, userId, data);
}

void lmcNetwork::sendWebMessage(const QString &url) {
    _webNetwork.sendMessage(url);
}

void lmcNetwork::settingsChanged() {
    _udpNetwork.settingsChanged();
    _tcpNetwork.settingsChanged();
}

void lmcNetwork::timer_timeout() {
    bool prev = isConnected;
    isConnected = getIPAddress();

    if(prev != isConnected) {
        LoggerManager::getInstance().writeInfo(QString("lmcNetwork.timer_timeout -|- Network interface selected: %1 \n\tIP address obtained: %2 \n\tSubnet mask obtained: %3 \n\tConnection status: %4").arg((_networkInterface.isValid() ? _networkInterface.humanReadableName() : "None"), (ipAddress.isEmpty() ? "NULL" : ipAddress), (subnetMask.isEmpty() ? "NULL" : subnetMask), (isConnected ? "OK" : "Fail")));

        if(isConnected) {
            _udpNetwork.setMulticastInterface(_networkInterface);
            _udpNetwork.setIPAddress(ipAddress, subnetMask);
            _udpNetwork.start();
            _tcpNetwork.setIPAddress(ipAddress);
            _tcpNetwork.start();
            canReceive = _udpNetwork.canReceive();
        } else {
            _udpNetwork.stop();
            _tcpNetwork.stop();
        }
        emit connectionStateChanged();
    }
}

void lmcNetwork::udp_receiveBroadcast(DatagramHeader header, QString data) {
    emit broadcastReceived(header, data);
}

void lmcNetwork::tcp_newConnection(QString userId, QString address) {
    emit newConnection(userId, address);
}

void lmcNetwork::tcp_connectionLost(QString userId) {
    emit connectionLost(userId);
}

void lmcNetwork::tcp_receiveMessage(DatagramHeader header, QString data) {
    emit messageReceived(header, data);
}

void lmcNetwork::tcp_receiveProgress(QString userId, QString userName, QString data) {
    emit progressReceived(userId, userName, data);
}

void lmcNetwork::web_receiveMessage(QString data) {
    emit webMessageReceived(data);
}

bool lmcNetwork::getIPAddress(bool getLanAddress) {
    QNetworkAddressEntry addressEntry;
     // If an interface is already being used, get it. Ignore all others
    _networkInterface = QNetworkInterface::interfaceFromName(_interfaceName);
    if(_networkInterface.isValid()) {
        if(isInterfaceUp(_networkInterface) && ((getLanAddress && getIPAddress(_networkInterface, addressEntry, getLanAddress)) || (!getLanAddress && getIPAddress(_networkInterface, addressEntry, false)))) {
            ipAddress = addressEntry.ip().toString();
            subnetMask = addressEntry.netmask().toString();
            LoggerManager::getInstance().writeInfo(QString("lmcNetwork.getIPAddress -|- Active network interface found: %1").arg(_networkInterface.humanReadableName()));
            return true;
        }
    }

    //	get a list of all network interfaces available in the system
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    bool activeFound = false;

    //	return the preferred interface if it is active
    for(int index = 0; index < allInterfaces.count(); index++) {
        if(isInterfaceUp(allInterfaces[index])) {
            activeFound = true;
            if(getIPAddress(allInterfaces[index], addressEntry, getLanAddress)) {
                ipAddress = addressEntry.ip().toString();
                subnetMask = addressEntry.netmask().toString();
                _networkInterface = allInterfaces[index];
                _interfaceName = allInterfaces[index].name();
                LoggerManager::getInstance().writeInfo(QString("lmcNetwork.getIPAddress -|- Active network interface found: %1").arg(allInterfaces[index].humanReadableName()));
                return true;
            }
        }
    }

    if (getLanAddress && getIPAddress(false))
        return true;

    LoggerManager::getInstance().writeWarning(QString("lmcNetwork.getIPAddress -|- %1").arg((activeFound ? "No IP address found" : "No active network interface found")));
    ipAddress = QString::null;
    subnetMask = QString::null;
    return false;
}

bool lmcNetwork::getIPAddress(const QNetworkInterface &networkInterface, QNetworkAddressEntry &addressEntry, bool getLanAddress) {
    //	get a list of all associated ip addresses of the interface
    QList<QNetworkAddressEntry> addressEntries = networkInterface.addressEntries();

    QNetworkAddressEntry *firstIp4Address = nullptr;

    //	return the first LAN address (e.g. 192.168.1.4) or the first address which is an ipv4 address
    for(int index = 0; index < addressEntries.count(); index++) {
        if(addressEntries[index].ip().protocol() == QAbstractSocket::IPv4Protocol) {
            if (addressEntries[index].ip().toString().section('.', -2, -2) == "1") {
                addressEntry = addressEntries[index];
                return true;
            }
            if (!getLanAddress && !firstIp4Address)
                firstIp4Address = &addressEntries[index];
        }
    }

    if (firstIp4Address) {
        addressEntry = *firstIp4Address;
        return true;
    }

    return false;
}

bool lmcNetwork::isInterfaceUp(const QNetworkInterface &networkInterface) {
    if(networkInterface.flags().testFlag(QNetworkInterface::IsUp)
        && networkInterface.flags().testFlag(QNetworkInterface::IsRunning)
        && !networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
    }

    return false;
}
