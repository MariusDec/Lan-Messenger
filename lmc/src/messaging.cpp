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


#include "messaging.h"
#include "stdlocation.h"
#include "loggermanager.h"
#include "globals.h"
#include "imageslist.h"

lmcMessaging::lmcMessaging() {
    pNetwork = new lmcNetwork();
    connect(pNetwork, &lmcNetwork::broadcastReceived,
        this, &lmcMessaging::receiveBroadcast);
    connect(pNetwork, &lmcNetwork::messageReceived,
        this, &lmcMessaging::receiveMessage);
    connect(pNetwork, &lmcNetwork::webMessageReceived,
        this, &lmcMessaging::receiveWebMessage);
    connect(pNetwork, &lmcNetwork::newConnection,
        this, &lmcMessaging::newConnection);
    connect(pNetwork, &lmcNetwork::connectionLost,
        this, &lmcMessaging::connectionLost);
    connect(pNetwork, &lmcNetwork::progressReceived,
        this, &lmcMessaging::receiveProgress);
    connect(pNetwork, &lmcNetwork::connectionStateChanged, this, &lmcMessaging::network_connectionStateChanged);
    localUser = NULL;
    userList.clear();
    groupList.clear();
    userGroupMap.clear();
    receivedList.clear();
    pendingList.clear();
    fileList.clear();
    folderList.clear();
    loopback = false;
}

lmcMessaging::~lmcMessaging() {
}

void lmcMessaging::init(XmlMessage *pInitParams) {
    LoggerManager::getInstance ().writeInfo (QStringLiteral("lmcMessaging.init started"));

    pNetwork->init(pInitParams);

    QString userId = pNetwork->IPAddress();

    pNetwork->setLocalId(&userId);

    pSettings = new lmcSettings();
    QString userStatus = pSettings->value(IDS_STATUS, IDS_STATUS_VAL).toString();
    //	if status not recognized, default to available
    if(!Globals::getInstance ().statusExists (userStatus))
        userStatus = "Available";
    QString userName = getUserName();

    int nAvatar = pSettings->value(IDS_AVATAR, IDS_AVATAR_VAL).toInt();
    QString userNote = pSettings->value(IDS_NOTE, IDS_NOTE_VAL).toString();
    uint userCaps = UC_File | UC_GroupMessage | UC_Folder;
    localUser = new User(userId, IDA_VERSION, pNetwork->ipAddress, userName, userStatus,
                         QString::null, nAvatar, userNote, ImagesList::getInstance ().getAvatar (nAvatar),
                         QString::number(userCaps), Helper::getHostName());

    loadGroups();

    nTimeout = pSettings->value(IDS_TIMEOUT, IDS_TIMEOUT_VAL).toInt() * 1000;
    nMaxRetry = pSettings->value(IDS_MAXRETRIES, IDS_MAXRETRIES_VAL).toInt();

    pTimer = new QTimer(this);
    connect(pTimer, &QTimer::timeout, this, &lmcMessaging::timer_timeout);
    pTimer->start(1000);

    msgId = 1;

    LoggerManager::getInstance ().writeInfo (QStringLiteral("lmcMessaging.init ended"));
}

void lmcMessaging::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.start started"));
    pNetwork->start();

    sendBroadcast(MT_Depart, NULL);
    sendBroadcast(MT_Announce, NULL);
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.start ended"));
}

void lmcMessaging::update() {
    sendBroadcast(MT_Announce, NULL);
}

void lmcMessaging::stop() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.stop started"));
    sendBroadcast(MT_Depart, NULL);
    pNetwork->stop();

    pSettings->setValue(IDS_STATUS, localUser->status);
    pSettings->setValue(IDS_AVATAR, localUser->avatar);

    saveGroups();

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.stop ended"));
}

bool lmcMessaging::isConnected() {
    return pNetwork->isConnected;
}

bool lmcMessaging::canReceive() {
    return pNetwork->canReceive;
}

void lmcMessaging::setLoopback(bool on) {
    loopback = on;
}

User* lmcMessaging::getUser(const QString* userId) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.getUser started"));

    if (!userId) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.getUser -|- User id is null"));
        return nullptr;
    }

    for(int index = 0; index < userList.count(); index++)
        if(userList[index].id.compare(*userId) == 0)
            return &userList[index];

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.getUser ended -|- The user with the id %1 doesn't exist").arg(*userId));
    return nullptr;
}

