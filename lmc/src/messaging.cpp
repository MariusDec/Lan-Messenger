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
    fileList.clear();
    folderList.clear();
    loopback = false;
}

lmcMessaging::~lmcMessaging() {
}

void lmcMessaging::init(const XmlMessage &initParams) {
    LoggerManager::getInstance ().writeInfo (QStringLiteral("lmcMessaging.init started"));

    pNetwork->init(initParams);

    QString userId = pNetwork->IPAddress();

    pNetwork->setLocalId(userId);

    QString userStatus = Globals::getInstance().userStatus();
    //	if status not recognized, default to available
    if(!Globals::getInstance ().statusExists (userStatus))
        userStatus = "Available";
    QString userName = getUserName();

    int nAvatar = Globals::getInstance().userAvatarIndex();
    QString userNote = Globals::getInstance().userNote();
    uint userCaps = UC_File | UC_GroupMessage | UC_Folder;
    localUser = new User(userId, IDA_VERSION, pNetwork->ipAddress, userName, userStatus,
                         QString::null, nAvatar, userNote, ImagesList::getInstance ().getAvatar (nAvatar),
                         QString::number(userCaps), Helper::getHostName());

    loadGroups();

    msgId = 1;

    LoggerManager::getInstance ().writeInfo (QStringLiteral("lmcMessaging.init ended"));
}

void lmcMessaging::start() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.start started"));
    pNetwork->start();

    sendBroadcast(MT_Announce);
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.start ended"));
}

void lmcMessaging::update() {
    sendBroadcast(MT_Announce);
}

void lmcMessaging::stop() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.stop started"));
    sendBroadcast(MT_Depart);
    pNetwork->stop();

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

User *lmcMessaging::getUser(const QString &userId) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.getUser started"));

    if (userId.isEmpty()) {
        LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessaging.getUser -|- User ID is empty"));
        return nullptr;
    }

    for(int index = 0; index < userList.count(); index++)
        if(userList[index].id.compare(userId) == 0)
            return &userList[index];

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.getUser ended -|- The user with the id %1 doesn't exist").arg(userId));
    return nullptr;
}

void lmcMessaging::settingsChanged() {
    pNetwork->settingsChanged();

    QString userName = getUserName();
    if(localUser->name.compare(userName) != 0) {
        localUser->setName(userName);

        XmlMessage xmlMessage;
        xmlMessage.addData(XN_NAME, userName);
        QString nullString = QString::null;
        sendMessage(MT_UserName, nullString, xmlMessage);
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

    if (groupSettings.isWritable())
#ifdef Q_OS_WIN
        if (!QFileInfo(QString("%1.lock").arg(groupSettings.fileName())).exists())
#endif
        groupSettings.sync();

    // make sure the correct version is set in the preferences file
    // so the group settings will not be wrongly migrated next time
    // application starts
    Globals::getInstance().setVersion(IDA_VERSION);
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
            pNetwork->setLocalId(userId);
        }
    }
    emit connectionStateChanged();
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
    QString userName = Globals::getInstance().userName();
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

void lmcMessaging::getUserInfo(XmlMessage &message) {
    QString firstName = Globals::getInstance().userFirstName();
    QString lastName = Globals::getInstance().userLastName();
    QString about = Globals::getInstance().userAbout();
    firstName = firstName.isEmpty() ? "N/A" : firstName;
    lastName = lastName.isEmpty() ? "N/A" : lastName;
    about = about.isEmpty() ? "N/A" : about;

    message.addData(XN_USERID, localUser->id);
    message.addData(XN_NAME, localUser->name);
    message.addData(XN_ADDRESS, localUser->address);
    message.addData(XN_VERSION, localUser->version);
    message.addData(XN_STATUS, localUser->status);
    message.addData(XN_NOTE, localUser->note);
    message.addData(XN_LOGON, Helper::getLogonName());
    message.addData(XN_HOST, Helper::getHostName());
    message.addData(XN_OS, Helper::getOSName());
    message.addData(XN_FIRSTNAME, firstName);
    message.addData(XN_LASTNAME, lastName);
    message.addData(XN_ABOUT, about);
}

