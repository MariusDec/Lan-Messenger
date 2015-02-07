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

#include "netstreamer.h"
#include "loggermanager.h"

#include <QDir>
#include <QFileInfo>

const qint64 bufferSize = 65535;

/****************************************************************************
** Class: FileSender
** Description: Handles sending files.
****************************************************************************/
FileSender::FileSender() {
}

FileSender::FileSender(const QString &szId, const QString &szLocalId, const QString &szPeerId, const QString &szPeerName, const QString &szFilePath,
    const QString &szFileName, qint64 nFileSize, const QString &szAddress, int nPort, const FileType &nType) {

        id = szId;
        localId = szLocalId;
        peerId = szPeerId;
        peerName = szPeerName;
        filePath = szFilePath;
        fileName = szFileName;
        fileSize = nFileSize;
        address = szAddress;
        port = nPort;
        active = false;
        mile = fileSize / 36;
        milestone = mile;
        file = NULL;
        socket = NULL;
        timer = NULL;
        type = nType;
}

FileSender::~FileSender() {
}

void FileSender::init() {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &FileSender::connected);
    connect(socket, &QTcpSocket::disconnected, this, &FileSender::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &FileSender::readyRead);
    connect(socket, &QTcpSocket::bytesWritten, this, &FileSender::bytesWritten);

    QHostAddress hostAddress(address);
    socket->connectToHost(hostAddress, port);
}

void FileSender::stop() {
    active = false;

    if(timer)
        timer->stop();
    if(file && file->isOpen())
        file->close();
    if(socket && socket->isOpen())
        socket->close();
}

void FileSender::connected() {
    QByteArray data = QString("FILE%0%1").arg(id, localId).toLocal8Bit();	// insert indicator that this socket handles file transfer
    //	send an id message and then wait for a START message
    //	from receiver, which will trigger readyRead signal
    socket->write(data);
}

void FileSender::disconnected() {
    if(active) {
        QString data;
        emit progressUpdated(FM_Send, FO_Error, type, &id, &peerId, &peerName, &data);
    }
}

void FileSender::readyRead() {
    //	message received from receiver, start sending the file
    sendFile();
}

void FileSender::timer_timeout() {
    if(!active)
        return;

    QString transferred = QString::number(file->pos());
    emit progressUpdated(FM_Send, FO_Progress, type, &id, &peerId, &peerName, &transferred);
}

void FileSender::bytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);

    if(!active)
        return;

    qint64 unsentBytes = fileSize - file->pos();

    if(unsentBytes == 0) {
        active = false;
        file->close();
        socket->close();
        emit progressUpdated(FM_Send, FO_Complete, type, &id, &peerId, &peerName, &filePath);
        return;
    }

    qint64 bytesToSend = (bufferSize < unsentBytes) ? bufferSize : unsentBytes;
    qint64 bytesRead = file->read(buffer, bytesToSend);
    socket->write(buffer, bytesRead);

//	if(file->pos() > milestone) {
//		QString transferred = QString::number(file->pos());
//		emit progressUpdated(FM_Send, FO_Progress, type, &id, &peerId, &transferred);
//		milestone += mile;
//	}
}

void FileSender::sendFile() {
    LoggerManager::getInstance().writeInfo(QString("MsgStream.sendFile started -|- Sending file to %1 (%2) on %3").arg(peerId, peerName, address));

    file = new QFile(filePath);

    if(file->open(QIODevice::ReadOnly)) {
        buffer = new char[bufferSize];
        active = true;

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &FileSender::timer_timeout);
        timer->start(PROGRESS_TIMEOUT);

        qint64 unsentBytes = fileSize - file->pos();
        qint64 bytesToSend = (bufferSize < unsentBytes) ? bufferSize : unsentBytes;
        qint64 bytesRead = file->read(buffer, bytesToSend);
        socket->write(buffer, bytesRead);
    } else {
        socket->close();
        QString data;
        emit progressUpdated(FM_Send, FO_Error, type, &id, &peerId, &peerName, &data);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.sendFile ended"));
}


/****************************************************************************
** Class: FileReceiver
** Description: Handles receiving files.
****************************************************************************/
FileReceiver::FileReceiver() {
}

