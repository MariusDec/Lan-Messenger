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


#ifndef CHATROOMWINDOW_H
#define CHATROOMWINDOW_H

#include <QWidget>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QFile>
#include <QTextStream>
#include <QWebFrame>
#include <QWebElement>
#include <qevent.h>
#include <QClipboard>
#include "ui_chatroomwindow.h"
#include "shared.h"
#include "settings.h"
#include "history.h"
#include "messagelog.h"
#include "subcontrols.h"
#include "imagepickeraction.h"
#include "soundplayer.h"
#include "chatdefinitions.h"
#include "chathelper.h"
#include "stdlocation.h"
#include "xmlmessage.h"

class lmcChatRoomWindow : public QWidget
{
    Q_OBJECT

public:
    explicit lmcChatRoomWindow(QWidget *parent = 0, bool isPublicChat = false);
    ~lmcChatRoomWindow();

    void init(User* pLocalUser, bool connected, const QString &thread = QString::null);
    void show();
    void stop();
    void addUser(User* pUser);
    void updateUser(User* pUser);
    void removeUser(QString* lpszUserId);
    void setHtml(const QString &html);
    void receiveMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void connectionStateChanged(bool connected);
    void settingsChanged();
    void selectContacts(const std::vector<User *> selectedContacts);
    bool isPublicChat () { return _isPublicChat; }
    bool isClosed () { return isHidden(); }
    const QString &getRoomId() { return _chatRoomId; }
    const QString &getOriginalRoomId() { return _originalChatRoomId; }
    void changeRoomId(const QString &newRoomId) { _chatRoomId = newRoomId; }
    const QHash<QString, QString> &getUsers() const { return _peerNames; }
    const QString &getLastUserId() const { return _lastUserId; }

    QStringList mimeTypes() const {
      return QStringList(
          {"text/uri-list"});
    }

signals:
    void messageSent(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void chatStarting(QString lpszThreadId, XmlMessage message, QStringList contacts);
    void contactsAdding(QStringList* excludeList);
    void showHistory(QString userId);
    void showTransfers(QString userId);
    void showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId, QString szTitle, TrayMessageIcon icon);
    void closed(QString* lpszThreadId);

protected:
    bool eventFilter(QObject* pObject, QEvent* pEvent);
    void changeEvent(QEvent* pEvent);
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent* pEvent);

private slots:
    void userConversationAction_triggered();
    void userFileAction_triggered();
    void userFolderAction_triggered();
    void userInfoAction_triggered();
    void buttonFont_clicked();
    void buttonFontColor_clicked();
    void buttonFile_clicked();
    void buttonFolder_clicked();
    void buttonSave_clicked();
    void buttonSendClipboard_clicked();
    void buttonHistory_clicked();
    void buttonTransfers_clicked();
    void buttonAddUsers_clicked();
    void buttonLeaveChat_clicked();
    void buttonToggleSideBar_clicked();
    void smileyAction_triggered();
    void emojiAction_triggered();
    void log_sendMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void treeWidgetUserList_itemActivated(QTreeWidgetItem* pItem, int column);
    void treeWidgetUserList_itemContextMenu(QTreeWidgetItem* pItem, QPoint& pos);
    void checkChatState();

private:
    void createUserMenu();
    void createSmileyMenu();
    void createEmojiMenu();
    void createToolBar();
    void setUIText();
    void sendMessage();
//    void encodeMessage(QString* lpszMessage);
    void appendMessageLog(MessageType type, QString* lpszUserId, QString* lpszUserName, XmlMessage* pMessage);
    void showStatus(int flag, bool isLocalUser);
    QString getWindowTitle();
    void setMessageFont(QFont& font);
    void updateStatusImage(QTreeWidgetItem* pItem, QString* lpszStatus);
    QTreeWidgetItem* getUserItem(QString* lpszUserId);
    QTreeWidgetItem* getGroupItem(QString* lpszGroupId);
    void setUserAvatar(QString* lpszUserId, QString* lpszFilePath = 0);
    void sendFile(bool sendFolder = false);
    void sendObject(const QString &peerId, MessageType type, QString *lpszPath, const QString &roomId = QString::null);
    void processFileOp(XmlMessage* pMessage);
    void setChatState(ChatState newChatState);
    void updateFileMessage(FileMode mode, FileOp op, QString fileId);
    void toggleSideBar(bool toggle = true, bool toggled = false);
    void setClibpoardIcon(QClipboard::Mode mode = QClipboard::Clipboard);
    void insertSmileyCode(const ImagesStruct &smiley);
    QFrame *createSeparator(QWidget *parent);

    const QClipboard *_clipboard = QApplication::clipboard();

    QString _chatRoomId;
    QString _originalChatRoomId;

    QString localId;
    QString localName;
    QHash<QString, QString> _peerNames;
    QHash<QString, QString> peerIds;
    User* pLocalUser;
    QString lastSenderId;
    QString _lastUserId;


    Ui::ChatRoomWindow ui;
    lmcSettings* pSettings = nullptr;
    lmcMessageLog* pMessageLog = nullptr;
    QMenu* pUserMenu = nullptr;
    QAction* userChatAction = nullptr;
    QAction* userFileAction = nullptr;
    QAction *userFolderAction = nullptr;
    QAction* userInfoAction = nullptr;
    ThemedButton *_buttonSaveConversation = nullptr;
    ThemedButton *_buttonSendClipboard = nullptr;
    ThemedButton *_buttonHistory = nullptr;
    ThemedButton *_buttonTransfer = nullptr;
    ThemedButton *_buttonToggleSideBar = nullptr;
    QMenu *pSmileyMenu = nullptr;
    QMenu *pEmojiMenu = nullptr;
    lmcImagePickerAction *pSmileyAction = nullptr;
    lmcImagePickerAction *pEmojiAction = nullptr;
    int nSmiley;
    int nEmoji;
    bool bConnected;
    int infoFlag;
    bool showSmiley;
    bool sendKeyMod;
    bool appendHistory;
    lmcSoundPlayer* pSoundPlayer = nullptr;
    QColor messageColor;
    bool dataSaved;
    bool windowLoaded;
    ChatState chatState;
    qint64 keyStroke;
    qint64 snapKeyStroke;

    bool _leaveChatTriggered = false;

    bool _isPublicChat = false;
};

#endif // CHATROOMWINDOW_H
