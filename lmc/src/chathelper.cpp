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


#include <QFile>
#include <QUrl>
#include "chathelper.h"
#include "imageslist.h"

QDataStream &operator << (QDataStream &out, const SingleMessage &message) {
    out << qint32(message.type) << message.userId << message.userName << message.message.toString()
        << message.id;
    return out;
}

QDataStream &operator >> (QDataStream &in, SingleMessage &message) {
    qint32 type;
    QString userId;
    QString userName;
    QString xmlMessage;
    QString id;
    in >> type >> userId >> userName >> xmlMessage >> id;
    message = SingleMessage((MessageType)type, userId, userName, XmlMessage(xmlMessage), id);
    return in;
}

void ChatHelper::makeHtmlSafe(QString* lpszMessage) {
    for(int index = 0; index < HTMLESC_COUNT; index++)
        lpszMessage->replace(htmlSymbol[index], htmlEscape[index]);
}

QString ChatHelper::makeHtmlSafe(const QString &message)
{
    QString escaped = message;

    for(int index = 0; index < HTMLESC_COUNT; index++)
        escaped.replace(htmlSymbol[index], htmlEscape[index]);

    return escaped;
}

QString ChatHelper::replaceSmiley(QString* lpszHtml) {
    auto smileys = ImagesList::getInstance ().getSmileys ();
    for(unsigned index = 0; index < smileys.size (); index++) {
        if(lpszHtml->compare(QString("<img src='%1' alt='%2' title='<b>%3</b><br />%2' />").arg (smileys[index].icon, smileys[index].code, smileys[index].description)) == 0) {
            QString code = smileys[index].code;
            makeHtmlSafe(&code);
            return code;
        }
    }

    return QString::null;
}

void ChatHelper::decodeSmileys(QString &message, const QRegularExpression &regex, bool isEmoji) {
    //	replace text emoticons with corresponding images
    QRegularExpressionMatchIterator iterator = regex.globalMatch(message);

    // I'm saving the matches to make the string.replace starting from the last match, so that I don't destroy the start index
    std::vector<std::pair<int, QString>> matches;
    if (iterator.isValid()) {
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            matches.push_back(std::make_pair(match.capturedStart(), match.captured()));
        }

        bool found;
        const ImagesStruct * smiley = nullptr;
        for (int index = matches.size() - 1; index >= 0; --index) {
            if (isEmoji) {
                smiley = ImagesList::getInstance().getEmojiByCode(matches[index].second, found);
                if (found)
                    message.replace(matches[index].first, matches[index].second.size(), QString("<span title=\"%1\" >%2</span>").arg (makeHtmlSafe(smiley->description), makeHtmlSafe(smiley->code)));
            } else {
                smiley = ImagesList::getInstance().getSmileyByCode(matches[index].second, found);
                if (found) {
                    message.replace(matches[index].first, matches[index].second.size(), QString("<img src='%1' alt=\"%2\" title=\"%3: %2\" />").arg (QUrl::fromLocalFile(smiley->icon).toString(), makeHtmlSafe(smiley->code), makeHtmlSafe(smiley->description)));
                }
            }
        }
    }
}
