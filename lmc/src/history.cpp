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

#include "history.h"
#include "imageslist.h"
#include "stdlocation.h"

#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QXmlStreamReader>
#include <QXmlSchemaValidator>
#include <QMessageBox>
#include <QXmlSchema>

QString History::historyFile(const QDate &date) {
    return StdLocation::historyFile(date);
}

QString History::historyFilesDir() {
    return StdLocation::historyFilesDir();
}

void History::save(const QString &user, const QString &userId, const QDateTime &date, const QList<QString> &peersList, QList<SingleMessage> &messages, bool isBroadcast) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("History.save started"));

    if (!messages.size()) {
        LoggerManager::getInstance().writeWarning(QString("History.save -|- user: %1, userId: %2, date: %3 - no message to save").arg(user, userId, date.toString("yyyy.MM.dd hh:mm:ss")));
        return;
    }

    QString path = historyFile(date.date());
    QString users;
    QStringList usersList;
    QStringList userIds;
    QString ConversationUserId = userId;
    int peersInConversation = 0;

    if (!userId.compare(QStringLiteral("-2"))) {
        for (const SingleMessage &message : messages)
            if (message.userId != userId) {
                int userIdIndex = userIds.indexOf(message.userId);

                if (userIdIndex >= 0) {
                    usersList[userIdIndex] = message.userName;
                } else {
                    usersList.append(message.userName);
                    userIds.append(message.userId);
                }
                ++peersInConversation;
            }
        users = usersList.join(QStringList(", "));
    }
    else
        users = user;

    if (users.isEmpty()) {
        if (peersList.isEmpty())
            users = user;
        else {
            for (const QString &peerName : peersList)
                users.append(QString("%1, ").arg(peerName));
            users.chop(2);

            peersInConversation = peersList.size();
        }
    }

    QDir dir = QFileInfo(path).dir();
    if(!dir.exists())
        dir.mkpath(dir.absolutePath());

    QDomDocument document;

    bool writeRootTag = true;
    QString xml;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        xml = file.readAll().trimmed();
        writeRootTag = !xml.startsWith(QStringLiteral("<root>"));
        file.setPermissions(QFile::WriteOwner); // make it writable

        file.close();
    }

    if (writeRootTag)
        xml.prepend(QStringLiteral("<root>\n"));
    if (xml.endsWith(QStringLiteral("</root>")))
        xml.remove((xml.size() - 7), 7);

    QDomElement conversation = document.createElement(QStringLiteral("conversation"));
    conversation.setAttribute(QStringLiteral("tstamp"), QString::number(date.toMSecsSinceEpoch()));
    conversation.setAttribute(QStringLiteral("user"), users);
    conversation.setAttribute(QStringLiteral("userID"), ConversationUserId);
    conversation.setAttribute(QStringLiteral("broadcast"), isBroadcast ? "true" : "false");

    for (SingleMessage &message : messages) {
        QString msg;
        if (message.message.dataExists(XN_MESSAGE))
            msg = message.message.data(XN_MESSAGE);
        else if (message.message.dataExists(XN_BROADCAST))
            msg = message.message.data(XN_BROADCAST);
        else if (message.message.dataExists(XN_FILENAME))
            msg = message.message.data(XN_FILENAME);

        if (isBroadcast)
            msg.prepend("Broadcast: ");

        QDomElement messageElement = document.createElement(QStringLiteral("message"));
        messageElement.setAttribute(QStringLiteral("user"), message.userName);
        messageElement.appendChild(document.createTextNode(msg.toHtmlEscaped()));
        conversation.appendChild(messageElement);
    }

    QDomElement htmlMessage = document.createElement(QStringLiteral("htmlMessage"));
    htmlMessage.appendChild(document.createTextNode(getHtmlFromMessages(messages)));
    conversation.appendChild(htmlMessage);

    document.appendChild(conversation);

    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream writer(&file);
        xml.append(QString("%1</root>").arg(document.toString()));
        writer << xml;

        file.setPermissions(QFile::ReadOwner|QFile::ExeOwner|QFile::ReadGroup|QFile::ExeGroup|QFile::ReadOther|QFile::ExeOther); // make it readonly
        file.close();
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.save ended"));
}

