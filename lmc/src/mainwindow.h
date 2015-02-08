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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <qevent.h>
#include <QTreeWidget>
#include <QWidgetAction>
#include <QInputDialog>
#include <QFileDialog>
#include <QClipboard>
#include "ui_mainwindow.h"
#include "shared.h"
#include "settings.h"
#include "imagepickeraction.h"
#include "soundplayer.h"
#include "stdlocation.h"
#include "xmlmessage.h"
#include "themedbutton.h"

class lmcMainWindow : public QWidget {
  Q_OBJECT

public:
  lmcMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~lmcMainWindow();

  void init(User *pLocalUser, QList<Group> *pGroupList, bool connected);
  void start();
  void show();
  void restore();
  void minimize();
  void stop();
  void addUser(User *pUser);
  void updateUser(User *pUser);
  void removeUser(QString *lpszUserId);
  void receiveMessage(MessageType type, QString *lpszUserId,
                      XmlMessage *pMessage);
  void connectionStateChanged(bool connected);
  void settingsChanged(bool init = false);
  void showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId = QString(),
                       QString szTitle = QString::null,
                       TrayMessageIcon icon = TMI_Info);
  QList<QTreeWidgetItem *> getContactsList();
  QList<QTreeWidgetItem *> getSelectedUserTreeItems(QTreeWidgetItem *dropTarget = nullptr);
  QStringList getSelectedUserIds();

signals:
  void appExiting();
  void messageSent(MessageType type, QString *lpszUserId, XmlMessage *pMessage);
  void chatStarting(QString *lpszUserId);
  void chatRoomStarting(QString lpszThreadId, XmlMessage message, QStringList contacts);
  void showTransfers(QString userId);
  void showMessage(QString chatRoomId);
  void showHistory(QString userId);
  void showSettings();
  void showHelp(QRect *pRect);
  void showUpdate(QRect *pRect);
  void showAbout();
  void showBroadcast();
  void showPublicChat();
  void groupUpdated(GroupOp op, QVariant value1, QVariant value2);

protected:
  bool eventFilter(QObject *pObject, QEvent *pEvent);
  void closeEvent(QCloseEvent *pEvent);
  void changeEvent(QEvent *pEvent);

private slots:
  void sendMessage(MessageType type, QString *lpszUserId, XmlMessage *pMessage);
  void trayShowAction_triggered();
  void trayHistoryAction_triggered();
  void trayFileAction_triggered();
  void traySettingsAction_triggered();
  void trayAboutAction_triggered();
  void trayExitAction_triggered();
  void statusAction_triggered(QAction *action);
  void avatarAction_triggered();
  void avatarBrowseAction_triggered();
  void helpAction_triggered();
  void homePageAction_triggered();
  void updateAction_triggered();
  void buttonStartChat_clicked();
  void buttonStartPublicChat_clicked();
  void refreshAction_triggered();
  void trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
  void trayMessage_clicked();
  void treeWidgetUserList_itemActivated(QTreeWidgetItem *pItem, int column);
  void treeWidgetUserList_itemContextMenu(QTreeWidgetItem *pItem, QPoint &pos);
  void treeWidgetUserList_itemDragDropped(QTreeWidgetItem *item);
  void treeWidgetUserList_fileDragDropped(QTreeWidgetItem *item, QStringList fileNames);
  void treeWidgetUserList_itemSelectionChanged();
  void groupAddAction_triggered();
  void groupRenameAction_triggered();
  void groupDeleteAction_triggered();
  void userConversationAction_triggered();
  void buttonSendBroadcast_clicked();
  void buttonSendFile_clicked();
  void userFolderAction_triggered();
  void userhistoryAction_triggered();
  void usertransferAction_triggered();
  void userInfoAction_triggered();
  void userSendScreenshotAction_triggered();
  void userSendFileClipboard_triggered();
  void textBoxNote_returnPressed();
  void actionSendBroadcast_triggered();
  void textBoxNote_lostFocus();