void lmcMessaging::settingsChanged() {
    nTimeout = pSettings->value(IDS_TIMEOUT, IDS_TIMEOUT_VAL).toInt() * 1000;
    nMaxRetry = pSettings->value(IDS_MAXRETRIES, IDS_MAXRETRIES_VAL).toInt();
    pNetwork->settingsChanged();

    QString userName = getUserName();
    if(localUser->name.compare(userName) != 0) {
        localUser->name = userName;
        XmlMessage xmlMessage;
        xmlMessage.addData(XN_NAME, userName);
        sendMessage(MT_UserName, NULL, &xmlMessage);
    }
}

void lmcMessaging::updateGroup(GroupOp op, QVariant value1, QVariant value2) {
    switch(op) {
    case GO_New:
        groupList.append(Group(value1.toString(), value2.toString()));
        break;
    case GO_Rename:
        for(int index = 0; index < groupList.count(); index++) {
            if(groupList[index].id.compare(value1.toString()) == 0) {
                groupList[index].name = value2.toString();
                break;
            }
        }
        break;
    case GO_Move:
        for(int index = 0; index < groupList.count(); index++) {
            if(groupList[index].id.compare(value1.toString()) == 0) {
                groupList.move(index, value2.toInt());
                break;
            }
        }
        break;
    case GO_Delete:
        for(int index = 0; index < groupList.count(); index++) {
            if(groupList[index].id.compare(value1.toString()) == 0) {
                groupList.removeAt(index);
                break;
            }
        }
        break;
    default:
        break;
    }

    saveGroups();
}

void lmcMessaging::updateGroupMap(QString oldGroup, QString newGroup) {
    QMap<QString, QString>::const_iterator index = userGroupMap.constBegin();
    while (index != userGroupMap.constEnd()) {
        if(((QString)index.value()).compare(oldGroup) == 0)
            userGroupMap.insert(index.key(), newGroup);
        ++index;
    }
}

//	save groups and group mapping
void lmcMessaging::saveGroups() {
    // TODO save groups after every modification
    QSettings groupSettings(StdLocation::writableGroupsFile(), QSettings::IniFormat);
    groupSettings.beginWriteArray(IDS_GROUPHDR);
    for(int index = 0; index < groupList.count(); index++) {
        groupSettings.setArrayIndex(index);
        groupSettings.setValue(IDS_GROUP, groupList[index].id);
        groupSettings.setValue(IDS_GROUPNAME, groupList[index].name);
    }
    groupSettings.endArray();

    groupSettings.beginWriteArray(IDS_GROUPMAPHDR);
    QMapIterator<QString, QString> i(userGroupMap);
    int count = 0;
    while(i.hasNext()) {
        groupSettings.setArrayIndex(count);
        i.next();
        groupSettings.setValue(IDS_USER, i.key());
        groupSettings.setValue(IDS_GROUP, i.value());
        count++;
    }
    groupSettings.endArray();

    groupSettings.sync();

    // make sure the correct version is set in the preferences file
    // so the group settings will not be wrongly migrated next time
    // application starts
    pSettings->setValue(IDS_VERSION, IDA_VERSION);
}

int lmcMessaging::userCount() {
    return userList.count();
}

void lmcMessaging::network_connectionStateChanged() {
    if(isConnected()) {
        localUser->address = pNetwork->ipAddress;
        if(localUser->id.isNull()) {
            QString userId = pNetwork->IPAddress();
            localUser->id = userId;
            pNetwork->setLocalId(&userId);
        }
    }
    emit connectionStateChanged();
}

void lmcMessaging::timer_timeout() {
    //	check if any pending message has timed out
    checkPendingMsg();
}

QString lmcMessaging::createUserId(const QString &lpszAddress, const QString &lpszUserName) {
    QString userId = lpszAddress;
    if(!userId.isNull()) {
        userId.append(lpszUserName);
        userId.remove(":");
    }
    return userId;
}

QString lmcMessaging::getUserName() {
    QString userName = pSettings->value(IDS_USERNAME, IDS_USERNAME_VAL).toString();
    if(userName.isEmpty())
        userName = Helper::getLogonName();
    return userName;
}

