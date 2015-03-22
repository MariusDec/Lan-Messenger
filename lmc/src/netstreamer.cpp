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

FileSender::FileSender(const QString &id, const QString &localId, const QString &peerId, const QString &peerName, const QString &filePath,
    const QString &fileName, qint64 fileSize, const QString &address, int port, const FileType &type) : id(id), peerId(peerId), peerName(peerName), type(type), _localId(localId), _filePath(filePath), _fileName(fileName), _fileSize(fileSize), _address(address), _port(port) {
        _mile = fileSize / 36;
        _milestone = _mile;
        _socket.setParent(this);
        _timer.setParent(this);
}

FileSender::~FileSender() {
}

void FileSender::init() {
    connect(&_socket, &QTcpSocket::connected, this, &FileSender::connected);
    connect(&_socket, &QTcpSocket::disconnected, this, &FileSender::disconnected);
    connect(&_socket, &QTcpSocket::readyRead, this, &FileSender::readyRead);
    connect(&_socket, &QTcpSocket::bytesWritten, this, &FileSender::bytesWritten);

    QHostAddress hostAddress(_address);
    _socket.connectToHost(hostAddress, _port);
}

void FileSender::stop() {
    _active = false;

    _timer.stop();
    if(_file.isOpen())
        _file.close();
    if(_socket.isOpen())
        _socket.close();
}

void FileSender::connected() {
    QByteArray data = QString("FILE%0%1").arg(id, _localId).toLocal8Bit();	// insert indicator that this socket handles file transfer
    //	send an id message and then wait for a START message
    //	from receiver, which will trigger readyRead signal
    _socket.write(data);
}

void FileSender::disconnected() {
    if(_active) {
        QString data;
        emit progressUpdated(FM_Send, FO_Error, type, id, peerId, peerName, data);
    }
}

void FileSender::readyRead() {
    //	message received from receiver, start sending the file
    sendFile();
}

void FileSender::timer_timeout() {
    if(!_active)
        return;

    QString transferred = QString::number(_file.pos());
    emit progressUpdated(FM_Send, FO_Progress, type, id, peerId, peerName, transferred);
}

void FileSender::bytesWritten(qint64 bytes) {
    Q_UNUSED(bytes);

    if(!_active)
        return;

    qint64 unsentBytes = _fileSize - _file.pos();

    if(unsentBytes == 0) {
        _active = false;
        _file.close();
        _socket.close();
        emit progressUpdated(FM_Send, FO_Complete, type, id, peerId, peerName, _filePath);
        return;
    }

    qint64 bytesToSend = (bufferSize < unsentBytes) ? bufferSize : unsentBytes;
    qint64 bytesRead = _file.read(_buffer, bytesToSend);
    _socket.write(_buffer, bytesRead);
}