FileReceiver::FileReceiver(const QString &szId, const QString &szPeerId, const QString &szPeerName, const QString &szFilePath, const QString &szFileName,
    qint64 nFileSize, const QString &szAddress, int nPort, const FileType &nType) {

        id = szId;
        peerId = szPeerId;
        peerName = szPeerName;
        filePath = szFilePath;
        fileName = szFileName;
        fileSize = nFileSize;
        address = szAddress;
        port = nPort;
        active = false;
        mile = fileSize / 36;
        milestone = mile;
        file = NULL;
        socket = NULL;
        timer = NULL;
        type = nType;
        lastPosition = 0;
        numTimeOuts = 0;
}

FileReceiver::~FileReceiver() {
}

void FileReceiver::init(QTcpSocket* socket) {
    this->socket = socket;
    connect(socket, &QTcpSocket::disconnected, this, &FileReceiver::disconnected);
    connect(this->socket, &QTcpSocket::readyRead, this, &FileReceiver::readyRead);

    receiveFile();
    //	now send a START message to sender
    socket->write("START");
}

void FileReceiver::stop() {
    bool deleteFile = false;

    active = false;

    if(timer)
        timer->stop();
    if(file && file->isOpen()) {
        deleteFile = (file->pos() < fileSize);
        file->close();
    }
    if(socket && socket->isOpen())
        socket->close();

    if(deleteFile)
        QFile::remove(filePath);
}

void FileReceiver::disconnected() {
    if(active) {
        QString data;
        emit progressUpdated(FM_Receive, FO_Error, type, &id, &peerId, &peerName, &data);
    }
}

void FileReceiver::readyRead() {
    if(!active)
        return;

    qint64 bytesReceived = socket->read(buffer, bufferSize);
    file->write(buffer, bytesReceived);

    qint64 unreceivedBytes = fileSize - file->pos();
    if(unreceivedBytes == 0) {
        active = false;
        file->close();
        socket->close();
        emit progressUpdated(FM_Receive, FO_Complete, type, &id, &peerId, &peerName, &filePath);
        return;
    }

//	if(file->pos() > milestone) {
//		QString transferred = QString::number(file->pos());
//		emit progressUpdated(FM_Receive, FO_Progress, type, &id, &peerId, &transferred);
//		milestone += mile;
//	}
}

void FileReceiver::timer_timeout() {
    if(!active)
        return;

    if(lastPosition < file->pos()) {
        lastPosition = file->pos();
        numTimeOuts = 0;
    } else {
        numTimeOuts++;
        if(numTimeOuts > 20) {
            QString data;
            emit progressUpdated(FM_Receive, FO_Error, type, &id, &peerId, &peerName, &data);
            stop();
            return;
        }
    }

    QString transferred = QString::number(file->pos());
    emit progressUpdated(FM_Receive, FO_Progress, type, &id, &peerId, &peerName, &transferred);
}

void FileReceiver::receiveFile() {
    LoggerManager::getInstance().writeInfo(QString("MsgStream.receiveFile started -|- Receiving file from %1 (%2) on %3").arg(peerId, peerName, address));

    QDir dir = QFileInfo(filePath).dir();
    if(!dir.exists())
        dir.mkpath(dir.absolutePath());

    file = new QFile(filePath);

    if(file->open(QIODevice::WriteOnly)) {
        buffer = new char[bufferSize];
        active = true;

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &FileReceiver::timer_timeout);
        timer->start(PROGRESS_TIMEOUT);
    } else {
        socket->close();
        emit progressUpdated(FM_Receive, FO_Error, type, &id, &peerId, &peerName, &filePath);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.receiveFile ended"));
}

#include <QMessageBox>

/****************************************************************************
** Class: MsgStream
** Description: Handles transmission and reception of TCP streaming messages.
****************************************************************************/
MsgStream::MsgStream() {
    _socket = NULL;
    reading = false;
}

MsgStream::MsgStream(QString szLocalId, QString szPeerId, QString szPeerAddress, int nPort) {
    localId = szLocalId;
    peerId = szPeerId;
    peerAddress = szPeerAddress;
    port = nPort;
    _socket = NULL;
    reading = false;
    outDataLen = 0;
    inDataLen = 0;
}

MsgStream::~MsgStream() {
}

void MsgStream::init() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.init started"));

    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &MsgStream::connected);
    connect(_socket, &QTcpSocket::disconnected, this, &MsgStream::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &MsgStream::readyRead);
    connect(_socket, &QTcpSocket::bytesWritten, this, &MsgStream::bytesWritten);

    QHostAddress hostAddress(peerAddress);
    _socket->connectToHost(hostAddress, port);
    LoggerManager::getInstance().writeInfo(QString("MsgStream.init ended -|- connected to %1 on %2").arg(hostAddress.toString(), QString::number(port)));
}