void lmcMessaging::loadGroups() {
    bool defaultFound = false;

    QSettings groupSettings(StdLocation::groupsFile(), QSettings::IniFormat);
    int size = groupSettings.beginReadArray(IDS_GROUPHDR);
    for(int index = 0; index < size; ++index) {
        groupSettings.setArrayIndex(index);
        QString groupId = groupSettings.value(IDS_GROUP).toString();
        QString group = groupSettings.value(IDS_GROUPNAME).toString();
        groupList.append(Group(groupId, group));
        // check if the default group is present in the group list
        if(groupId.compare(GRP_DEFAULT_ID) == 0)
            defaultFound = true;
    }
    groupSettings.endArray();

    if(groupList.count() == 0 || !defaultFound)
        groupList.append(Group(GRP_DEFAULT_ID, GRP_DEFAULT));

    size = groupSettings.beginReadArray(IDS_GROUPMAPHDR);
    for(int index = 0; index < size; index++)
    {
        groupSettings.setArrayIndex(index);
        QString user = groupSettings.value(IDS_USER).toString();
        QString group = groupSettings.value(IDS_GROUP).toString();
        userGroupMap.insert(user, group);
    }
    groupSettings.endArray();
}

void lmcMessaging::getUserInfo(XmlMessage* pMessage) {
    QString firstName = pSettings->value(IDS_USERFIRSTNAME, IDS_USERFIRSTNAME_VAL).toString();
    QString lastName = pSettings->value(IDS_USERLASTNAME, IDS_USERLASTNAME_VAL).toString();
    QString about = pSettings->value(IDS_USERABOUT, IDS_USERABOUT_VAL).toString();
    firstName = firstName.isEmpty() ? "N/A" : firstName;
    lastName = lastName.isEmpty() ? "N/A" : lastName;
    about = about.isEmpty() ? "N/A" : about;

    pMessage->addData(XN_USERID, localUser->id);
    pMessage->addData(XN_NAME, localUser->name);
    pMessage->addData(XN_ADDRESS, localUser->address);
    pMessage->addData(XN_VERSION, localUser->version);
    pMessage->addData(XN_STATUS, localUser->status);
    pMessage->addData(XN_NOTE, localUser->note);
    pMessage->addData(XN_LOGON, Helper::getLogonName());
    pMessage->addData(XN_HOST, Helper::getHostName()); // NOTE  Computer name
    pMessage->addData(XN_OS, Helper::getOSName());
    pMessage->addData(XN_FIRSTNAME, firstName);
    pMessage->addData(XN_LASTNAME, lastName);
    pMessage->addData(XN_ABOUT, about);
}

bool lmcMessaging::addUser(QString szUserId, const QString &szVersion, const QString &userIP, const QString &szName, const QString &szStatus, const QString &szAvatar, const QString &szNote, const QString &szCaps, const QString &hostName) { // TODO !!! Add pc name and stuff here
    for(int index = 0; index < userList.count(); index++)
        if(userList[index].id.compare(szUserId) == 0) {
            LoggerManager::getInstance().writeInfo(QString("lmcMessaging.addUser failed -|- Adding new user: %1, %2, %3").arg(szUserId, szVersion, userIP));
            return false;
        }

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.addUser started -|- Adding new user: %1, %2, %3").arg(szUserId, szVersion, userIP));

    if(!userGroupMap.contains(szUserId) || !groupList.contains(Group(userGroupMap.value(szUserId))))
        userGroupMap.insert(szUserId, GRP_DEFAULT_ID);

    uint nAvatar = szAvatar.isNull() ? -1 : szAvatar.toInt();

    userList.append(User(szUserId, szVersion, userIP, szName, szStatus, userGroupMap[szUserId],
                         nAvatar, szNote, ImagesList::getInstance ().getAvatar (nAvatar), szCaps, hostName));
    if(!szStatus.isNull()) {
        XmlMessage xmlMessage;
        xmlMessage.addHeader(XN_FROM, szUserId);
        xmlMessage.addData(XN_STATUS, szStatus);
        //	send a status message to app layer, this is different from announce message
        emit messageReceived(MT_Status, &szUserId, &xmlMessage);
        if(Globals::getInstance ().getStatusType (szStatus) == StatusTypeEnum::StatusOffline) { // offline status
            LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.addUser ended-|- szStatus is offline"));
            return false;	//	no need to send a new user message to app layer
        }
    }

    // TODO avatar
    emit messageReceived(MT_Announce, &szUserId, NULL);
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.addUser ended"));
    return true;
}

