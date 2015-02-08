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

lmcNetwork::lmcNetwork() {
    pUdpNetwork = new lmcUdpNetwork();
    pTcpNetwork = new lmcTcpNetwork();
    pWebNetwork = new lmcWebNetwork();
    connect(pUdpNetwork, &lmcUdpNetwork::broadcastReceived,
        this, &lmcNetwork::udp_receiveBroadcast);
    connect(pTcpNetwork, &lmcTcpNetwork::newConnection,
        this, &lmcNetwork::tcp_newConnection);
    connect(pTcpNetwork, &lmcTcpNetwork::connectionLost,
        this, &lmcNetwork::tcp_connectionLost);
    connect(pTcpNetwork, &lmcTcpNetwork::messageReceived,
        this, &lmcNetwork::tcp_receiveMessage);
    connect(pTcpNetwork, &lmcTcpNetwork::progressReceived,
        this, &lmcNetwork::tcp_receiveProgress);
    connect(pWebNetwork, &lmcWebNetwork::messageReceived,
        this, &lmcNetwork::web_receiveMessage);
    pTimer = NULL;
    ipAddress = QString::null;
    subnetMask = QString::null;
    networkInterface = QNetworkInterface();
    interfaceName = QString::null;
    isConnected = false;
    canReceive = false;
}

lmcNetwork::~lmcNetwork() {
}

void lmcNetwork::init(XmlMessage *pInitParams) {
    LoggerManager::getInstance().writeInfo(QString("lmcNetwork.init started-|- init parameters: %1").arg(pInitParams->toString()));

    pSettings = new lmcSettings();
    isConnected = getIPAddress();

    LoggerManager::getInstance().writeInfo(QString("lmcNetwork.init -|- Network interface selected: %1\n\tIP address obtained: %2\n\tSubnet mask obtained: %3\n\tConnection status: %4").arg((networkInterface.isValid() ? networkInterface.humanReadableName() : "None"), (ipAddress.isEmpty() ? "NULL" : ipAddress), (subnetMask.isEmpty() ? "NULL" : subnetMask), (isConnected ? "OK" : "Fail")));

    int port = pInitParams->data(XN_PORT).toInt();

    pUdpNetwork->init(port);
    pTcpNetwork->init(port);

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.init ended"));
}

void lmcNetwork::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.start started"));

    pTimer = new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(timer_timeout()));
    pTimer->start(2000);

    if(isConnected) {
        pUdpNetwork->setMulticastInterface(networkInterface);
        pUdpNetwork->setIPAddress(ipAddress, subnetMask);
        pUdpNetwork->start();
        pTcpNetwork->setIPAddress(ipAddress);
        pTcpNetwork->start();
        canReceive = pUdpNetwork->canReceive;
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.start ended"));
}

void lmcNetwork::stop() {
     LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.stop started"));

    pTimer->stop();

    pUdpNetwork->stop();
    pTcpNetwork->stop();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.stop ended"));
}

QString lmcNetwork::physicalAddress() {
    if(networkInterface.isValid())
        return networkInterface.hardwareAddress();

    return QString::null;
}

QString lmcNetwork::IPAddress() {
    return ipAddress;
}

void lmcNetwork::setLocalId(QString* lpszLocalId) {
    pUdpNetwork->setLocalId(lpszLocalId);
    pTcpNetwork->setLocalId(lpszLocalId);
}

void lmcNetwork::sendBroadcast(QString* lpszData) {
    pUdpNetwork->sendBroadcast(lpszData);
}

void lmcNetwork::addConnection(QString* lpszUserId, QString* lpszAddress) {
    pTcpNetwork->addConnection(lpszUserId, lpszAddress);
}

void lmcNetwork::sendMessage(QString* lpszReceiverId, QString* lpszAddress, QString* lpszData) {
    Q_UNUSED(lpszAddress);
    pTcpNetwork->sendMessage(lpszReceiverId, lpszData);
}

void lmcNetwork::initSendFile(QString* lpszReceiverId, QString *lpszReceiverName, QString* lpszAddress, QString* lpszData) {
    pTcpNetwork->initSendFile(lpszReceiverId, lpszReceiverName, lpszAddress, lpszData);
}

void lmcNetwork::initReceiveFile(QString* lpszSenderId, QString* lpszSenderName, QString* lpszAddress, QString* lpszData) {
    pTcpNetwork->initReceiveFile(lpszSenderId, lpszSenderName, lpszAddress, lpszData);
}

void lmcNetwork::fileOperation(FileMode mode, QString* lpszUserId, QString* lpszData) {
    pTcpNetwork->fileOperation(mode, lpszUserId, lpszData);
}