void MsgStream::init(QTcpSocket* socket) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.init started"));

    _socket = socket;
    connect(_socket, &QTcpSocket::disconnected, this, &MsgStream::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &MsgStream::readyRead);
    connect(_socket, &QTcpSocket::bytesWritten, this, &MsgStream::bytesWritten);

    LoggerManager::getInstance().writeInfo(QString("MsgStream.init ended -|- connected to %1 on %2").arg(socket->peerAddress().toString(), QString::number(socket->peerPort())));
}

void MsgStream::stop() {
    if(_socket && _socket->isOpen())
        _socket->close();
}

void MsgStream::sendMessage(QByteArray& data) {
    outData.swap(data);
    outData.prepend(QString("%1").arg(outData.length(), 10).toLocal8Bit());
    outDataLen += outData.length();

    LoggerManager::getInstance().writeInfo(QString("MsgStream.sendMessage started -|- Writing %1 bytes: %2").arg(QString::number(outData.length()), outData.data()));

    qint64 numBytesWritten = _socket->write(outData);
    if(numBytesWritten < 0)
        LoggerManager::getInstance().writeError(QStringLiteral("MsgStream.sendMessage -|- Socket write failed"));

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.sendMessage ended"));
}

void MsgStream::connected() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.connected started"));

    outData = QString("MSG%1").arg(localId).toLocal8Bit(); // insert indicator that this socket handles messages
    outDataLen = outData.length();

    //	send an id message and then wait for public key message
    //	from receiver, which will trigger readyRead signal
    qint64 numBytesWritten = _socket->write(outData, outDataLen);
    if(numBytesWritten < 0)
         LoggerManager::getInstance().writeError(QStringLiteral("MsgStream.connected -|- Socket write failed"));

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.connected ended"));
}

void MsgStream::disconnected() {
    emit connectionLost(&peerId);
}

void MsgStream::readyRead() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.readyRead started"));

    qint64 available = _socket->bytesAvailable();
    while(available > 0) {
        if(!reading) {
            reading = true;
            if (available > 10) {
                QByteArray len = _socket->read(10);
                LoggerManager::getInstance().writeInfo(QString("MsgStream.readyRead-|- reading input: %1").arg(len.data()));

                inDataLen = len.toUInt();
            } else
                inDataLen = 0;

            inData.clear();
            QByteArray data = _socket->read(inDataLen);
            LoggerManager::getInstance().writeInfo(QString("MsgStream.readyRead-|- data: %1").arg(data.data()));

            inData.append(data);
            inDataLen -= data.length();
            available -= ((available > 10 ? 10 : available) +  data.length());
            if(inDataLen == 0) {
                reading = false;
                emit messageReceived(&peerId, &peerAddress, inData);
            }
        } else {
            QByteArray data = _socket->read(inDataLen);
            inData.append(data);
            inDataLen -= data.length();
            available -= data.length();
            if(inDataLen == 0) {
                reading = false;
                emit messageReceived(&peerId, &peerAddress, inData);
            }
        }
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.readyRead ended"));
}

void MsgStream::bytesWritten(qint64 bytes) {
    outDataLen -= bytes;
    if(outDataLen == 0){
        LoggerManager::getInstance().writeWarning(QStringLiteral("MsgStream.bytesWritten -|- Socket write operation completed"));
        return;
    }

    if(outDataLen > 0)
         LoggerManager::getInstance().writeWarning(QString("MsgStream.bytesWritten -|- Socket write operation not completed. Written %1 bytes out of %2 - error: %3").arg(QString::number(bytes), QString::number(outDataLen + bytes), QString::number(_socket->error())));
    if(outDataLen < 0)
        LoggerManager::getInstance().writeWarning(QString("MsgStream.bytesWritten -|- Socket write overrun. Written %1 bytes out of %2 - error: %3").arg(QString::number(bytes), QString::number(outDataLen + bytes), QString::number(_socket->error())));

    //	TODO: handle situation when entire message is not written to stream in one write operation
    //	The following code is not functional currently, hence commented out.
    /*outData = outData.mid(outDataLen);
    socket->write(outData);*/
}
