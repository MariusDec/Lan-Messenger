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

#include "webnetwork.h"
#include <QDebug>

const QString ErrorTypeNames[] = {"busy", "error"};

lmcWebNetwork::lmcWebNetwork() {
  active = false;
  manager = new QNetworkAccessManager(this);
  connect(manager, &QNetworkAccessManager::finished, this,
          &lmcWebNetwork::replyFinished);
}

lmcWebNetwork::~lmcWebNetwork() {}

void lmcWebNetwork::init() {}

void lmcWebNetwork::start() {}

void lmcWebNetwork::stop() {}

void lmcWebNetwork::sendMessage(QString *lpszUrl, QString *lpszData) {
  Q_UNUSED(lpszData);

  if (!active)
    sendMessage(QUrl(*lpszUrl));
  else
    raiseError(ET_Busy);
}

void lmcWebNetwork::settingsChanged() {}

void lmcWebNetwork::slotError(QNetworkReply::NetworkError code) {
  if (code != QNetworkReply::NoError) {
    raiseError(ET_Error);
    active = false;
  }
}

void lmcWebNetwork::replyFinished(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError)
    return;

  // check if there was an HTTP redirection
  QVariant redirect =
      reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (!redirect.isNull()) {
    // send a new request to the redirected url
    sendMessage(redirect.toUrl());
  } else {
    // no redirection, get the data from the reply
    QByteArray data = reply->readAll();
    QString szMessage = QString(data.constData());
    emit messageReceived(&szMessage);
    reply->close();
  }

  reply->deleteLater();
  active = false;
}

void lmcWebNetwork::sendMessage(const QUrl &url) {
  if (url.isEmpty()) {
    raiseError(ET_Error);
    return;
  }

  active = true;

  QNetworkReply *reply = manager->get(QNetworkRequest(url));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
          SLOT(slotError(QNetworkReply::NetworkError)));
}

void lmcWebNetwork::raiseError(ErrorType type) {
  XmlMessage xmlMessage;
  xmlMessage.addHeader(XN_TYPE, MessageTypeNames[MT_WebFailed]);
  xmlMessage.addData(XN_ERROR, ErrorTypeNames[type]);

  QString szMessage = xmlMessage.toString();
  emit messageReceived(&szMessage);
}
