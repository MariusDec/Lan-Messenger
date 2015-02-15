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


#ifndef MESSAGELOG_H
#define MESSAGELOG_H

#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include "shared.h"
#include "chatdefinitions.h"
#include "chathelper.h"
#include "xmlmessage.h"
#include "thememanager.h"

enum OutputFormat{ HtmlFormat, TextFormat };

class lmcMessageLog : public QWebView
{
    Q_OBJECT

public:
    lmcMessageLog(QWidget *parent = 0);
    ~lmcMessageLog();

    void initMessageLog(bool themePreviewMode = false, bool clearLog = true, bool reloadMessages = true);
    void appendMessageLog(MessageType type, QString* lpszUserId, QString* lpszUserName, XmlMessage* pMessage,
        bool bReload = false, bool groupMessage = true, bool saveLog = true, User *localUser = nullptr, const QHash<QString, QString> &peersList = QHash<QString, QString> ());
    void updateFileMessage(FileMode mode, FileOp op, QString fileId);
    void updateUserName(QString* lpszUserId, QString* lpszUserName);
    void updateAvatar(QString* lpszUserId, QString* lpszFilePath);
    void reloadMessageLog();
    QString prepareMessageLogForSave(OutputFormat format);
    void setAutoScroll(bool enable);
    void abortPendingFileOperations();
    static void saveMessageLog(const QString &user, const QString &userId, User *localUser, const QList<QString> &peersList, const QDateTime &date, SingleMessage &message, const QString &savePath);
    void prependHtml(const QString &html);

    static void decodeMessage(QString* lpszMessage, bool trimMessage, bool allowLinks, bool pathToLink, bool showSmileys, bool useDefaults = false);

    QString localId;
    QString demoPeerId;
    QString demoPeerName;
    std::vector<QPair<QString, QString>> peers;
    QString savePath;
    QHash<QString, QString> participantAvatars;
    QString lastId;
    bool hasData;
    bool showSmiley;
    bool autoFile;
    bool messageTime;
    bool messageDate;
    QString themePath;
    bool allowLinks;
    bool pathToLink;
    bool trimMessage;
    bool overrideIncomingStyle;
    QString defaultColor;
    QFont defaultFont;

    bool _themePreviewMode = false;

signals:
    void messageSent(MessageType type, QString* lpszUserId, XmlMessage* pMessage);

protected:
    void changeEvent(QEvent* event);

private slots:
    void log_linkClicked(QUrl url);
    void log_contentsSizeChanged(QSize size);
    void log_linkHovered(const QString& link, const QString& title, const QString& textContent);
    void showContextMenu(const QPoint& pos);
    void copyAction_triggered();
    void copyLinkAction_triggered();
    void selectAllAction_triggered();

private:
    void setUIText();
    void createContextMenu();
    void appendMessageLog(QString* lpszHtml);
    void removeMessageLog(QString divClass);
    void appendBroadcast(QString* lpszUserId, QString* lpszUserName, QString* lpszMessage, QDateTime* pTime);
    void appendMessage(QString* lpszUserId, QString* lpszUserName, QString* lpszMessage, QDateTime* pTime,
        QFont* pFont, QColor* pColor);
    void appendPublicMessage(QString* lpszUserId, QString* lpszUserName, QString* lpszMessage, QDateTime* pTime,
        QFont* pFont, QColor* pColor);
    void appendFileMessage(MessageType type, QString* lpszUserName, QString *lpszUserId, XmlMessage* pMessage, bool bReload = false);
    QString getFontStyle(QFont* pFont, QColor* pColor, bool localUser = false);
    QString getFileStatusMessage(FileMode mode, FileOp op);
    QString getChatStateMessage(ChatState chatState);
    QString getChatRoomMessage(GroupMsgOp op, bool groupMessage = true);
    void fileOperation(QString fileId, QString action, QString fileType, QString senderId, FileMode mode = FM_Receive);
    QString getTimeString(QDateTime* pTime);

    static void processMessageText(QString* lpszMessageText, bool showSmileys, bool useDefaults);

    const ChatThemeStruct &getTheme();

    QMap<QString, XmlMessage> sendFileMap;
    QMap<QString, XmlMessage> receiveFileMap;
    QList<SingleMessage> messageLog;
    ChatThemeStruct themeData;
    QMenu* contextMenu;
    QAction* copyAction;
    QAction* copyLinkAction;
    QAction* selectAllAction;
    bool linkHovered;
    bool outStyle;
    bool autoScroll;
};

#endif // MESSAGELOG_H
