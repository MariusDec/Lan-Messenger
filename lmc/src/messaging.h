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


#ifndef MESSAGING_H
#define MESSAGING_H

#include "shared.h"
#include "message.h"
#include "network.h"
#include "settings.h"
#include "xmlmessage.h"

#include <QObject>
#include <QTimer>


struct PendingMsg {
    qint64 msgId;
    bool active;
    QDateTime timeStamp;
    MessageType type;
    QString userId;
    XmlMessage xmlMessage;
    int retry;

    PendingMsg() {}
    PendingMsg(qint64 msgId, bool active, const QDateTime &timeStamp, MessageType type, const QString &userId, const XmlMessage &xmlMessage, int retry) : msgId(msgId), active(active), timeStamp(timeStamp), type(type), userId(userId), xmlMessage(xmlMessage), retry(retry) { }
};

struct ReceivedMsg {
    qint64 msgId;
    QString userId;

    ReceivedMsg() {}
    ReceivedMsg(qint64 nMsgId, QString szUserId) {
        this->msgId = nMsgId;
        this->userId = szUserId;
    }

    bool operator == (const ReceivedMsg& v) const { return ((this->msgId == v.msgId) && (this->userId.compare(v.userId) == 0)); }
};

struct Transfer {
    QString id;
    QString userId;
    QString path;
    QString name;
    qint64 size;
    qint64 pos;
    FileMode mode;
    FileOp op;
    FileType type;
};

struct TransFile : public Transfer {
    QString folderId;
    QString relPath;

    TransFile() {}
    TransFile(QString szId, QString szFolderId, QString szUserId, QString szPath, QString szName,
              qint64 nSize, FileMode mode, FileOp op, FileType type) {
        this->id = szId;
        this->folderId = szFolderId;
        this->userId = szUserId;
        this->path = szPath;
        this->name = szName;
        this->size = nSize;
        this->pos = 0;
        this->mode = mode;
        this->op = op;
        this->type = type;
        this->relPath = QString::null;
    }
};

struct TransFolder : public Transfer {
    QStringList fileList;
    QString currentFile;
    int fileIndex;
    int fileCount;
    qint64 filePos;
    QDateTime lastUpdated;

    TransFolder() {}
    TransFolder(QString szId, QString szUserId, QString szPath, QString szName,
                qint64 nSize, FileMode mode, FileOp op, FileType type, int nFileCount) {
        this->id = szId;
        this->userId = szUserId;
        this->path = szPath;
        this->name = szName;
        this->size = nSize;
        this->pos = 0;
        this->mode = mode;
        this->op = op;
        this->type = type;
        this->fileList.clear();
        this->currentFile = QString::null;
        this->fileIndex = 0;
        this->fileCount = nFileCount;
        this->filePos = 0;
      }
};

class lmcMessaging : public QObject {
    Q_OBJECT

public:
    lmcMessaging();
    ~lmcMessaging();

    void init(const XmlMessage &initParams);
    void start();
    void update();
    void stop();
    bool isConnected();
    bool canReceive();
    void setLoopback(bool on);
    User *getUser(const QString &lpszUserId);
    void sendBroadcast(MessageType type);
    void sendMessage(MessageType type, const QString &userId, XmlMessage &message);
    void sendWebMessage(MessageType type);
    void settingsChanged();
    void updateGroup(GroupOp op, QVariant value1, QVariant value2);
    void updateGroupMap(QString oldGroup, QString newGroup);
    void saveGroups();
    int userCount();

    User* localUser;
    QList<User> userList;
    QList<Group> groupList;

signals:
    void messageReceived(MessageType type, QString userId, XmlMessage message);
    void connectionStateChanged();

private slots:
    void receiveBroadcast(const DatagramHeader &header, const QString &data);
    void receiveMessage(const DatagramHeader &header, const QString &lpszData);
    void receiveWebMessage(const QString &data);
    void newConnection(QString userId, QString address);
    void connectionLost(QString userId);
    void receiveProgress(QString userId, QString userName, QString data);
    void network_connectionStateChanged();

private:
    QString createUserId(const QString &lpszAddress, const QString &lpszUserName);
    QString getUserName();
    void loadGroups();
    void getUserInfo(XmlMessage &message);
    void sendUserData(MessageType type, QueryOp op, const QString &userId);
    void prepareBroadcast(MessageType type);
    void prepareMessage(MessageType type, qint64 msgId, const QString &userId, XmlMessage &message);
    void prepareFile(const QString &userId, XmlMessage &message);
    void prepareFolder(const QString &userId, XmlMessage &message);
    void processBroadcast(const MessageHeader &header);
    void processMessage(const MessageHeader &header, XmlMessage &message);
    void processFile(const MessageHeader &header, XmlMessage &message);
    void processFolder(const MessageHeader &header, XmlMessage &message);
    void processWebMessage(const MessageHeader &header, const XmlMessage &message);
    bool addUser(QString szUserId, const QString &szVersion, const QString &userIP, const QString &szName, const QString &szStatus, const QString &szAvatar, const QString &szNote, const QString &szCaps, const QString &hostName);
    void updateUser(MessageType type, const QString &userId, const QString &userData);
    void removeUser(const QString &szUserId);
    bool addReceivedMsg(qint64 msgId, const QString &userId);
    bool addFileTransfer(FileMode fileMode, const QString &userId, XmlMessage &message);
    bool updateFileTransfer(FileMode fileMode, FileOp fileOp, const QString &userId, const QString &userName, XmlMessage &pMessage);
    QString getFreeFileName(const QString &fileName);
    bool addFolderTransfer(FileMode folderMode, const QString &userId, XmlMessage &message);
    bool updateFolderTransfer(FileMode folderMode, FileOp folderOp, const QString &userId, XmlMessage &message);
    QString getFreeFolderName(const QString &folderName);
    QString getFolderPath(const QString &folderId, const QString &userId, FileMode mode);

    lmcNetwork*			pNetwork;
    qint64				msgId;
    QList<ReceivedMsg>	receivedList;
    QList<TransFile>    fileList;
    QList<TransFolder>  folderList;
    bool				loopback;
    QMap<QString, QString> userGroupMap;
};

#endif // MESSAGING_H
