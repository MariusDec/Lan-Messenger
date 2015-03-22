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


#ifndef NETSTREAMER_H
#define NETSTREAMER_H

#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QFile>
#include "shared.h"

/****************************************************************************
** Class: FileSender
** Description: Handles sending files.
****************************************************************************/
class FileSender : public QObject {
    Q_OBJECT

public:
    FileSender();
    FileSender(const QString &id, const QString &_localId, const QString &peerId, const QString &peerName, const QString &_filePath, const QString &_fileName, qint64 _fileSize,
        const QString &_address, int _port, const FileType &type);
    ~FileSender();

    void init();
    void stop();

    QString id;
    QString peerId;
    QString peerName;
    FileType type;

signals:
    void progressUpdated(FileMode mode, FileOp fileOp, FileType type, QString id, QString userId, QString userName, QString data);

private slots:
    void connected();
    void disconnected();
    void readyRead();
    void bytesWritten(qint64 bytes);
    void timer_timeout();

private:
    void sendFile();

    QString _localId;
    QString _filePath;
    QString _fileName;
    qint64 _fileSize;
    QString _address;
    int _port;
    qint64 _sentBytes;
    QTcpSocket _socket;
    QFile _file;
    char* _buffer = nullptr;
    bool _active = false;
    qint64 _milestone;
    qint64 _mile;
    QTimer _timer;
};

/****************************************************************************
** Class: FileReceiver
** Description: Handles receiving files.
****************************************************************************/
class FileReceiver : public QObject {
    Q_OBJECT

public:
    FileReceiver();
    FileReceiver(const QString &id, const QString &peerId, const QString &peerName, const QString &_filePath, const QString &_fileName, qint64 _fileSize,
        const QString &_address, int _port, const FileType &type);
    ~FileReceiver();

    void init(QTcpSocket *_socket);
    void stop();

    QString id;
    QString peerId;
    QString peerName;
    FileType type;

signals:
    void progressUpdated(FileMode mode, FileOp fileOp, FileType type, QString id, QString userId, QString userName, QString data);

private slots:
    void disconnected();
    void readyRead();
    void timer_timeout();

private:
    void receiveFile();

    QString _filePath;
    QString _fileName;
    qint64 _fileSize;
    QString _address;
    int _port;
    qint64 _sentBytes;
    QTcpSocket *_socket = nullptr;
    QFile _file;
    char* _buffer = nullptr;
    bool _active = false;
    qint64 _milestone;
    qint64 _mile;
    QTimer _timer;
    int _numTimeOuts = 0;
    qint64 _lastPosition = 0;
};

/****************************************************************************
** Class: MsgStream
** Description: Handles transmission and reception of TCP streaming messages.
****************************************************************************/
class MsgStream : public QObject {
    Q_OBJECT

public:
    MsgStream() { }
    MsgStream(const QString &localId, const QString &peerId, const QString &peerAddress, int port)  : _port(port), _localId(localId), _peerId(peerId), _peerAddress(peerAddress) { }
    ~MsgStream() { }

    void init();
    void init(QTcpSocket *_socket);
    void stop();
    void sendMessage(QByteArray &data);

signals:
    void connectionLost(QString userId);
    void messageReceived(QString userId, QString address, QByteArray data);

private slots:
    void connected();
    void disconnected();
    void readyRead();
    void bytesWritten(qint64 bytes);

private:
    QTcpSocket* _socket = nullptr;
    int _port;
    QString _localId;
    QString _peerId;
    QString _peerAddress;
    QByteArray _outData;
    QByteArray _inData;
    qint32 _outDataLen = 0;
    qint32 _inDataLen = 0;
    bool _reading = false;
};

#endif // NETSTREAMER_H