void lmcNetwork::sendWebMessage(QString *lpszUrl, QString *lpszData) {
    pWebNetwork->sendMessage(lpszUrl, lpszData);
}

void lmcNetwork::settingsChanged() {
    pUdpNetwork->settingsChanged();
    pTcpNetwork->settingsChanged();
}

void lmcNetwork::timer_timeout() {
    bool prev = isConnected;
    isConnected = getIPAddress();

    if(prev != isConnected) {
        LoggerManager::getInstance().writeInfo(QString("lmcNetwork.timer_timeout -|- Network interface selected: %1 \n\tIP address obtained: %2 \n\tSubnet mask obtained: %3 \n\tConnection status: %4").arg((networkInterface.isValid() ? networkInterface.humanReadableName() : "None"), (ipAddress.isEmpty() ? "NULL" : ipAddress), (subnetMask.isEmpty() ? "NULL" : subnetMask), (isConnected ? "OK" : "Fail")));

        if(isConnected) {
            pUdpNetwork->setMulticastInterface(networkInterface);
            pUdpNetwork->setIPAddress(ipAddress, subnetMask);
            pUdpNetwork->start();
            pTcpNetwork->setIPAddress(ipAddress);
            pTcpNetwork->start();
            canReceive = pUdpNetwork->canReceive;
        } else {
            pUdpNetwork->stop();
            pTcpNetwork->stop();
        }
        emit connectionStateChanged();
    }
}

void lmcNetwork::udp_receiveBroadcast(DatagramHeader* pHeader, QString* lpszData) {
    emit broadcastReceived(pHeader, lpszData);
}

void lmcNetwork::tcp_newConnection(QString* lpszUserId, QString* lpszAddress) {
    LoggerManager::getInstance().writeInfo("TCP new connection emited");
    emit newConnection(lpszUserId, lpszAddress);
}

void lmcNetwork::tcp_connectionLost(QString* lpszUserId) {
    emit connectionLost(lpszUserId);
}

void lmcNetwork::tcp_receiveMessage(DatagramHeader* pHeader, QString* lpszData) {
    emit messageReceived(pHeader, lpszData);
}

void lmcNetwork::tcp_receiveProgress(QString* lpszUserId, QString *lpszUserName, QString* lpszData) {
    emit progressReceived(lpszUserId, lpszUserName, lpszData);
}

void lmcNetwork::web_receiveMessage(QString *lpszData) {
    emit webMessageReceived(lpszData);
}

bool lmcNetwork::getIPAddress() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.getIPAddress started -|- Checking for active network interface..."));

     // If an interface is already being used, get it. Ignore all others
    networkInterface = QNetworkInterface::interfaceFromName(interfaceName);
    if(networkInterface.isValid()) {
        QNetworkAddressEntry addressEntry;
        if(isInterfaceUp(&networkInterface) && getIPAddress(&networkInterface, &addressEntry)) {
            ipAddress = addressEntry.ip().toString();
            subnetMask = addressEntry.netmask().toString();
            LoggerManager::getInstance().writeInfo(QString("lmcNetwork.getIPAddress ended -|- Active network interface found: %1").arg(networkInterface.humanReadableName()));
            return true;
        }
        ipAddress = QString::null;
        subnetMask = QString::null;
        return false;
    }


    //	get a list of all network interfaces available in the system
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

    bool activeFound = false;

    //	return the preferred interface if it is active
    for(int index = 0; index < allInterfaces.count(); index++) {
        if(isInterfaceUp(&allInterfaces[index])) {
            activeFound = true;
            QNetworkAddressEntry addressEntry;
            if(getIPAddress(&allInterfaces[index], &addressEntry)) {
                ipAddress = addressEntry.ip().toString();
                subnetMask = addressEntry.netmask().toString();
                networkInterface = allInterfaces[index];
                interfaceName = allInterfaces[index].name();
                LoggerManager::getInstance().writeInfo(QString("lmcNetwork.getIPAddress ended -|- Active network interface found: %1").arg(allInterfaces[index].humanReadableName()));
                return true;
            }
        }
    }

    LoggerManager::getInstance().writeWarning(QString("lmcNetwork.getIPAddress ended -|- %1").arg((activeFound ? "No IP address found" : "No active network interface found")));
    ipAddress = QString::null;
    subnetMask = QString::null;
    return false;
}