void FileSender::sendFile() {
    LoggerManager::getInstance().writeInfo(QString("MsgStream.sendFile started -|- Sending file to %1 (%2) on %3").arg(peerId, peerName, _address));

    _file.setFileName(_filePath);

    if(_file.open(QIODevice::ReadOnly)) {
        _buffer = new char[bufferSize];
        _active = true;

        connect(&_timer, &QTimer::timeout, this, &FileSender::timer_timeout);
        _timer.start(PROGRESS_TIMEOUT);

        qint64 unsentBytes = _fileSize - _file.pos();
        qint64 bytesToSend = (bufferSize < unsentBytes) ? bufferSize : unsentBytes;
        qint64 bytesRead = _file.read(_buffer, bytesToSend);
        _socket.write(_buffer, bytesRead);
    } else {
        _socket.close();
        QString data;
        emit progressUpdated(FM_Send, FO_Error, type, id, peerId, peerName, data);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.sendFile ended"));
}


/****************************************************************************
** Class: FileReceiver
** Description: Handles receiving files.
****************************************************************************/
FileReceiver::FileReceiver() {
    _timer.setParent(this);
}

FileReceiver::FileReceiver(const QString &id, const QString &peerId, const QString &peerName, const QString &filePath, const QString &fileName,
                           qint64 fileSize, const QString &address, int port, const FileType &type) : id(id), peerId(peerId), peerName(peerName), type(type), _filePath(filePath), _fileName(fileName), _fileSize(fileSize), _address(address), _port(port) {
    _mile = fileSize / 36;
    _milestone = _mile;

    _timer.setParent(this);
}

FileReceiver::~FileReceiver() {
}

void FileReceiver::init(QTcpSocket *socket) {
    _socket = socket;
    connect(_socket, &QTcpSocket::disconnected, this, &FileReceiver::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &FileReceiver::readyRead);

    receiveFile();
    //	now send a START message to sender
    _socket->write("START");
}

void FileReceiver::stop() {
    bool deleteFile = false;

    _active = false;

    _timer.stop();
    if(_file.isOpen()) {
        deleteFile = (_file.pos() < _fileSize);
        _file.close();
    }
    if(_socket->isOpen())
        _socket->close();

    if(deleteFile)
        QFile::remove(_filePath);
}

void FileReceiver::disconnected() {
    if(_active) {
        QString data;
        emit progressUpdated(FM_Receive, FO_Error, type, id, peerId, peerName, data);
    }
}

void FileReceiver::readyRead() {
    if(!_active)
        return;

    qint64 bytesReceived = _socket->read(_buffer, bufferSize);
    _file.write(_buffer, bytesReceived);

    qint64 unreceivedBytes = _fileSize - _file.pos();
    if(unreceivedBytes == 0) {
        _active = false;
        _file.close();
        _socket->close();
        emit progressUpdated(FM_Receive, FO_Complete, type, id, peerId, peerName, _filePath);
        return;
    }

//	if(file->pos() > milestone) {
//		QString transferred = QString::number(file->pos());
//		emit progressUpdated(FM_Receive, FO_Progress, type, &id, &peerId, &transferred);
//		milestone += mile;
//	}
}

void FileReceiver::timer_timeout() {
    if(!_active)
        return;

    if(_lastPosition < _file.pos()) {
        _lastPosition = _file.pos();
        _numTimeOuts = 0;
    } else {
        _numTimeOuts++;
        if(_numTimeOuts > 20) {
            QString data;
            emit progressUpdated(FM_Receive, FO_Error, type, id, peerId, peerName, data);
            stop();
            return;
        }
    }

    QString transferred = QString::number(_file.pos());
    emit progressUpdated(FM_Receive, FO_Progress, type, id, peerId, peerName, transferred);
}

void FileReceiver::receiveFile() {
    LoggerManager::getInstance().writeInfo(QString("MsgStream.receiveFile started -|- Receiving file from %1 (%2) on %3").arg(peerId, peerName, _address));

    QDir dir = QFileInfo(_filePath).dir();
    if(!dir.exists())
        dir.mkpath(dir.absolutePath());

    _file.setFileName(_filePath);

    if(_file.open(QIODevice::WriteOnly)) {
        _buffer = new char[bufferSize];
        _active = true;

        connect(&_timer, &QTimer::timeout, this, &FileReceiver::timer_timeout);
        _timer.start(PROGRESS_TIMEOUT);
    } else {
        _socket->close();
        emit progressUpdated(FM_Receive, FO_Error, type, id, peerId, peerName, _filePath);
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.receiveFile ended"));
}

/****************************************************************************
** Class: MsgStream
** Description: Handles transmission and reception of TCP streaming messages.
****************************************************************************/

void MsgStream::init() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.init started"));

    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &MsgStream::connected);
    connect(_socket, &QTcpSocket::disconnected, this, &MsgStream::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &MsgStream::readyRead);
    connect(_socket, &QTcpSocket::bytesWritten, this, &MsgStream::bytesWritten);

    QHostAddress hostAddress(_peerAddress);
    _socket->connectToHost(hostAddress, _port);
    LoggerManager::getInstance().writeInfo(QString("MsgStream.init ended -|- connected to %1 on %2").arg(hostAddress.toString(), QString::number(_port)));
}

