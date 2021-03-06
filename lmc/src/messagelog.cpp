﻿/****************************************************************************
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

#include "messagelog.h"
#include "loggermanager.h"
#include "imageslist.h"
#include "settings.h"
#include "globals.h"

#include <QMenu>
#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QStandardPaths>

const QString acceptOp("accept");
const QString declineOp("decline");
const QString cancelOp("cancel");

lmcMessageLog::lmcMessageLog(QWidget *parent) : QWebView(parent) {
    connect(this, &lmcMessageLog::linkClicked, this, &lmcMessageLog::log_linkClicked);
    connect(this->page()->mainFrame(), &QWebFrame::contentsSizeChanged,
            this, &lmcMessageLog::log_contentsSizeChanged);
    connect(this->page(), &QWebPage::linkHovered,
            this, &lmcMessageLog::log_linkHovered);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    settings()->setObjectCacheCapacities(0, 0, 0);

    createContextMenu();

    participantAvatars.clear();
    hasData = false;
    sendFileMap.clear();
    receiveFileMap.clear();
    lastId = QString::null;
    messageLog.clear();
    linkHovered = false;
    outStyle = false;
    autoScroll = true;
}

lmcMessageLog::~lmcMessageLog() {
}

void lmcMessageLog::initMessageLog(bool themePreviewMode, bool clearLog, bool reloadMessages) {
    LoggerManager::getInstance ().writeInfo (QStringLiteral("initMessageLog.initMessageLog started"));

    if(clearLog)
        messageLog.clear();

    if (reloadMessages)
        reloadMessageLog();
    lastId = QString::null;

    _themePreviewMode = themePreviewMode;

    setHtml(getTheme().document);

    LoggerManager::getInstance ().writeInfo (QString("initMessageLog.initMessageLog ended -|- Theme: %1").arg (themeData.themePath));
}

void lmcMessageLog::createContextMenu() {
    QAction* action = pageAction(QWebPage::Copy);
    action->setShortcut(QKeySequence::Copy);
    addAction(action);
    action = pageAction(QWebPage::CopyLinkToClipboard);
    addAction(action);
    action = pageAction(QWebPage::SelectAll);
    action->setShortcut(QKeySequence::SelectAll);
    addAction(action);

    contextMenu = new QMenu(this);
    copyAction = contextMenu->addAction("&Copy", this, SLOT(copyAction_triggered()), QKeySequence::Copy);
    copyLinkAction = contextMenu->addAction("&Copy Link", this, SLOT(copyLinkAction_triggered()));
    contextMenu->addSeparator();
    selectAllAction = contextMenu->addAction("Select &All", this,
                            SLOT(selectAllAction_triggered()), QKeySequence::SelectAll);
    connect(this, &lmcMessageLog::customContextMenuRequested, this, &lmcMessageLog::showContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void lmcMessageLog::appendMessageLog(MessageType type, const QString &userId, const QString &userName, const XmlMessage &message,
        bool reload, bool groupMessage, bool saveLog, User *localUser, const QHash<QString, QString> &peersList) {
    if(!message.isValid() && type != MT_Error)
        return;

    QString messageString;
    QString html;
    QString caption;
    QDateTime time;
    QFont font;
    QColor color;
    QString fontStyle;
    QString id = QString::null;
    bool addToLog = true;

    removeMessageLog("_lmc_statediv");

    switch(type) {
    case MT_GroupMessage:
    case MT_Message:
        time.setMSecsSinceEpoch(message.header(XN_TIME).toLongLong());
        messageString = message.data(XN_MESSAGE);
        font.fromString(message.data(XN_FONT));
        color.setNamedColor(message.data(XN_COLOR));
        appendMessage(userId, userName, messageString, time, font, color);
        lastId = userId;
        break;
    case MT_PublicMessage:
        time.setMSecsSinceEpoch(message.header(XN_TIME).toLongLong());
        messageString = message.data(XN_MESSAGE);
        font.fromString(message.data(XN_FONT));
        color.setNamedColor(message.data(XN_COLOR));
        appendPublicMessage(userId, userName, messageString, time, font, color);
        lastId = userId;
        break;
    case MT_Broadcast:
        time.setMSecsSinceEpoch(message.header(XN_TIME).toLongLong());
        messageString = message.data(XN_MESSAGE);
        appendBroadcast(userName, messageString, time);
        lastId  = QString::null;
        break;
    case MT_InstantMessage:
        time.setMSecsSinceEpoch(message.header(XN_TIME).toLongLong());
        messageString = message.data(XN_MESSAGE);
        font.fromString(message.data(XN_FONT));
        color.setNamedColor(message.data(XN_COLOR));
        appendInstantMessage(userId, userName, messageString, time, font, color);
        break;
    case MT_ChatState:
        messageString = message.data(XN_CHATSTATE);
        caption = getChatStateMessage((ChatState)Helper::indexOf(ChatStateNames, CS_Max, messageString));
        if(!caption.isNull()) {
            html = getTheme().stateMsg;
            html.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("blank"))).toString());
            html.replace("%sender%", caption.arg(userName));
            html.replace("%message%", "");
            appendMessageLog(html);
        }
        addToLog = false;
        break;
    case MT_Failed:
        messageString = message.data(XN_MESSAGE);
        font.fromString(message.data(XN_FONT));
        color.setNamedColor(message.data(XN_COLOR));
        html = getTheme().sysMsg;
        if (peersList.size() == 1)
            caption = tr("This message was not delivered to %1.").arg(peersList.keys()[0]);
        else
            caption = tr("This message was not delivered.");
        fontStyle = getFontStyle(font, color, true);
        decodeMessage(messageString, Globals::getInstance().trimMessages(), Globals::getInstance().showLinks(), Globals::getInstance().showPathsAsLinks(), Globals::getInstance().showEmoticons());
        html.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("criticalmsg"))).toString());
        html.replace("%sender%", caption);
        html.replace("%style%", fontStyle);
        html.replace("%message%", messageString);
        appendMessageLog(html);
        lastId  = QString::null;
        break;
    case MT_Error:
        html = themeData.sysMsg;
        html.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("criticalmsg"))).toString());
        html.replace("%sender%", tr("Your message was not sent."));
        html.replace("%message%", "");
        appendMessageLog(html);
        lastId  = QString::null;
        addToLog = false;
        break;
    case MT_File:
    case MT_Folder:
        appendFileMessage(type, userName, userId, const_cast<XmlMessage &>(message), reload);
        id = message.data(XN_TEMPID);
        lastId = QString::null;
        break;
    case MT_Join:
    case MT_Leave:
        messageString = message.data(XN_GROUPMSGOP);
        caption = getChatRoomMessage((GroupMsgOp)Helper::indexOf(GroupMsgOpNames, GMO_Max, messageString), groupMessage);
        if(!caption.isNull()) {
            html = getTheme().sysMsg;
            html.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("blank"))).toString());
            html.replace("%sender%", caption.arg(userName));
            html.replace("%message%", "");
            appendMessageLog(html);
        }
        lastId = QString::null;
    default:
        break;
    }

    if (saveLog) {
        if(!reload and addToLog and !messageString.isEmpty() and (type == MT_GroupMessage or type == MT_Message or type == MT_Broadcast or type == MT_File or type == MT_Folder or type == MT_PublicMessage)) {
            XmlMessage xmlMessage(message.toString());
            SingleMessage message(type, userId, userName, xmlMessage, id);
            messageLog.append(message);

            QString nameForLog;
            QString idForLog;
            if (type == MT_PublicMessage) {
                nameForLog = tr("Public Conversation");
                idForLog = QStringLiteral("-1");
            } else if (type == MT_GroupMessage) {
                idForLog = QStringLiteral("-2");
            } else if (peersList.size() == 1) {
                nameForLog = peersList.values().first();
                idForLog = peersList.keys().first();
            }

            saveMessageLog(nameForLog, idForLog, localUser, peersList.values(), QDateTime::currentDateTime(), message, savePath);
        }
    }
}

void lmcMessageLog::updateFileMessage(FileMode mode, FileOp op, QString fileId) {
    QString szMessage = getFileStatusMessage(mode, op);
    QWebFrame* frame = page()->mainFrame();
    QWebElement document = frame->documentElement();
    QWebElement body = document.findFirst("body");
    QString selector = "span#";
    QString tempId = (mode == FM_Send) ? "send" : "receive";
    tempId.append(fileId);
    selector.append(tempId);
    QWebElement span = body.findFirst(selector);
    span.setPlainText(szMessage);

    //	update the entry in message log
    for(int index = 0; index < messageLog.count(); index++) {
        SingleMessage msg = messageLog.at(index);
        if(tempId.compare(msg.id) == 0) {
            XmlMessage xmlMessage = msg.message;
            xmlMessage.removeData(XN_FILEOP);
            xmlMessage.addData(XN_FILEOP, FileOpNames[op]);
            msg.message = xmlMessage;
            break;
        }
    }
}

void lmcMessageLog::updateUserName(const QString &userId, const QString &userName) {
    //	update the entries in message log
    for(int index = 0; index < messageLog.count(); index++) {
        SingleMessage msg = messageLog.takeAt(index);
        if(userId.compare(msg.userId) == 0)
            msg.userName = userName;
        messageLog.insert(index, msg);
    }

    reloadMessageLog();
}

void lmcMessageLog::updateAvatar(const QString &userId, const QString &filePath) {
    participantAvatars.insert(userId, filePath);

    reloadMessageLog();
}

void lmcMessageLog::reloadMessageLog() {
    initMessageLog(false, false, false);
    for(int index = 0; index < messageLog.count(); index++) {
        SingleMessage msg = messageLog[index];
        appendMessageLog(msg.type, msg.userId, msg.userName, msg.message, true);
    }

    prependHtml(_prependedHtml);
}

QString lmcMessageLog::prepareMessageLogForSave(OutputFormat format) {
    if (format == TextFormat)
        return page()->mainFrame()->toPlainText();
    else
        return page()->mainFrame()->toHtml();
}

void lmcMessageLog::setAutoScroll(bool enable) {
    autoScroll = enable;
}

void lmcMessageLog::abortPendingFileOperations() {
    QMap<QString, XmlMessage>::iterator sIndex = sendFileMap.begin();
    while(sIndex != sendFileMap.end()) {
        XmlMessage fileData = sIndex.value();
        FileOp fileOp = (FileOp)Helper::indexOf(FileOpNames, FO_Max, fileData.data(XN_FILEOP));
        if(fileOp == FO_Request) {
            updateFileMessage(FM_Send, FO_Abort, fileData.data(XN_FILEID));
            sIndex.value().removeData(XN_FILEOP);
            sIndex.value().addData(XN_FILEOP, FileOpNames[FO_Abort]);
        }
        sIndex++;
    }
    QMap<QString, XmlMessage>::iterator rIndex = receiveFileMap.begin();
    while(rIndex != receiveFileMap.end()) {
        XmlMessage fileData = rIndex.value();
        FileOp fileOp = (FileOp)Helper::indexOf(FileOpNames, FO_Max, fileData.data(XN_FILEOP));
        if(fileOp == FO_Request) {
            updateFileMessage(FM_Receive, FO_Abort, fileData.data(XN_FILEID));
            rIndex.value().removeData(XN_FILEOP);
            rIndex.value().addData(XN_FILEOP, FileOpNames[FO_Abort]);
        }
        rIndex++;
    }
}

void lmcMessageLog::saveMessageLog(const QString &userName, const QString &userId, User *localUser, const QList<QString> &peersList, const QDateTime &date, SingleMessage &message, const QString &savePath) {
    // TODO !!! add log
    if (message.message.data(XN_MESSAGE).isEmpty() && message.message.data(XN_FILENAME).isEmpty())
        return;

    QDir dir = QFileInfo(savePath).dir();
    if(!dir.exists())
        dir.mkpath(dir.absolutePath());

    QFile file(savePath);
    bool fileExists = file.exists();
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;

    QDataStream stream(&file);

    if (!fileExists) {
        if (!userName.isEmpty() && !userId.isEmpty())
            stream << userName << userId << date.toMSecsSinceEpoch() << peersList;
        else if (localUser)
            stream << localUser->name << localUser->id << date.toMSecsSinceEpoch() << peersList;
        else
            LoggerManager::getInstance().writeWarning(QStringLiteral("lmcMessageLog.saveMessageLog-|- No peer and no local user specified. History will not be saved."));
    }

    stream << message;

    file.close();
}

void lmcMessageLog::changeEvent(QEvent* event) {
    switch(event->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QWidget::changeEvent(event);
}

void lmcMessageLog::log_linkClicked(QUrl url) {
    QString linkPath = url.toString();

    //	this is a hack so qdesktopservices can open a network path
    if(linkPath.startsWith("file")) {
        // strip out the 'file:' prefix and get the path
        linkPath = linkPath.mid(5);
        // use a url that contains native separators

        QDesktopServices::openUrl(QUrl::fromLocalFile(linkPath));
        return;
    } else if(linkPath.startsWith("www")) {
        // prepend 'http://' to link
        linkPath.prepend("http://");
        QDesktopServices::openUrl(QUrl(linkPath));
        return;
    } else if(!linkPath.startsWith("lmc")) {
        QDesktopServices::openUrl(url);
        return;
    }

    QStringList linkData = linkPath.split("/", QString::SkipEmptyParts);
    FileMode mode;
    FileOp op;

    if(!linkData[3].compare(acceptOp)) {
        mode = FM_Receive;
        op = FO_Accept;
    } else if(!linkData[3].compare(declineOp)) {
        mode = FM_Receive;
        op = FO_Decline;
    } else if(!linkData[3].compare(cancelOp)) {
        mode = FM_Send;
        op = FO_Cancel;
    } else	// unknown link command
        return;

    //	Remove the link and show a confirmation message.
    updateFileMessage(mode, op, linkData[4]);

    fileOperation(linkData[4], linkData[3], linkData[1], linkData[2], mode);
}

void lmcMessageLog::log_contentsSizeChanged(QSize size) {
    if(autoScroll) {
        QWebFrame* frame = page()->mainFrame();
        frame->scroll(0, size.height());
    }
}

void lmcMessageLog::log_linkHovered(QString link, QString title, QString textContent) {
    Q_UNUSED(title);
    Q_UNUSED(textContent);
    linkHovered = !link.isEmpty();
}

void lmcMessageLog::showContextMenu(const QPoint& pos) {
    copyAction->setEnabled(!selectedText().isEmpty());
    copyLinkAction->setEnabled(linkHovered);
    //	Copy Link is currently hidden since it performs the same action as regular Copy
    //copyLinkAction->setVisible(false);
    selectAllAction->setEnabled(!page()->mainFrame()->documentElement().findFirst("body").firstChild().isNull());
    contextMenu->exec(mapToGlobal(pos));
}

void lmcMessageLog::copyAction_triggered() {
    pageAction(QWebPage::Copy)->trigger();
}

void lmcMessageLog::copyLinkAction_triggered() {
    pageAction(QWebPage::CopyLinkToClipboard)->trigger();
}

void lmcMessageLog::selectAllAction_triggered() {
    pageAction(QWebPage::SelectAll)->trigger();
}

void lmcMessageLog::appendMessageLog(const QString &htmlString) {
    QWebFrame* frame = page()->mainFrame();
    QWebElement document = frame->documentElement();
    QWebElement body = document.findFirst("body");
    body.appendInside(htmlString);
}

void lmcMessageLog::removeMessageLog(const QString &divClass) {
    QWebFrame* frame = page()->mainFrame();
    QWebElement document = frame->documentElement();
    QWebElement body = document.findFirst("body");
    QWebElement element = body.findFirst("div." + divClass);
    element.removeFromDocument();
}

void lmcMessageLog::appendBroadcast(const QString &userName, QString &message, const QDateTime &dateTime) {
    decodeMessage(message, Globals::getInstance().trimMessages(), Globals::getInstance().showLinks(), Globals::getInstance().showPathsAsLinks(), Globals::getInstance().showEmoticons());

    QString html = getTheme().pubMsg;
    QString caption = tr("Broadcast message from %1:");
    html.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("broadcastformsg"))).toString());
    html.replace("%sender%", caption.arg(userName));
    html.replace("%time%", getTimeString(dateTime));
    html.replace("%style%", "");
    html.replace("%message%", message);

    appendMessageLog(html);
}

void lmcMessageLog::appendInstantMessage(const QString &userId, const QString &userName, QString &message, const QDateTime &dateTime,
                                         const QFont &font, const QColor &color) {
    bool localUser = (userId.compare(localId) == 0);

    decodeMessage(message, Globals::getInstance().trimMessages(), Globals::getInstance().showLinks(), Globals::getInstance().showPathsAsLinks(), Globals::getInstance().showEmoticons());

    //	get the avatar image for this user from the cache folder
    QString filePath = participantAvatars.value(userId);
    //	if image not found, use the default avatar image for this user
    QString iconPath = QUrl::fromLocalFile (QFile::exists(filePath) ? filePath : ImagesList::getInstance().getDefaultAvatar()).toString();
    QString fontStyle = getFontStyle(font, color, localUser);

    QString html = getTheme().inMsg;
    html.replace("%iconpath%", iconPath);
    html.replace("%sender%", userName);
    html.replace("%time%", getTimeString(dateTime));
    html.replace("%style%", fontStyle);
    html.replace("%message%", message);

    appendMessageLog(html);
}

void lmcMessageLog::prependHtml(const QString &html) {
    QWebFrame* frame = page()->mainFrame();
    QWebElement document = frame->documentElement();
    document.prependInside(html);
    lastId.clear();

    _prependedHtml.append(html);
}

void lmcMessageLog::appendMessage(const QString &userId, const QString &userName, QString &message, const QDateTime &dateTime,
                                  const QFont &font, const QColor &color) {
    QString html = QString::null;
    bool localUser = (userId.compare(localId) == 0);

    decodeMessage(message, Globals::getInstance().trimMessages(), Globals::getInstance().showLinks(), Globals::getInstance().showPathsAsLinks(), Globals::getInstance().showEmoticons());

    QString fontStyle = getFontStyle(font, color, localUser);

    if(userId.compare(lastId) != 0) {
        html = localUser ? getTheme().outMsg : getTheme().inMsg;

        //	get the avatar image for this user from the cache folder
        QString filePath = participantAvatars.value(userId);
        //	if image not found, use the default avatar image for this user
        QString iconPath = QUrl::fromLocalFile (QFile::exists(filePath) ? filePath : ImagesList::getInstance().getDefaultAvatar()).toString();

        html.replace("%iconpath%", iconPath);
        html.replace("%sender%", userName);
        html.replace("%time%", getTimeString(dateTime));
        html.replace("%style%", fontStyle);
        html.replace("%message%", message);

        QWebFrame* frame = page()->mainFrame();
        QWebElement document = frame->documentElement();
        QWebElement body = document.findFirst("body");
        body.appendInside(html);
    } else {
        html = localUser ? getTheme().outNextMsg : getTheme().inNextMsg;
        html.replace("%time%", getTimeString(dateTime));
        html.replace("%style%", fontStyle);
        html.replace("%message%", message);

        QWebFrame* frame = page()->mainFrame();
        QWebElement document = frame->documentElement();
        QWebElement body = document.findFirst("body");
        QWebElement last = body.lastChild();
        QWebElement insert = last.findFirst("div#insert");
        insert.replace(html);
    }

    hasData = true;
}

void lmcMessageLog::appendPublicMessage(const QString &userId, const QString &userName, QString &message,
                                        const QDateTime &dateTime, const QFont &font, const QColor &color) {
    QString html = QString::null;
    bool localUser = (userId.compare(localId) == 0);

    decodeMessage(message, Globals::getInstance().trimMessages(), Globals::getInstance().showLinks(), Globals::getInstance().showPathsAsLinks(), Globals::getInstance().showEmoticons());

    QString fontStyle = getFontStyle(font, color, localUser);

    if(userId.compare(lastId) != 0) {
        outStyle = !outStyle;
        html = outStyle ? getTheme().outMsg : getTheme().inMsg;

        //	get the avatar image for this user from the cache folder
        QString filePath = participantAvatars.value(userId);
        //	if image not found, use the default avatar image for this user
        QString iconPath = QUrl::fromLocalFile (QFile::exists(filePath) ? filePath : ImagesList::getInstance().getDefaultAvatar()).toString();

        html.replace("%iconpath%", iconPath);
        html.replace("%sender%", userName);
        html.replace("%time%", getTimeString(dateTime));
        html.replace("%style%", fontStyle);
        html.replace("%message%", message);

        QWebFrame* frame = page()->mainFrame();
        QWebElement document = frame->documentElement();
        QWebElement body = document.findFirst("body");
        body.appendInside(html);
    } else {
        html = outStyle ? getTheme().outNextMsg : getTheme().inNextMsg;
        html.replace("%time%", getTimeString(dateTime));
        html.replace("%style%", fontStyle);
        html.replace("%message%", message);

        QWebFrame* frame = page()->mainFrame();
        QWebElement document = frame->documentElement();
        QWebElement body = document.findFirst("body");
        QWebElement last = body.lastChild();
        QWebElement insert = last.findFirst("div#insert");
        insert.replace(html);
    }

    hasData = true;
}

// This function is called to display a file request message on chat box
void lmcMessageLog::appendFileMessage(MessageType type, const QString &userName, const QString &userId, XmlMessage &message,
                                      bool reload) {
    QString htmlMsg;
    QString caption;
    QString fileId = message.data(XN_FILEID);
    QString tempId;
    QString status;
    QString fileType;

    switch(type) {
    case MT_File:
        fileType = "file";
        break;
    case MT_Folder:
        fileType = "folder";
        break;
    default:
        return;
        break;
    }

    htmlMsg = getTheme().reqMsg;
    htmlMsg.replace("%iconpath%", QUrl::fromLocalFile(ThemeManager::getInstance().getAppIcon(QStringLiteral("filemsg"))).toString());

    FileOp fileOp = (FileOp)Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
    FileMode fileMode = (FileMode)Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));

    if(fileMode == FM_Send) {
        tempId = "send" + fileId;
        caption = tr("Sending '%1' to %2.");
        htmlMsg.replace("%sender%", caption.arg(message.data(XN_FILENAME), userName));
        htmlMsg.replace("%message%", "");
        htmlMsg.replace("%fileid%", tempId);

        switch(fileOp) {
        case FO_Request:
            sendFileMap.insert(fileId, message);
            message.removeData(XN_TEMPID);
            message.addData(XN_TEMPID, tempId);
            htmlMsg.replace("%links%", QString("<a href='lmc://%1/%2/%3/%4'>%5</a> &nbsp;").arg(fileType, userId, cancelOp, fileId,  tr("Cancel")));
            break;
        case FO_Cancel:
        case FO_Accept:
        case FO_Decline:
        case FO_Error:
        case FO_Abort:
        case FO_Complete:
            status = getFileStatusMessage(FM_Send, fileOp);
            htmlMsg.replace("%links%", status);
            break;
        default:
            return;
            break;
        }
    } else {
        tempId = "receive" + fileId;
        if(Globals::getInstance().autoReceiveFiles()) {
            if(type == MT_File)
                caption = tr("%1 is sending you a file:");
            else
                caption = tr("%1 is sending you a folder:");
            htmlMsg.replace("%sender%", caption.arg(userName));
            htmlMsg.replace("%message%", message.data(XN_FILENAME) + " (" +
                Helper::formatSize(message.data(XN_FILESIZE).toLongLong()) + ")");
            htmlMsg.replace("%fileid%", "");
        } else {
            if(type == MT_File)
                caption = tr("%1 sends you a file:");
            else
                caption = tr("%1 sends you a folder:");
            htmlMsg.replace("%sender%", caption.arg(userName));
            htmlMsg.replace("%message%", message.data(XN_FILENAME) + " (" +
                Helper::formatSize(message.data(XN_FILESIZE).toLongLong()) + ")");
            htmlMsg.replace("%fileid%", tempId);
        }

        switch(fileOp) {
        case FO_Request:
            receiveFileMap.insert(fileId, message);
            message.removeData(XN_TEMPID);
            message.addData(XN_TEMPID, tempId);

            if(Globals::getInstance().autoReceiveFiles()) {
                htmlMsg.replace("%links%", tr("Accepted"));
                if(!reload)
                    fileOperation(fileId, acceptOp, fileType, userId);
            } else {
                htmlMsg.replace("%links%",
                    QString("<a href='lmc://%1/%2/%3/%4'>%5</a> &nbsp;"
                            "<a href='lmc://%6/%7/%8/%9'>%10</a>").arg(fileType, userId, acceptOp, fileId,  tr("Accept")).arg(fileType, userId, declineOp, fileId, tr("Decline")));
            }
            break;
        case FO_Cancel:
        case FO_Accept:
        case FO_Decline:
        case FO_Error:
        case FO_Abort:
        case FO_Complete:
            status = getFileStatusMessage(FM_Receive, fileOp);
            htmlMsg.replace("%links%", status);
            break;
        default:
            return;
            break;
        }
    }

    QWebFrame* frame = page()->mainFrame();
    QWebElement document = frame->documentElement();
    QWebElement body = document.findFirst("body");
    body.appendInside(htmlMsg);
}

QString lmcMessageLog::getFontStyle(const QFont &font, const QColor &color, bool localUser) {
    QString style;

    if(font.italic())
        style.append("font-style:italic; ");
    if(font.bold())
        style.append("font-weight:bold; ");
    if(font.strikeOut())
        style.append("text-decoration:line-through; ");
    if(font.underline())
        style.append("text-decoration:underline; ");

    if (!Globals::getInstance().overrideInMessagesStyle() or localUser) {
        style.append(QString("font-family:\"%1\"; ").arg(font.family()));
        style.append(QString("font-size:%1pt; ").arg(QString::number(font.pointSize())));
        style.append(QString("color:%1; ").arg(color.name()));
    } else {
        style.append(QString("font-family:\"%1\"; ").arg(Globals::getInstance().messagesFont().family()));
        style.append(QString("font-size:%1pt; ").arg(QString::number(Globals::getInstance().messagesFont().pointSize())));
        style.append(QString("color:%1; ").arg(Globals::getInstance().messagesColor()));
    }

    return style;
}

QString lmcMessageLog::getFileStatusMessage(FileMode mode, FileOp op) {
    QString message;

    switch(op) {
    case FO_Accept:
        message = (mode == FM_Send) ? tr("Accepted") : tr("Accepted");
        break;
    case FO_Decline:
        message = (mode == FM_Send) ? tr("Declined") : tr("Declined");
        break;
    case FO_Cancel:
        message = (mode == FM_Send) ? tr("Canceled") : tr("Canceled");
        break;
    case FO_Error:
    case FO_Abort:
        message = (mode == FM_Send) ? tr("Interrupted") : tr("Interrupted");
        break;
    case FO_Complete:
        message = (mode == FM_Send) ? tr("Completed") : tr("Completed");
        break;
    default:
        break;
    }

    return message;
}

QString lmcMessageLog::getChatStateMessage(ChatState chatState) {
    QString message = QString::null;

    switch(chatState) {
    case CS_Composing:
        message = tr("%1 is typing...");
        break;
    case CS_Paused:
        message = tr("%1 has stopped typing");
        break;
    case CS_Read:
        message = tr("%1 has read the message");
        break;
    default:
        break;
    }

    return message;
}

QString lmcMessageLog::getChatRoomMessage(GroupMsgOp op, bool groupMessage) {
    QString message = QString::null;

    switch(op) {
    case GMO_Join:
        message = tr("%1 has joined this conversation");
        break;
    case GMO_Leave:
        if (groupMessage)
            message = tr("%1 has left this conversation");
        else
            message = tr("%1 went offline");
        break;
    default:
        break;
    }

    return message;
}

void lmcMessageLog::fileOperation(QString fileId, const QString &action, const QString &fileType, const QString &senderId, FileMode mode) {
    XmlMessage fileData, xmlMessage;

    MessageType type;
    if(fileType.compare("file") == 0)
        type = MT_File;
    else if(fileType.compare("folder") == 0)
        type = MT_Folder;
    else
        return;

    if(action.compare(acceptOp) == 0) {
        fileData = receiveFileMap.value(fileId);
        xmlMessage.addData(XN_MODE, FileModeNames[FM_Receive]);
        xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
        xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Accept]);
        xmlMessage.addData(XN_FILEID, fileData.data(XN_FILEID));
        xmlMessage.addData(XN_FILEPATH, fileData.data(XN_FILEPATH));
        xmlMessage.addData(XN_FILENAME, fileData.data(XN_FILENAME));
        xmlMessage.addData(XN_FILESIZE, fileData.data(XN_FILESIZE));
    }
    else if(action.compare(declineOp) == 0) {
        fileData = receiveFileMap.value(fileId);
        xmlMessage.addData(XN_MODE, FileModeNames[FM_Receive]);
        xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
        xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Decline]);
        xmlMessage.addData(XN_FILEID, fileData.data(XN_FILEID));
    }
    else if(action.compare(cancelOp) == 0) {
        if(mode == FM_Receive)
            fileData = receiveFileMap.value(fileId);
        else
            fileData = sendFileMap.value(fileId);
        xmlMessage.addData(XN_MODE, FileModeNames[mode]);
        xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
        xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Cancel]);
        xmlMessage.addData(XN_FILEID, fileData.data(XN_FILEID));
    }

    emit messageSent(type, senderId, xmlMessage);
}

//	Called when message received, before adding to message log
//	The useDefaults parameter is an override flag that will ignore app settings
//	while decoding the message. If the flag is set, message is not trimmed,
//	smileys are left as text and links will be detected and converted.
void lmcMessageLog::decodeMessage(QString &message, bool trimMessage, bool allowLinks, bool pathToLink, bool showSmileys, bool useDefaults) {
    if(!useDefaults && trimMessage)
        message = message.trimmed();

    //	The url detection regexps only work with plain text, so link detection is done before
    //	making the text html safe. The converted links are given a "data-isLink" custom
    //	attribute to differentiate them from the message content
    if(useDefaults || allowLinks) {
        message.replace(QRegExp("((?:(?:https?|ftp|file)://|www\\.|ftp\\.)[-A-Z0-9+&@#/%=~_|$?!:,.]*[A-Z0-9+&@#/%=~_|$])", Qt::CaseInsensitive),
                             "<a data-isLink='true' href='\\1'>\\1</a>");
        message.replace("<a data-isLink='true' href='www", "<a data-isLink='true' href='http://www");

        if(!useDefaults && pathToLink)
            message.replace(QRegExp("((\\\\\\\\[\\w-]+\\\\[^\\\\/:*?<>|""]+)((?:\\\\[^\\\\/:*?<>|""]+)*\\\\?)$)"),
                                 "<a data-isLink='true' href='file:\\1'>\\1</a>");
        // Alternative: ^(?:[a-zA-Z]\:|\\\\\\\\[\\w\\.]+\\[\\w.$]+)\\\\(?:[\\w]+\\)*\\w([\\w.])+$

        QString erpAddress = Globals::getInstance().erpAddress();
        if (!erpAddress.endsWith('/'))
            erpAddress.append('/');

        // replace ITx, COMMx, ATTx and VIDx with links
        message.replace(QRegExp("\\b(IT(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?IssueId=\\2' title='%1Pages/Issues/IssueDetail.aspx?IssueId=\\2'>\\1</a>").arg(erpAddress));
        message.replace(QRegExp("\\b(COMM(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?CommentId=\\2#COMMN\\2' title='%1erp/Pages/Issues/IssueDetail.aspx?CommentId=\\2#COMMN\\2'>\\1</a>").arg(erpAddress));
        message.replace(QRegExp("\\b(VID(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?VideoId=\\2' title='%1erp/Pages/Issues/IssueDetail.aspx?VideoId=\\2'>\\1</a>").arg(erpAddress));
        message.replace(QRegExp("\\b(ATT(\\d+))\\b", Qt::CaseInsensitive),  QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?AttachmentId=\\2' title='%1Pages/Issues/IssueDetail.aspx?AttachmentId=\\2'>\\1</a>").arg(erpAddress));
    }

    QString temp = QString::null;
    int index = 0;

    while(index < message.length()) {
        int aStart = message.indexOf("<a data-isLink='true'", index);
        if(aStart != -1) {
            QString messageSegment = message.mid(index, aStart - index);
            processMessageText(messageSegment, showSmileys, useDefaults);
            temp.append(messageSegment);
            index = message.indexOf("</a>", aStart) + 4;
            QString linkSegment = message.mid(aStart, index - aStart);
            temp.append(linkSegment);
        } else {
            QString messageSegment = message.mid(index);
            processMessageText(messageSegment, showSmileys, useDefaults);
            temp.append(messageSegment);
            break;
        }
    }

    // TODO this is not nice, use regex to turn [] into <>
    temp = temp.replace("\n", "<br />").replace("[b]", "<b>").replace("[/b]", "</b>").replace("[u]", "<u>").replace("[/u]", "</u>").replace("[i]", "<i>").replace("[/i]", "</i>").replace("[q]", "<q>").replace("[/q]", "</q>").replace("[quote]", "<blockquote>").replace("[/quote]", "</blockquote>");

    message = temp;
}

void lmcMessageLog::processMessageText(QString &messageText, bool showSmileys, bool useDefaults) {
    messageText = messageText.toHtmlEscaped();

    //	if smileys are enabled, replace text emoticons with corresponding images
    if(!useDefaults && showSmileys)
        ChatHelper::decodeSmileys(messageText, ImagesList::getInstance().getSmileysRegex(), false);

    ChatHelper::decodeSmileys(messageText, ImagesList::getInstance().getEmojisRegex(), true);

    messageText.replace("  ", "&nbsp;&nbsp;"); // TODO this is not okay, should figure out a solution for the html safe/unsafe thing
}

QString lmcMessageLog::getTimeString(const QDateTime &dateTime) {
    QString timeStamp;
    if(Globals::getInstance().showMessageTime()) {
        timeStamp.append("(");
        if(Globals::getInstance().showMessageDate())
            timeStamp.append(dateTime.date().toString(Qt::SystemLocaleShortDate) + "&nbsp;");
        timeStamp.append(dateTime.time().toString(Qt::SystemLocaleShortDate) + ")&nbsp;");
    }

    return timeStamp;
}

const ChatThemeStruct &lmcMessageLog::getTheme()
{
    return ThemeManager::getInstance().currentChatTheme(_themePreviewMode);
}

void lmcMessageLog::setUIText() {
    copyAction->setText(tr("&Copy"));
    selectAllAction->setText(tr("Select &All"));
    reloadMessageLog();
}