bool lmcNetwork::getIPAddress(QNetworkInterface* pNetworkInterface, QNetworkAddressEntry *pAddressEntry) {
    //	get a list of all associated ip addresses of the interface
    QList<QNetworkAddressEntry> addressEntries = pNetworkInterface->addressEntries();
    //	return the first address which is an ipv4 address
    for(int index = 0; index < addressEntries.count(); index++) {
        if(addressEntries[index].ip().protocol() == QAbstractSocket::IPv4Protocol) {
            *pAddressEntry = addressEntries[index];
            return true;
        }
    }
    // if ipv4 address is not present, check for ipv6 address
    for(int index = 0; index < addressEntries.count(); index++) {
        if(addressEntries[index].ip().protocol() == QAbstractSocket::IPv6Protocol) {
            *pAddressEntry = addressEntries[index];
            return true;
        }
    }

    return false;
}

bool lmcNetwork::getNetworkInterface(QNetworkInterface* pNetworkInterface) {
    // If an interface is already being used, get it. Ignore all others
    if(networkInterface.isValid()) {
        *pNetworkInterface = networkInterface;
        return isInterfaceUp(pNetworkInterface);
    }

    // Get the preferred interface name from settings if checking for the first time
    if(interfaceName.isNull())
        interfaceName = pSettings->value(IDS_CONNECTION, IDS_CONNECTION_VAL).toString();

    QString szPreferred = interfaceName;
    // Currently, hard coding usePreferred to False, since using preferred connection is not
    // working properly.
    //bool usePreferred = (szPreferred.compare(IDS_CONNECTION_VAL, Qt::CaseInsensitive) != 0);
    bool usePreferred = false;

    // Return true if preferred interface is available
    if(usePreferred && getNetworkInterface(pNetworkInterface, &szPreferred))
        return true;

    // Return true if a fallback interface is available
    if(!usePreferred && getNetworkInterface(pNetworkInterface, NULL))
        return true;

    return false;
}

bool lmcNetwork::getNetworkInterface(QNetworkInterface* pNetworkInterface, QString* lpszPreferred) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcNetwork.getNetworkInterface started -|- Checking for active network interface..."));

    //	get a list of all network interfaces available in the system
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

    //	return the preferred interface if it is active
    for(int index = 0; index < allInterfaces.count(); index++) {
        // Skip to the next interface if it is not the preferred one
        // Checked only if searching for the preferred adapter
        if(lpszPreferred && lpszPreferred->compare(allInterfaces[index].name()) != 0)
            continue;

        if(isInterfaceUp(&allInterfaces[index])) {
            *pNetworkInterface = allInterfaces[index];
            LoggerManager::getInstance().writeInfo(QString("lmcNetwork.getNetworkInterface ended -|- Active network interface found: %1").arg(pNetworkInterface->humanReadableName()));
            return true;
        }
    }

    LoggerManager::getInstance().writeWarning(QStringLiteral("lmcNetwork.getNetworkInterface ended -|- No active network interface found"));
    return false;
}

bool lmcNetwork::isInterfaceUp(QNetworkInterface* pNetworkInterface) {
    if(pNetworkInterface->flags().testFlag(QNetworkInterface::IsUp)
        && pNetworkInterface->flags().testFlag(QNetworkInterface::IsRunning)
        && !pNetworkInterface->flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
    }

    return false;
}

bool lmcNetwork::getNetworkAddressEntry(QNetworkAddressEntry* pAddressEntry) {
    //	get the first active network interface
    QNetworkInterface networkInterface;

    if(getNetworkInterface(&networkInterface)) {
        //lmcTrace::write("Querying IP address from network interface...");

        //	get a list of all associated ip addresses of the interface
        QList<QNetworkAddressEntry> addressEntries = networkInterface.addressEntries();
        //	return the first address which is an ipv4 address
        for(int index = 0; index < addressEntries.count(); index++) {
            if(addressEntries[index].ip().protocol() == QAbstractSocket::IPv4Protocol) {
                *pAddressEntry = addressEntries[index];
                this->networkInterface = networkInterface;
                this->interfaceName = networkInterface.name();
                //lmcTrace::write("IPv4 address found for network interface.");
                return true;
            }
        }
        // if ipv4 address is not present, check for ipv6 address
        for(int index = 0; index < addressEntries.count(); index++) {
            if(addressEntries[index].ip().protocol() == QAbstractSocket::IPv6Protocol) {
                *pAddressEntry = addressEntries[index];
                this->networkInterface = networkInterface;
                this->interfaceName = networkInterface.name();
                //lmcTrace::write("IPv6 address found for network interface.");
                return true;
            }
        }

        //lmcTrace::write("Warning: No IP address found for network interface.");
    }

    return false;
}
