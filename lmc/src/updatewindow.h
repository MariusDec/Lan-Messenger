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


#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QWidget>
#include <qevent.h>
#include "xmlmessage.h"

namespace Ui {
    class UpdateWindow;
}

class lmcUpdateWindow : public QWidget
{
    Q_OBJECT

public:
    explicit lmcUpdateWindow(const QRect &pRect, QWidget *parent = 0);
    ~lmcUpdateWindow();

    void init();
    void stop();
    void receiveMessage(MessageType type, const XmlMessage &message);
    void settingsChanged();

signals:
    void messageSent(MessageType type, QString lpszUserId, XmlMessage pMessage);

protected:
    bool eventFilter(QObject* pObject, QEvent* pEvent);
    void changeEvent(QEvent* pEvent);

private slots:
    void btnRecheck_clicked();

private:
    void setUIText();
    void checkForUpdates();
    QString getStatusMessage();

    enum UpdateStatus{US_Check, US_Error, US_New, US_Latest};

    Ui::UpdateWindow *ui;
    QString webVersion;
    UpdateStatus status;
};

#endif // UPDATEWINDOW_H
