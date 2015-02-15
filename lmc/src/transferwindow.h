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


#ifndef TRANSFERWINDOW_H
#define TRANSFERWINDOW_H

#include <QtWidgets/QWidget>
#include <QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include <QDir>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <qevent.h>
#include "ui_transferwindow.h"
#include "shared.h"
#include "settings.h"
#include "stdlocation.h"
#include "soundplayer.h"
#include "xmlmessage.h"
#include "filemodelview.h"

class lmcTransferWindow : public QWidget
{
    Q_OBJECT

public:
    lmcTransferWindow(QWidget *parent = 0);
    ~lmcTransferWindow();

    void init();
    void updateList();
    void stop();
    void createTransfer(MessageType type, FileMode mode, QString* lpszUserId, QString* lpszUserName, XmlMessage* pMessage);
    void receiveMessage(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void settingsChanged();
    void setUserFilter(const QString &userName);

signals:
    void messageSent(MessageType type, QString* lpszUserId, XmlMessage* pMessage);
    void showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId, QString szTitle, TrayMessageIcon icon);

protected:
    bool eventFilter(QObject* pObject, QEvent* pEvent);
    void changeEvent(QEvent* pEvent);

private slots:
    void listWidgetTransferList_currentRowChanged(int currentRow);
    void listWidgetTransferList_activated(const QModelIndex& index);
    void buttonCancel_clicked();
    void buttonRemove_clicked();
    void buttonClear_clicked();
    void buttonClose_clicked();
    void comboBoxSelectedUser_textChanged(const QString &);
    void buttonShowFolder_clicked();
    void updateProgress(FileView* view, qint64 currentPos);

private:
    void setUIText();
    void createToolBar();
    QFrame *createSeparator(QWidget *parent);
    void setButtonState(FileView::TransferState state);
    QPixmap getIcon(QString filePath);
    QString formatTime(qint64 size, qint64 speed);
    void clearList();
    QList<QString> getUniqueUsers();
    void saveHistory(const FileView *transfer = nullptr);

    Ui::TransferWindow ui;
    lmcSettings* pSettings;
    lmcSoundPlayer* pSoundPlayer;
    ThemedButton *_buttonCancel;
    ThemedButton *_buttonShowFolder;
    ThemedButton *_buttonRemove;
    QList<FileView> pendingSendList;
};

#endif // TRANSFERWINDOW_H
