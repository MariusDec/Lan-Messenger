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


#ifndef LMC_H
#define LMC_H

#include "shared.h"
#include "settings.h"
#include "messaging.h"
#include "mainwindow.h"
#include "history.h"
#include "stdlocation.h"
#include "transferwindow.h"
#include "historywindow.h"
#include "settingsdialog.h"
#include "userinfowindow.h"
#include "helpwindow.h"
#include "updatewindow.h"
#include "chatroomwindow.h"
#include "userselectdialog.h"
#include "aboutdialog.h"
#include "broadcastwindow.h"

#include <QObject>
#include <QTimer>
#include <QSysInfo>
#include <QPointer>

class lmcCore : public QObject {
    Q_OBJECT

public:
    lmcCore();
    ~lmcCore();
    void init(const QString& szCommandArgs);
    bool start();

public slots:
    bool receiveAppMessage(const QString& szMessage);
    void aboutToExit();

private slots:
    void exitApp();
    void timer_timeout();
    void startChat(QString roomId, XmlMessage message = XmlMessage(), QStringList contacts = QStringList());
    void sendMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void receiveMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void connectionStateChanged();
    void showTransfers(QString userId = QString());
    void showMessage(QString chatRoomId);
    void showHistory(QString userId = QString());
    void showSettings();
    void showHelp(QRect* pRect);
    void showUpdate(QRect* pRect);
    void showAbout();
    void showBroadcast();
    void showPublicChat();
    void historyCleared();
    void fileHistoryCleared();
    void showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId, QString szTitle = QString::null, TrayMessageIcon icon = TMI_Info);
    void updateGroup(GroupOp op, QVariant value1, QVariant value2);
    void addContacts(QStringList *pExcludList);
    void chatRoomWindow_closed(QString* lpszThreadId);

private:
    void stop();
    void loadSettings();
    void settingsChanged();
    void processMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void processFile(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void routeMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void processPublicMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void createTransferWindow();
    void showTransferWindow(bool show = false, QString userId = QString());
    void initFileTransfer(MessageType type, FileMode mode, QString* lpszUserId, XmlMessage* pMessage);
    void showUserInfo(XmlMessage* pMessage);
    void createChatRoomWindow(const QString &lpszThreadId);
    void showChatRoomWindow(lmcChatRoomWindow* chatRoomWindow, bool show, bool alert = false, bool add = false, QStringList selectedContacts = QStringList());
    void showPublicChatWindow(bool show, bool alert = false, bool open = false);
    QStringList showSelectContacts(QWidget* parent, uint caps, QStringList* excludeList);
    void showPortConflictMessage();
    void saveChatLog(QString charRoomId, bool isPath = false);
    void saveUnsavedChatLog();

    lmcSettings*					pSettings;
    QTimer*							pTimer;
    lmcMessaging*					pMessaging;
    lmcMainWindow*					pMainWindow;
    QList<lmcChatRoomWindow*>		chatRoomWindows;
    lmcTransferWindow*				pTransferWindow;
    QPointer<lmcHistoryWindow>		pHistoryWindow;
    QPointer<lmcSettingsDialog>		pSettingsDialog;
    QPointer<lmcUserInfoWindow>		pUserInfoWindow;
    QPointer<lmcHelpWindow>			pHelpWindow;
    lmcUpdateWindow*				pUpdateWindow;
    QPointer<lmcChatRoomWindow>		pPublicChatWindow;
    QPointer<lmcUserSelectDialog>	pUserSelectDialog;
    QPointer<lmcAboutDialog>		pAboutDialog;
    QPointer<lmcBroadcastWindow>	pBroadcastWindow;
    bool							messagePop;
    bool							pubMessagePop;
    QString							lang;
    bool							adaptiveRefresh;
    int								refreshTime;
    XmlMessage*						pInitParams;
};

#endif // LMC_H
