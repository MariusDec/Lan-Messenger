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
    void appendMessageLog(MessageType type, const QString &userId, const QString &userName, const XmlMessage &message,
        bool reload = false, bool groupMessage = true, bool saveLog = true, User *localUser = nullptr, const QHash<QString, QString> &peersList = QHash<QString, QString> ());
    void updateFileMessage(FileMode mode, FileOp op, QString fileId);
    void updateUserName(const QString &userId, const QString &userName);
    void updateAvatar(const QString &userId, const QString &filePath);
    void reloadMessageLog();
    QString prepareMessageLogForSave(OutputFormat format);
    void setAutoScroll(bool enable);
    void abortPendingFileOperations();
    static void saveMessageLog(const QString &userName, const QString &userId, User *localUser, const QList<QString> &peersList, const QDateTime &date, SingleMessage &message, const QString &savePath);
    void prependHtml(const QString &html);

    static void decodeMessage(QString &message, bool trimMessage, bool allowLinks, bool pathToLink, bool showSmileys, bool useDefaults = false);

    QString localId;
    QString demoPeerId;
    QString demoPeerName;
    std::vector<QPair<QString, QString>> peers;
    QString savePath;
    QHash<QString, QString> participantAvatars;
    QString lastId;
    bool hasData;

    bool _themePreviewMode = false;

signals:
    void messageSent(MessageType type, QString lpszUserId, XmlMessage pMessage);

protected:
    void changeEvent(QEvent* event);

private slots:
    void log_linkClicked(QUrl url);
    void log_contentsSizeChanged(QSize size);
    void log_linkHovered(QString link, QString title, QString textContent);
    void showContextMenu(const QPoint& pos);
    void copyAction_triggered();
    void copyLinkAction_triggered();
    void selectAllAction_triggered();

private:
    void setUIText();
    void createContextMenu();
    void appendMessageLog(const QString &htmlString);
    void removeMessageLog(const QString &divClass);
    void appendBroadcast(const QString &userName, QString &message, const QDateTime &dateTime);
    void appendInstantMessage(const QString &userId, const QString &userName, QString &message, const QDateTime &dateTime,
                              const QFont &font, const QColor &color);
    void appendMessage(const QString &userId, const QString &userName, QString &message, const QDateTime &dateTime,
                       const QFont &font, const QColor &color);
    void appendPublicMessage(const QString &userId, const QString &userName, QString &message, const QDateTime &dateTime,
        const QFont &font, const QColor &color);
    void appendFileMessage(MessageType type, const QString &userName, const QString &userId, XmlMessage &message, bool reload = false);
    QString getFontStyle(const QFont &font, const QColor &color, bool localUser = false);
    QString getFileStatusMessage(FileMode mode, FileOp op);
    QString getChatStateMessage(ChatState chatState);
    QString getChatRoomMessage(GroupMsgOp op, bool groupMessage = true);
    void fileOperation(QString fileId, const QString &action, const QString &fileType, const QString &senderId, FileMode mode = FM_Receive);
    QString getTimeString(const QDateTime &dateTime);

    static void processMessageText(QString &messageText, bool showSmileys, bool useDefaults);

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
