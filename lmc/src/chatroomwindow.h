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
#include <QMessageBox>
class lmcChatRoomWindow : public QWidget
{
    Q_OBJECT

public:
    explicit lmcChatRoomWindow(QWidget *parent = 0, bool isPublicChat = false);
    ~lmcChatRoomWindow();

    void init(User* _localUser, bool connected, const QString &thread = QString::null);
    void show();
    void stop();
    void addUser(User* pUser);
    void updateUser(User* pUser);
    void removeUser(const QString &userId);
    void setHtml(const QString &html);
    void receiveMessage(MessageType type, const QString &userId, const XmlMessage &message);
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

    void showHTMLz()
    {
        QMessageBox::information(0, 0, ui.textEditMain->toHtml() + "\n\n" + ui.textEditMain->toHtml().toHtmlEscaped());
        QMessageBox::information(0, 0, ui.textEditMain->toPlainText());

        QTextCharFormat fmt;
        fmt.setFontWeight(QFont::Bold);

        QTextCursor cursor = ui.textEditMain->textCursor();
        if (!cursor.hasSelection())
            cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(fmt);
        ui.textEditMain->mergeCurrentCharFormat(fmt);
    }

    QStringList mimeTypes() const {
      return QStringList(
          {"text/uri-list"});
    }

signals:
    void messageSent(MessageType type, QString userId, XmlMessage message);
    void chatStarting(QString roomId, XmlMessage message, QStringList contacts);
    void contactsAdding(QStringList excludeList);
    void showHistory(QString userId);
    void showTransfers(QString userId);
    void showTrayMessage(TrayMessageType type, QString message, QString chatRoomId, QString title, TrayMessageIcon icon);
    void closed(QString chatRoomId);

protected:
    bool eventFilter(QObject* object, QEvent* event);
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void moveEvent(QMoveEvent *event);

private slots:
    void userConversationAction_triggered();
    void userFileAction_triggered();
    void userFolderAction_triggered();
    void userInfoAction_triggered();
    void buttonHistory_clicked();
    void checkChatState();

private:
    void buttonFont_clicked();
    void buttonFontColor_clicked();
    void buttonFile_clicked();
    void buttonFolder_clicked();
    void buttonSave_clicked();
    void buttonSendClipboard_clicked();
    void buttonTransfers_clicked();
    void buttonAddUsers_clicked();
    void buttonLeaveChat_clicked();
    void buttonToggleSideBar_clicked();
    void smileyAction_triggered();
    void emojiAction_triggered();
    void log_sendMessage(MessageType type, QString userId, XmlMessage message);
    void treeWidgetUserList_itemActivated(QTreeWidgetItem* item, int column);
    void treeWidgetUserList_itemContextMenu(QTreeWidgetItem* item, QPoint pos);
    void textChanged();

    void createUserMenu();
    void createSmileyMenu();
    void createEmojiMenu();
    void createToolBar();
    void setUIText();
    void sendMessage();
    void appendMessageLog(MessageType type, const QString &userId, const QString &userName, const XmlMessage &message);
    void showStatus(int flag, bool isLocalUser);
    QString getWindowTitle();
    void setMessageFont(const QFont &font);
    void updateStatusImage(QTreeWidgetItem* pItem, const QString &status);
    QTreeWidgetItem* getUserItem(const QString &userId);
    QTreeWidgetItem* getGroupItem(const QString &groupId);
    void setUserAvatar(const QString &userId, const QString &filePath = QString::null);
    void sendFile(bool sendFolder = false);
    void sendObject(const QString &peerId, MessageType type, QString *lpszPath, const QString &roomId = QString::null);
    void processFileOp(const XmlMessage &message);
    void setChatState(ChatState newChatState);
    void updateFileMessage(FileMode mode, FileOp op, QString fileId);
    void toggleSideBar(bool toggle = true, bool toggled = false);
    void setClibpoardIcon(QClipboard::Mode mode = QClipboard::Clipboard);
    void insertSmileyCode(const ImagesStruct &smiley);
    void insertHtmlTag(const QString &beginTag, const QString &endTag);
    QFrame *createSeparator(QWidget *parent);

    const QClipboard *_clipboard = QApplication::clipboard();

    QString                 _chatRoomId;
    QString                 _originalChatRoomId;

    QString                 _localId;
    QString                 _localName;
    QHash<QString, QString> _peerNames;
    QHash<QString, QString> _peerIds;
    User*                   _localUser;
    QString                 _lastSenderId;
    QString                 _lastUserId;
    bool                    _hasUnreadMessages = false;


    Ui::ChatRoomWindow ui;
    lmcMessageLog* _messageLog = nullptr;
    QMenu* _userMenu = nullptr;
    QAction* _userChatAction = nullptr;
    QAction* _userFileAction = nullptr;
    QAction *_userFolderAction = nullptr;
    QAction* _userInfoAction = nullptr;
    ThemedButton *_buttonSaveConversation = nullptr;
    ThemedButton *_buttonSendClipboard = nullptr;
    ThemedButton *_buttonHistory = nullptr;
    ThemedButton *_buttonTransfer = nullptr;
    ThemedButton *_buttonToggleSideBar = nullptr;
    QMenu *_smileyMenu = nullptr;
    QMenu *_emojiMenu = nullptr;
    lmcImagePickerAction *_smileyAction = nullptr;
    lmcImagePickerAction *_emojiAction = nullptr;
    int nSmiley = -1;
    int nEmoji = -1;
    bool bConnected;
    int infoFlag;
    lmcSoundPlayer* _soundPlayer = nullptr;
    QColor _messageColor;
    bool dataSaved;
    bool windowLoaded;
    ChatState chatState;
    qint64 keyStroke;
    qint64 snapKeyStroke;

    bool _leaveChatTriggered = false;
    bool _isPublicChat = false;
    bool _fontChangedManually = false;
};

#endif // CHATROOMWINDOW_H