void MsgStream::init(QTcpSocket *socket) {
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

void MsgStream::sendMessage(QByteArray &data) {
    _outData.swap(data);
    _outData.prepend(QString("%1").arg(_outData.length(), 10).toLocal8Bit());
    _outDataLen += _outData.length();

    LoggerManager::getInstance().writeInfo(QString("MsgStream.sendMessage started -|- Writing %1 bytes: %2").arg(QString::number(_outData.length()), _outData.data()));

    qint64 numBytesWritten = _socket->write(_outData);
    _socket->flush();
    if(numBytesWritten < 0)
        LoggerManager::getInstance().writeError(QStringLiteral("MsgStream.sendMessage -|- Socket write failed"));

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.sendMessage ended"));
}

void MsgStream::connected() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.connected started"));

    _outData = QString("MSG%1").arg(_localId).toLocal8Bit(); // insert indicator that this socket handles messages
    _outDataLen = _outData.length();

    //	send an id message and then wait for public key message
    //	from receiver, which will trigger readyRead signal
    qint64 numBytesWritten = _socket->write(_outData, _outDataLen);
    if(numBytesWritten < 0)
         LoggerManager::getInstance().writeError(QStringLiteral("MsgStream.connected -|- Socket write failed"));

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.connected ended"));
}

void MsgStream::disconnected() {
    LoggerManager::getInstance().writeInfo(QString("MsgStream.disconnected -|- Connection to user %1 lost").arg(_peerId));
    emit connectionLost(_peerId);
}

void MsgStream::readyRead() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.readyRead started"));

    qint64 available = _socket->bytesAvailable();
    while(available > 0) {
        if(!_reading) {
            _reading = true;
            if (available > 10) {
                QByteArray len = _socket->read(10);
                LoggerManager::getInstance().writeInfo(QString("MsgStream.readyRead-|- reading input: %1").arg(len.data()));

                _inDataLen = len.toUInt();
            } else
                _inDataLen = 0;

            _inData.clear();
            QByteArray data = _socket->read(_inDataLen);
            LoggerManager::getInstance().writeInfo(QString("MsgStream.readyRead-|- data: %1").arg(data.data()));

            _inData.append(data);
            _inDataLen -= data.length();
            available -= ((available > 10 ? 10 : available) +  data.length());
            if(_inDataLen == 0) {
                _reading = false;
                emit messageReceived(_peerId, _peerAddress, _inData);
            }
        } else {
            QByteArray data = _socket->read(_inDataLen);
            _inData.append(data);
            _inDataLen -= data.length();
            available -= data.length();
            if(_inDataLen == 0) {
                _reading = false;
                emit messageReceived(_peerId, _peerAddress, _inData);
            }
        }
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.readyRead ended"));
}

void MsgStream::bytesWritten(qint64 bytes) {
    _outDataLen -= bytes;
    if(_outDataLen == 0){
        LoggerManager::getInstance().writeInfo(QStringLiteral("MsgStream.bytesWritten -|- Socket write operation completed"));
        return;
    }

    if(_outDataLen > 0)
         LoggerManager::getInstance().writeWarning(QString("MsgStream.bytesWritten -|- Socket write operation not completed. Written %1 bytes out of %2 - error: %3").arg(QString::number(bytes), QString::number(_outDataLen + bytes), QString::number(_socket->error())));
    if(_outDataLen < 0)
        LoggerManager::getInstance().writeWarning(QString("MsgStream.bytesWritten -|- Socket write overrun. Written %1 bytes out of %2 - error: %3").arg(QString::number(bytes), QString::number(_outDataLen + bytes), QString::number(_socket->error())));
}