QString History::getUserMessageHistory(const QString &userId, const QDate &date) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getUserMessageHistory started"));

    QString history;

    QDir historyDir = historyFilesDir();
    historyDir.setNameFilters(QStringList() << "*.xml");
    historyDir.setFilter(QDir::Files);

    for (const QString &file : historyDir.entryList()) {
        if (date <= QDate::fromString(file.mid(13, (file.length() - 17)), Qt::ISODate)
                and validateXml(historyDir.absoluteFilePath(file))) // the date is characters 13 -> length - 4
            history.append(getMessage(historyDir.absoluteFilePath(file), userId, true));
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getUserMessageHistory ended"));
    return history;
}

QList<MsgInfo> History::getList() {
    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getList started"));

    QList<MsgInfo> list;

    QDir historyDir = historyFilesDir();
    historyDir.setNameFilters(QStringList() << "*.xml");
    historyDir.setFilter(QDir::Files);

    for (const QString &file : historyDir.entryList())
        if (validateXml(historyDir.absoluteFilePath(file)))
            list.append(getConversationInfo(historyDir.absoluteFilePath(file)));

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getList ended"));
    return list;
}

QList<MsgInfo> History::getConversationInfo(const QString &fileName) {
    LoggerManager::getInstance().writeInfo(QString("History.getConversationInfo started -|- file: %1").arg(fileName));

    QList<MsgInfo> msgInfo;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return msgInfo;

    QXmlStreamReader reader(&file);

    while(!reader.isEndDocument()) {
        if (reader.isStartElement()) {
            if (reader.name() == "conversation") {
                QXmlStreamAttributes attributes = reader.attributes();
                QString name = attributes.value("user").toString();
                QString id = attributes.value("userID").toString();
                qint64 tstamp = attributes.value("tstamp").toLongLong();
                QDateTime date = QDateTime::fromMSecsSinceEpoch(tstamp);
                msgInfo.append(MsgInfo(name, id, date, tstamp, fileName));
            }
        }
        reader.readNext();
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getConversationInfo ended"));
    return msgInfo;
}

///
/// \brief History::getMessage
/// \param fileName
/// \param date - is the return from QDateTime::toMSecsSinceEpoch(). Garanteed to be unique. If useID is true, this param is the UserID
/// \return The html of the conversation
///
QString History::getMessage(const QString &fileName, const QString &criteria, bool useID) {
    LoggerManager::getInstance().writeInfo(QString("History.getMessage started -|- file: %1, criteria: %2, use id: %3").arg(fileName, criteria, useID ? "true" : "false"));

    QString htmlData;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString::null;

    QXmlStreamReader reader(&file);

    while(!reader.isEndDocument()) {
        if (reader.isStartElement()) {
            if (reader.name() == "conversation")
                if ((!useID and reader.attributes().value("tstamp") == criteria)
                        or (useID and reader.attributes().value("userID") == criteria
                            and reader.attributes().value("broadcast") == "false")) {
                    while (reader.name() != "htmlMessage" and !reader.isEndDocument())
                        reader.readNext();

                    if (!reader.isEndDocument()) {
                        htmlData.append(reader.readElementText(QXmlStreamReader::IncludeChildElements));

                        if (!useID) {
                            file.close();
                            return htmlData;
                        }
                    }
                }
        }
        reader.readNext();
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getMessage ended"));

    file.close();
    return htmlData;
}

bool History::validateXml(const QString &fileName) {
    QString schemaPath = QFileInfo(fileName).absolutePath();
    if (!schemaPath.endsWith('/'))
        schemaPath.append('/');
    schemaPath.append(QStringLiteral("XML schema.xsd"));

    QXmlSchema schema;
    schema.load(QUrl::fromLocalFile(schemaPath));

    if (schema.isValid()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::information(0, 0, QString("The history file \"%1\" could not be opened.").arg(fileName));
            return false;
        }
        QXmlSchemaValidator validator(schema);
        if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName()))) {
            QMessageBox::information(0, 0, QString("The history file \"%1\" is not a valid XML file.").arg(fileName));
            return false;
        }
    }

    return true;
}