void lmcMessaging::updateUser(MessageType type, QString &szUserId, const QString &szUserData) {
    User* pUser = getUser(&szUserId);
    if(!pUser)
        return;

    XmlMessage updateMsg;
    switch(type) {
    case MT_Status:
        if(pUser->status.compare(szUserData) != 0) {
            QString oldStatus = pUser->status;
            pUser->status = szUserData;

            if(Globals::getInstance ().getStatusType (oldStatus) == StatusTypeEnum::StatusOffline) // old status is offline
                emit messageReceived(MT_Announce, &szUserId, NULL);

            updateMsg.addData(XN_STATUS, pUser->status);
            emit messageReceived(MT_Status, &szUserId, &updateMsg);

            if(Globals::getInstance ().getStatusType (pUser->status) == StatusTypeEnum::StatusOffline) { // new status is offline
                // Send a dummy xml message. A non null xml message implies that the
                // user is only in offline status, and not actually offline.
                XmlMessage xmlMessage;
                emit messageReceived(MT_Depart, &szUserId, &xmlMessage);
            }
        }
        break;
    case MT_UserName:
        if(pUser->name.compare(szUserData) != 0) {
            pUser->name = szUserData;
            updateMsg.addData(XN_NAME, pUser->name);
            emit messageReceived(MT_UserName, &szUserId, &updateMsg);
        }
        break;
    case MT_Note:
        if(pUser->note.compare(szUserData) != 0) {
            pUser->note = szUserData;
            updateMsg.addData(XN_NOTE, pUser->note);
            emit messageReceived(MT_Note, &szUserId, &updateMsg);
        }
        break;
    case MT_Group:
        pUser->group = szUserData;
        userGroupMap.insert(pUser->id, pUser->group);
        saveGroups();
        break;
    case MT_Avatar:
        pUser->avatarPath = szUserData;
        break;
    default:
        break;
    }
}

void lmcMessaging::removeUser(QString szUserId) {
    for(int index = 0; index < userList.count(); index++)
        if(userList.value(index).id.compare(szUserId) == 0) {
            XmlMessage statusMsg;
            statusMsg.addData(XN_STATUS, Globals::getInstance ().getStatuses ().back ().description);
            emit messageReceived(MT_Status, &szUserId, &statusMsg);
            emit messageReceived(MT_Depart, &szUserId, NULL);
            userList.removeAt(index);
            return;
        }
}

bool lmcMessaging::addReceivedMsg(qint64 msgId, const QString &userId) {
    ReceivedMsg received(msgId, userId);

    if(receivedList.contains(received))
        return false;

    receivedList.append(received);
    return true;
}

void lmcMessaging::addPendingMsg(qint64 msgId, MessageType type, QString* lpszUserId, XmlMessage* pMessage) {
    XmlMessage xmlMessage;
    if(pMessage)
        xmlMessage = pMessage->clone();
    pendingList.append(PendingMsg(msgId, true, QDateTime::currentDateTime(), type, *lpszUserId, xmlMessage, 1));
}

void lmcMessaging::removePendingMsg(qint64 msgId) {
    for(int index = 0; index < pendingList.count(); index++) {
        if(pendingList[index].msgId == msgId) {
            pendingList[index].active = false;
            pendingList.removeAt(index);
            return;
        }
    }
}

void lmcMessaging::removeAllPendingMsg(QString* lpszUserId) {
    for(int index = 0; index < pendingList.count(); index++) {
        if(pendingList[index].userId.compare(*lpszUserId) == 0) {
            pendingList.removeAt(index);
            index--;
        }
    }
}

void lmcMessaging::checkPendingMsg() {
    for(int index = 0; index < pendingList.count(); index++) {
        //	check if message has timed out
        if(pendingList[index].active && pendingList[index].timeStamp.msecsTo(QDateTime::currentDateTime()) > pendingList[index].retry * nTimeout) {
            if(pendingList[index].retry < nMaxRetry) {
                //	send the message once more
                pendingList[index].retry++;
                pendingList[index].timeStamp = QDateTime::currentDateTime();
                resendMessage(pendingList[index]);
            }
            else {
                XmlMessage statusMsg;
                //	max retries exceeded. mark message as failed.
                switch(pendingList[index].type) {
                case MT_Message:
                    emit messageReceived(MT_Failed, &pendingList[index].userId, &pendingList[index].xmlMessage);
                    break;
                default:
                    break;
                }
                pendingList[index].active = false;
                pendingList.removeAt(index);
                index--;	//	since next item will have this index now
            }
        }
    }
}

void lmcMessaging::resendMessage(MessageType type, qint64 msgId, QString* lpszUserId, XmlMessage* pMessage) {
    if(lpszUserId && !getUser(lpszUserId))
        return;

    prepareMessage(type, msgId, true, lpszUserId, pMessage);
}

void lmcMessaging::resendMessage(PendingMsg msg) {
    // type, pendingList[index].msgId, &pendingList[index].userId, &pendingList[index].xmlMessage

    if(msg.userId != QString::null && !getUser(&msg.userId))
        return;

    prepareMessage(msg.type, msg.msgId, true, &msg.userId, &msg.xmlMessage);
}
