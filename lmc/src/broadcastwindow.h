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


#ifndef BROADCASTWINDOW_H
#define BROADCASTWINDOW_H

#include <QWidget>
#include <QToolBar>
#include <QMenu>
#include <qevent.h>
#include "ui_broadcastwindow.h"
#include "shared.h"
#include "settings.h"
#include "imagepickeraction.h"
#include "subcontrols.h"
#include "chatdefinitions.h"
#include "chathelper.h"
#include "xmlmessage.h"

class lmcBroadcastWindow : public QWidget
{
    Q_OBJECT

public:
    lmcBroadcastWindow(User *localUser, QWidget *parent = 0);
    ~lmcBroadcastWindow();

    void init(bool connected);
    void stop();
    void show(const QList<QTreeWidgetItem *> &pGroupList = QList<QTreeWidgetItem*>(), const QStringList &selectedUsers = QStringList());
    void connectionStateChanged(bool connected);
    void settingsChanged();

signals:
    void messageSent(MessageType type, QString* lpszUserId, XmlMessage* pMessage);

protected:
    bool eventFilter(QObject* pObject, QEvent* pEvent);
    void changeEvent(QEvent* pEvent);
    void closeEvent(QCloseEvent* pEvent);

private slots:
    void smileyAction_triggered();
    void emojiAction_triggered();
    void buttonSelectAll_clicked();
    void buttonSelectNone_clicked();
    void treeWidgetUserList_selectedItemsChanged(unsigned count);
    void buttonSend_clicked();

private:
    void createToolBar();
    void setUIText();
    void sendMessage();
    void showStatus(int flag, bool add);
    void insertSmileyCode(const ImagesStruct &smiley);

    Ui::BroadcastWindow ui;
    lmcSettings* pSettings;
    QToolBar* _toolbar;
    //ThemedButton* _buttonFontSize;
    ThemedButton* _buttonSmiley;
    ThemedButton* _buttonEmoji;
    int nSmiley;
    int nEmoji;
    bool bConnected;
    int infoFlag;
    bool showSmiley;
    bool sendKeyMod;
    bool parentToggling;
    bool childToggling;
    //QActionGroup* _groupActionFont;
    User *localUser;
};

#endif // BROADCASTWINDOW_H