QString History::getHtmlFromMessages(const QList<SingleMessage> messageLog) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getHtmlFromMessages started"));

    QDateTime time;

    QString html =
        "<style type='text/css'>"\
        "div.body_history {font-size: 9pt; -webkit-nbsp-mode: space; word-wrap: break-word; margin-left: 5px; margin-right: 5px;}"\
        "span.salutation_history {font-size: 9pt; float:left; font-weight: bold;} span.time_history {font-size: 9pt; float: right;}"\
        "span.message_history {font-size: 9pt; clear: both; display: block;} p.p_history {font-size: 9pt; border-bottom: 1px solid #CCC;}"\
        "</style><div class='body_history'>";

    for(int index = 0; index < messageLog.count(); index++) {
        SingleMessage msg = messageLog.at(index);
        if(msg.type == MT_Message || msg.type == MT_GroupMessage || msg.type == MT_PublicMessage || msg.type == MT_Broadcast) {
            time.setMSecsSinceEpoch(msg.message.header(XN_TIME).toLongLong());
            QString messageText;
            if (msg.type == MT_Broadcast)
                messageText = msg.message.data(XN_BROADCAST);
            else
                messageText = msg.message.data(XN_MESSAGE);

            decodeMessage(messageText);
            QString htmlMsg =
                "<p class='p_history'><span class='salutation_history'>" + msg.userName + ":</span>"\
                "<span class='time_history'>" + time.time().toString(Qt::SystemLocaleShortDate) + "</span>"\
                "<span class='message_history'>" + messageText + "</span></p>";
            html.append(htmlMsg);
        }
    }

    html.append("</div>");

    LoggerManager::getInstance().writeInfo(QStringLiteral("History.getHtmlFromMessages ended"));
    return html;
}


void History::decodeMessage(QString &html) {
     html = html.trimmed();

     html.replace(QRegExp("((?:(?:https?|ftp|file)://|www\\.|ftp\\.)[-A-Z0-9+&@#/%=~_|$?!:,.]*[A-Z0-9+&@#/%=~_|$])", Qt::CaseInsensitive),
                          "<a data-isLink='true' href='\\1'>\\1</a>");
     html.replace("<a data-isLink='true' href='www", "<a data-isLink='true' href='http://www");

     html.replace(QRegExp("((\\\\\\\\[\\w-]+\\\\[^\\\\/:*?<>|""]+)((?:\\\\[^\\\\/:*?<>|""]+)*\\\\?)$)"),
                          "<a data-isLink='true' href='file:\\1'>\\1</a>");

     lmcSettings settings;
     QString erpAddress = settings.value(IDS_ERPADDRESS, IDS_ERPADDRESS_VAL).toString();
     if (!erpAddress.endsWith('/'))
         erpAddress.append('/');

     // replace ITx, COMMx, ATTx and VIDx with links
     html.replace(QRegExp("\\b(IT(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?IssueId=\\2' title='%1Pages/Issues/IssueDetail.aspx?IssueId=\\2'>\\1</a>").arg(erpAddress));
     html.replace(QRegExp("\\b(COMM(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?CommentId=\\2#COMMN\\2' title='%1erp/Pages/Issues/IssueDetail.aspx?CommentId=\\2#COMMN\\2'>\\1</a>").arg(erpAddress));
     html.replace(QRegExp("\\b(ATT(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?VideoId=\\2' title='%1erp/Pages/Issues/IssueDetail.aspx?VideoId=\\2'>\\1</a>").arg(erpAddress));
     html.replace(QRegExp("\\b(VID(\\d+))\\b", Qt::CaseInsensitive), QString("<a data-isLink='true' href='%1Pages/Issues/IssueDetail.aspx?AttachmentId=\\2' title='%1Pages/Issues/IssueDetail.aspx?AttachmentId=\\2'>\\1</a>").arg(erpAddress));

    QString message;
    int index = 0;

    while(index < html.length()) {
        int aStart = html.indexOf("<a data-isLink='true'", index);
        if(aStart != -1) {
            QString messageSegment = html.mid(index, aStart - index);
            processMessageText(messageSegment);
            message.append(messageSegment);
            index = html.indexOf("</a>", aStart) + 4;
            QString linkSegment = html.mid(aStart, index - aStart);
            message.append(linkSegment);
        } else {
            QString messageSegment = html.mid(index);
            processMessageText(messageSegment);
            message.append(messageSegment);
            break;
        }
    }

    message.replace("\n", "<br/>");
    message = message.replace("[b]", "<b>").replace("[/b]", "</b>").replace("[u]", "<u>").replace("[/u]", "</u>").replace("[i]", "<i>").replace("[/i]", "</i>").replace("[q]", "<q>").replace("[/q]", "</q>").replace("[quote]", "<blockquote>").replace("[/quote]", "</blockquote>");

    html = message;
}


void History::processMessageText(QString &message) {
    ChatHelper::makeHtmlSafe(message);

    ChatHelper::decodeSmileys(message, ImagesList::getInstance().getSmileysRegex(), false);
    ChatHelper::decodeSmileys(message, ImagesList::getInstance().getEmojisRegex(), true);

    message.replace("  ", "&nbsp;&nbsp;");
}