private:
  void createMainMenu();
  void createTrayMenu();
  void createTrayIcon();
  void changeTrayIcon(QString icon);
  void createStatusMenu();
  void createAvatarMenu();
  void createGroupMenu();
  void createUserMenu();
  void createUsersListMainMenu();
  void createToolBar();
  void setUIText();
  void showMinimizeMessage();
  void initGroups(QList<Group> *pGroupList);
  void updateStatusImage(QTreeWidgetItem *pItem, QString *lpszStatus);
  void setAvatar(QString fileName = QString());
  QTreeWidgetItem *getUserItem(QString *lpszUserId);
  QTreeWidgetItem *getGroupItem(QString *lpszGroupId);
  QTreeWidgetItem *getGroupItemByName(QString *lpszGroupName);
  void sendMessage(MessageType type, QString *lpszUserId, QString *lpszMessage, QString chatRoomId = "");
  void sendAvatar(QString *lpszUserId);
  void setUserAvatar(QString *lpszUserId, QString *lpszFilePath);
  void processTrayIconTrigger();
  void setTrayTooltip();
  void sendFile(bool sendFolder = false, QTreeWidgetItem *dropTarget = nullptr, QStringList files = QStringList());
  void startChatRoom(bool fromToolbar = false);

  Ui::MainWindow ui;

  const QClipboard *_clipboard = QApplication::clipboard();

  lmcSettings *pSettings;
  QMenuBar *pMainMenu;
  QSystemTrayIcon *pTrayIcon;
  QMenu *pFileMenu;
  QMenu *pToolsMenu;
  QMenu *pTrayMenu;
  QMenu *pHelpMenu;
  QMenu *pStatusMenu;
  QMenu *pAvatarMenu = nullptr;
  QMenu *pGroupMenu;
  QMenu *pUserMenu;
  QMenu *_usersListMainMenu;
  ThemedButton *btnStatus;
  ThemedButton *buttonStartChat;
  ThemedButton *buttonSendFile;
  ThemedButton *buttonSendBroadcast;
  ThemedButton *buttonStartPublicChat;
  User *pLocalUser;

  bool bConnected;
  int nAvatar;
  bool showSysTray;
  bool showSysTrayMsg;
  bool showMinimizeMsg;
  bool minimizeHide;
  bool singleClickActivation;
  bool allowSysTrayMinimize;
  bool showAlert;
  bool noBusyAlert;
  bool noDNDAlert;

  lmcSoundPlayer *pSoundPlayer;
  TrayMessageType lastTrayMessageType;
  QString lastTrayMessageChatRoomId;
  QActionGroup *statusGroup;
  QAction *chatRoomAction;
  QAction *publicChatAction;
  QAction *refreshAction;
  QAction *exitAction;
  QAction *historyAction;
  QAction *transferAction;
  QAction *settingsAction;
  QAction *helpAction;
  QAction *onlineAction;
  QAction *updateAction;
  QAction *aboutAction;
  QAction *trayShowAction;
  QAction *trayStatusAction;
  QAction *trayHistoryAction;
  QAction *trayTransferAction;
  QAction *traySettingsAction;
  QAction *trayAboutAction;
  QAction *trayExitAction;
  QAction *groupChatAction;
  QAction *groupFileAction;
  QAction *groupFolderAction;
  QAction *groupBroadcastAction;
  QAction *groupAddAction;
  QAction *groupRenameAction;
  QAction *groupDeleteAction;
  QAction *_groupSendScreenshotAction;
  QAction *_groupSendFileClipboardAction;
  QAction *_userChatAction;
  QAction *_userBroadcastAction;
  QAction *_userFileAction;
  QAction *_userFolderAction;
  QAction *_userhistoryAction;
  QAction *_usertransferAction;
  QAction *_userInfoAction;
  QAction *_userSendScreenshotAction;
  QAction *_userSendFileClipboardAction;
  QAction *_usersMainAddAction;
  QAction *_avatarBrowseAction;

  bool windowLoaded;
};

#endif // MAINWINDOW_H