bool lmcMessaging::addUser(QString userId, const QString &szVersion, const QString &userIP, const QString &szName, const QString &szStatus, const QString &szAvatar, const QString &szNote, const QString &szCaps, const QString &hostName) {
    for(int index = 0; index < userList.count(); index++)
        if(userList[index].id.compare(userId) == 0) {
            return false;
        }

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.addUser started -|- Adding new user: %1, %2, %3").arg(userId, szVersion, userIP));

    if(!userGroupMap.contains(userId) || !groupList.contains(Group(userGroupMap.value(userId))))
        userGroupMap.insert(userId, GRP_DEFAULT_ID);

    uint nAvatar = szAvatar.isNull() ? -1 : szAvatar.toInt();

    userList.append(User(userId, szVersion, userIP, szName, szStatus, userGroupMap[userId],
                         nAvatar, szNote, ImagesList::getInstance ().getAvatar (nAvatar), szCaps, hostName));
    if(!szStatus.isNull()) {
        XmlMessage xmlMessage;
        xmlMessage.addHeader(XN_FROM, userId);
        xmlMessage.addData(XN_STATUS, szStatus);
        //	send a status message to app layer, this is different from announce message
        emit messageReceived(MT_Status, userId, xmlMessage);
        if(Globals::getInstance ().getStatusType (szStatus) == StatusTypeEnum::StatusOffline) { // offline status
            LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.addUser ended-|- User was not added: user is offline"));
            return false;	//	no need to send a new user message to app layer
        }
    }

    emit messageReceived(MT_Announce, userId, XmlMessage());
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.addUser ended"));
    return true;
}

void lmcMessaging::updateUser(MessageType type, const QString &userId, const QString &userData) {
    User *user = getUser(userId);
    if(!user)
        return;

    XmlMessage updateMsg;
    switch(type) {
    case MT_Status:
        if(user->status.compare(userData) != 0) {
            QString oldStatus = user->status;
            user->status = userData;

            if(Globals::getInstance ().getStatusType (oldStatus) == StatusTypeEnum::StatusOffline) // old status is offline
                emit messageReceived(MT_Announce, userId, XmlMessage());

            updateMsg.addData(XN_STATUS, user->status);
            emit messageReceived(MT_Status, userId, updateMsg);

            if(Globals::getInstance ().getStatusType (user->status) == StatusTypeEnum::StatusOffline) { // new status is offline
                // Send a dummy xml message. A non null xml message implies that the
                // user is only in offline status, and not actually offline.

                LoggerManager::getInstance().writeWarning(QString("User %1 has offline status (%2)").arg(user->name, user->status));
                emit messageReceived(MT_Depart, userId, XmlMessage());
            }
        }
        break;
    case MT_UserName:
        if(user->name.compare(userData) != 0) {
            user->setName(userData);
            updateMsg.addData(XN_NAME, user->name);
            emit messageReceived(MT_UserName, userId, updateMsg);
        }
        break;
    case MT_Note:
        if(user->note.compare(userData) != 0) {
            user->note = userData;
            updateMsg.addData(XN_NOTE, user->note);
            emit messageReceived(MT_Note, userId, updateMsg);
        }
        break;
    case MT_Group:
        user->group = userData;
        userGroupMap.insert(user->id, user->group);
        saveGroups();
        break;
    case MT_Avatar:
        user->avatarPath = userData;
        break;
    default:
        break;
    }
}

void lmcMessaging::removeUser(const QString &userId) {
    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.removeUser started -|- Removing user %1").arg(userId));

    for(int index = 0; index < userList.count(); index++)
        if(userList.value(index).id.compare(userId) == 0) {
            XmlMessage statusMsg;
            statusMsg.addData(XN_STATUS, Globals::getInstance ().getStatuses ().back ().description);
            emit messageReceived(MT_Status, userId, statusMsg);
            emit messageReceived(MT_Depart, userId, XmlMessage());
            userList.removeAt(index);
            LoggerManager::getInstance().writeInfo(QString("lmcMessaging.removeUser ended -|- User %1 removed").arg(userId));
            return;
        }

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.removeUser ended -|- User %1 not found in the list of users").arg(userId));
}

bool lmcMessaging::addReceivedMsg(qint64 msgId, const QString &userId) {
    ReceivedMsg received(msgId, userId);

    if(receivedList.contains(received))
        return false;

    receivedList.append(received);
    return true;
}
