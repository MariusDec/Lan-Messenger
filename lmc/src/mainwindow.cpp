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

#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QMimeData>

#include "mainwindow.h"
#include "messagelog.h"
#include "history.h"
#include "loggermanager.h"
#include "imageslist.h"
#include "globals.h"

lmcMainWindow::lmcMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags) {
  ui.setupUi(this);
  setProperty("isWindow", true);
  ui.textBoxNote->setProperty("square", true);
  ui.buttonAvatar->setProperty("bigButton", true);

  ui.buttonAvatar->setIconFitSize(true);

  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemActivated,
          this, &lmcMainWindow::treeWidgetUserList_itemActivated);
  connect(
      ui.treeWidgetUserList,
      &lmcUserTreeWidget::itemContextMenu, this,
      &lmcMainWindow::treeWidgetUserList_itemContextMenu);
  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemDragDropped,
          this, &lmcMainWindow::treeWidgetUserList_itemDragDropped);
  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::fileDragDropped,
          this, &lmcMainWindow::treeWidgetUserList_fileDragDropped);
  connect(ui.treeWidgetUserList,
          &lmcUserTreeWidget::itemSelectionChanged,
          this, &lmcMainWindow::treeWidgetUserList_itemSelectionChanged);
  connect(ui.textBoxNote, SIGNAL(returnPressed()), this,
          SLOT(textBoxNote_returnPressed()));
  connect(ui.textBoxNote, SIGNAL(lostFocus()), this,
          SLOT(textBoxNote_lostFocus()));

  ui.textBoxNote->installEventFilter(this);
  ui.treeWidgetUserList->installEventFilter(this);

  windowLoaded = false;
}

lmcMainWindow::~lmcMainWindow() {}

void lmcMainWindow::init(User *pLocalUser, QList<Group> *pGroupList,
                         bool connected) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.init started"));

  setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

  this->pLocalUser = pLocalUser;

  createMainMenu();
  createToolBar();
  createStatusMenu();
  createAvatarMenu();

  createTrayMenu();
  createTrayIcon();
  connectionStateChanged(connected);

  createGroupMenu();
  createUserMenu();
  createUsersListMainMenu();

  ui.labelDividerTop->setBackgroundRole(QPalette::Highlight);
  ui.labelDividerTop->setAutoFillBackground(true);

  ui.treeWidgetUserList->setIconSize(QSize(16, 16));
  ui.treeWidgetUserList->header()->setDragEnabled(false);
  ui.treeWidgetUserList->header()->setStretchLastSection(false);
  ui.treeWidgetUserList->header()->setSectionResizeMode(0,
                                                        QHeaderView::Stretch);
  btnStatus->setIconSize(QSize(20, 20));

  // get current status struct
  StatusStruct *currentStatus = nullptr;
  int statusIndex;

  pSettings = new lmcSettings();
  if (pSettings->value(IDS_RESTORESTATUS, IDS_RESTORESTATUS_VAL).toBool()) {
      auto statuses = Globals::getInstance().getStatuses();
      for (unsigned index = 0; index < statuses.size(); ++index)
          if (!statuses[index].description.compare(pLocalUser->status)) {
              currentStatus = &statuses[index];
              statusIndex = index;
              break;
          }
  }

  if (!currentStatus) {
      // set available status
      currentStatus = &Globals::getInstance().getStatuses().front();
      statusIndex = 0;
  }

  btnStatus->setIcon(QIcon(QPixmap(
      ThemeManager::getInstance().getAppIcon(currentStatus->icon))));
  statusGroup->actions()[statusIndex]->setChecked(true);
  QFont font = ui.labelUserName->font();
  int fontSize = ui.labelUserName->fontInfo().pixelSize();
  fontSize += (fontSize * 0.1);
  font.setPixelSize(fontSize);
  font.setBold(true);
  ui.labelUserName->setFont(font);
  ui.labelStatus->setText(statusGroup->checkedAction()->text());
  nAvatar = pLocalUser->avatar;
  ui.textBoxNote->setText(pLocalUser->note);

  pSoundPlayer = new lmcSoundPlayer();
  restoreGeometry(pSettings->value(IDS_WINDOWMAIN).toByteArray());
  //	get saved settings
  settingsChanged(true);
  setUIText();

  initGroups(pGroupList);

  refreshAction_triggered();

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.init ended"));
}

void lmcMainWindow::start() {
  int avatarsCount = ImagesList::getInstance().getAvatars().size();
  //	if no avatar is set, select a random avatar (useful when running for the
  // first time)
  if (nAvatar > avatarsCount) {
    qsrand((uint)QTime::currentTime().msec());
    nAvatar = qrand() % avatarsCount;
  }
  // This method should only be called from here, otherwise an MT_Notify message
  // is sent
  // and the program will connect to the network before start() is called.
  setAvatar();
  pTrayIcon->setVisible(showSysTray);
  if (pSettings->value(IDS_AUTOSHOW, IDS_AUTOSHOW_DEFAULT_VAL).toBool())
    show();
}

void lmcMainWindow::show() {
  windowLoaded = true;
  QWidget::show();
}

void lmcMainWindow::restore() {
  //	if window is minimized, restore it to previous state
  if (windowState().testFlag(Qt::WindowMinimized))
    setWindowState(windowState() & ~Qt::WindowMinimized);
  setWindowState(windowState() | Qt::WindowActive);
  raise(); // make main window the top most window of the application
  show();
  activateWindow(); // bring window to foreground
}

void lmcMainWindow::minimize() {
  // This function actually hides the window, basically the opposite of
  // restore()
  hide();
  showMinimizeMessage();
}

void lmcMainWindow::stop() {
  //	These settings are saved only if the window was opened at least once by
  // the user
  if (windowLoaded) {
    pSettings->setValue(IDS_WINDOWMAIN, saveGeometry());
    pSettings->setValue(IDS_MINIMIZEMSG, showMinimizeMsg);
  }

  pSettings->beginWriteArray(IDS_GROUPEXPHDR);
  for (int index = 0; index < ui.treeWidgetUserList->topLevelItemCount();
       index++) {
    pSettings->setArrayIndex(index);
    pSettings->setValue(
        IDS_GROUP, ui.treeWidgetUserList->topLevelItem(index)->isExpanded());
  }
  pSettings->endArray();

  pTrayIcon->hide();
}

void lmcMainWindow::addUser(User *pUser) {
  if (!pUser)
    return;

  // get current status struct
  int index;
  StatusStruct *currentStatus =
      Globals::getInstance().getStatus(pUser->status, &index);

  lmcUserTreeWidgetUserItem *pItem = new lmcUserTreeWidgetUserItem();
  pItem->setData(0, IdRole, pUser->id);
  pItem->setData(0, TypeRole, "User");
  pItem->setData(0, StatusRole, index);
  pItem->setData(0, SubtextRole, pUser->note);
  pItem->setData(0, CapsRole, pUser->caps);
  pItem->setData(0, DataRole, pUser->lanIndex);
  pItem->setText(0, pUser->name);

  QString userTooltip = QString("<b>%1</b><br />Name: %2<br />IP: %3<br />Computer: %4<br />Version: %5").arg(currentStatus->uiDescription, pUser->name, pUser->address, pUser->hostName, pUser->version);
  pItem->setToolTip(0, userTooltip);

  if (currentStatus)
    pItem->setIcon(
        0, QIcon(ThemeManager::getInstance().getAppIcon(
               currentStatus->icon)));

  lmcUserTreeWidgetGroupItem *pGroupItem =
      (lmcUserTreeWidgetGroupItem *)getGroupItem(&pUser->group);
  pGroupItem->addChild(pItem);
  pGroupItem->sortChildren(0, Qt::AscendingOrder);

  // this should be called after item has been added to tree
  setUserAvatar(&pUser->id, &pUser->avatarPath);

  if (isHidden() || !isActiveWindow()) {
    QString msg = tr("%1 is online.");
    showTrayMessage(TM_Status, msg.arg(pItem->text(0)));
    pSoundPlayer->play(SE_UserOnline);
  }

  sendAvatar(&pUser->id);
  sendMessage(MT_Status, NULL, &pLocalUser->status);
}

void lmcMainWindow::updateUser(User *pUser) {
  if (!pUser)
    return;

  QTreeWidgetItem *pItem = getUserItem(&pUser->id);
  if (pItem) {
    updateStatusImage(pItem, &pUser->status);

    // get current status struct
    int index;
    StatusStruct *currentStatus =
        Globals::getInstance().getStatus(pUser->status, &index);

    pItem->setData(0, StatusRole, index);
    pItem->setData(0, SubtextRole, pUser->note);
    pItem->setText(0, pUser->name);

    QString userTooltip = QString("<b>%1</b><br />Name: %2<br />IP: %3<br />Computer: %4<br />Version: %5").arg(currentStatus->uiDescription, pUser->name, pUser->address, pUser->hostName, pUser->version);
    pItem->setToolTip(0, userTooltip);

    QTreeWidgetItem *pGroupItem = pItem->parent();
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
  }
}

void lmcMainWindow::removeUser(QString *lpszUserId) {
  QTreeWidgetItem *pItem = getUserItem(lpszUserId);
  if (!pItem)
    return;

  QTreeWidgetItem *pGroup = pItem->parent();
  pGroup->removeChild(pItem);

  if (isHidden() || !isActiveWindow()) {
    QString msg = tr("%1 is offline.");
    showTrayMessage(TM_Status, msg.arg(pItem->text(0)));
    pSoundPlayer->play(SE_UserOffline);
  }
}

void lmcMainWindow::receiveMessage(MessageType type, QString *lpszUserId,
                                   XmlMessage *pMessage) {
  QString filePath;

  switch (type) {
  case MT_Avatar:
    filePath = pMessage->data(XN_FILEPATH);
    setUserAvatar(lpszUserId, &filePath);
    break;
  default:
    break;
  }
}

void lmcMainWindow::connectionStateChanged(bool connected) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.connectionStateChanged started"));

  if (connected)
    showTrayMessage(TM_Connection, tr("You are online."));
  else
    showTrayMessage(TM_Connection, tr("You are no longer connected."), QString(),
                    lmcStrings::appName(), TMI_Warning);

  bConnected = connected;
  setTrayTooltip();

  LoggerManager::getInstance().writeInfo(
      QString("lmcMainWindow.connectionStateChanged ended. State: %1")
          .arg(connected ? "connected" : "disconnected"));
}

void lmcMainWindow::settingsChanged(bool init) {
  showSysTray = pSettings->value(IDS_SYSTRAY, IDS_SYSTRAY_VAL).toBool();
  showSysTrayMsg =
      pSettings->value(IDS_SYSTRAYMSG, IDS_SYSTRAYMSG_VAL).toBool();
  //	this setting should be loaded only at window init
  if (init)
    showMinimizeMsg =
        pSettings->value(IDS_MINIMIZEMSG, IDS_MINIMIZEMSG_VAL).toBool();
  //	this operation should not be done when window inits
  if (!init)
    pTrayIcon->setVisible(showSysTray);
  minimizeHide =
      pSettings->value(IDS_MINIMIZETRAY, IDS_MINIMIZETRAY_VAL).toBool();
  singleClickActivation =
      pSettings->value(IDS_SINGLECLICKTRAY, IDS_SINGLECLICKTRAY_VAL).toBool();
  allowSysTrayMinimize =
      pSettings->value(IDS_ALLOWSYSTRAYMIN, IDS_ALLOWSYSTRAYMIN_VAL).toBool();
  showAlert = pSettings->value(IDS_ALERT, IDS_ALERT_VAL).toBool();
  noBusyAlert = pSettings->value(IDS_NOBUSYALERT, IDS_NOBUSYALERT_VAL).toBool();
  noDNDAlert = pSettings->value(IDS_NODNDALERT, IDS_NODNDALERT_VAL).toBool();
  int viewType =
      pSettings->value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt();
  ui.treeWidgetUserList->setView((UserListView)viewType);

  pSoundPlayer->settingsChanged();
  ui.labelUserName->setText(
      pLocalUser->name); // in case display name has been changed
}

void lmcMainWindow::showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId,
                                    QString szTitle, TrayMessageIcon icon) {
  if (!showSysTray || !showSysTrayMsg)
    return;

  bool showMsg = showSysTray;

  switch (type) {
  case TM_Status:
    if (!showAlert || (pLocalUser->status == "Busy" && noBusyAlert) ||
        (pLocalUser->status == "NoDisturb" && noDNDAlert))
      return;
    break;
  default:
    break;
  }

  if (szTitle.isNull())
    szTitle = lmcStrings::appName();

  QSystemTrayIcon::MessageIcon trayIcon = QSystemTrayIcon::Information;
  switch (icon) {
  case TMI_Info:
    trayIcon = QSystemTrayIcon::Information;
    break;
  case TMI_Warning:
    trayIcon = QSystemTrayIcon::Warning;
    break;
  case TMI_Error:
    trayIcon = QSystemTrayIcon::Critical;
    break;
  default:
    break;
  }

  if (showMsg) {
    lastTrayMessageType = type;
    lastTrayMessageChatRoomId = chatRoomId;
    pTrayIcon->showMessage(szTitle, szMessage, trayIcon);
  }
}

QList<QTreeWidgetItem *> lmcMainWindow::getContactsList() {
  QList<QTreeWidgetItem *> contactsList;
  for (int index = 0; index < ui.treeWidgetUserList->topLevelItemCount();
       index++)
    contactsList.append(ui.treeWidgetUserList->topLevelItem(index)->clone());

  return contactsList;
}

bool lmcMainWindow::eventFilter(QObject *pObject, QEvent *pEvent) {
  Q_UNUSED(pObject);
  if (pEvent->type() == QEvent::KeyPress) {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);
    if (pKeyEvent->key() == Qt::Key_Escape) {
      close();
      return true;
    }
  }

  return false;
}

void lmcMainWindow::closeEvent(QCloseEvent *pEvent) {
  //	close main window to system tray
  pEvent->ignore();
  minimize();
}

void lmcMainWindow::changeEvent(QEvent *pEvent) {
  switch (pEvent->type()) {
  case QEvent::WindowStateChange:
    if (minimizeHide) {
      QWindowStateChangeEvent *e = (QWindowStateChangeEvent *)pEvent;
      if (isMinimized() && e->oldState() != Qt::WindowMinimized) {
        QTimer::singleShot(0, this, SLOT(hide()));
        pEvent->ignore();
        showMinimizeMessage();
      }
    }
    break;
  case QEvent::LanguageChange:
    setUIText();
    break;
  default:
    break;
  }

  QWidget::changeEvent(pEvent);
}

void lmcMainWindow::sendMessage(MessageType type, QString *lpszUserId,
                                XmlMessage *pMessage) {
  emit messageSent(type, lpszUserId, pMessage);
}

void lmcMainWindow::trayShowAction_triggered() { restore(); }

void lmcMainWindow::trayHistoryAction_triggered() { emit showHistory(QString()); }

void lmcMainWindow::trayFileAction_triggered() { emit showTransfers(QString()); }

void lmcMainWindow::traySettingsAction_triggered() { emit showSettings(); }

void lmcMainWindow::trayAboutAction_triggered() { emit showAbout(); }

void lmcMainWindow::trayExitAction_triggered() { emit appExiting(); }

void lmcMainWindow::statusAction_triggered(QAction *action) {
  QString status = action->data().toString();
  StatusStruct *currentStatus = Globals::getInstance().getStatus(status);

  if (currentStatus) {
    QString icon =
        ThemeManager::getInstance().getAppIcon(currentStatus->icon);
    btnStatus->setIcon(QIcon(icon));
    changeTrayIcon(icon);
    ui.labelStatus->setText(statusGroup->checkedAction()->text());
    pLocalUser->status = currentStatus->description;
    pSettings->setValue(IDS_STATUS, pLocalUser->status);

    sendMessage(MT_Status, NULL, &status);
  }
}

void lmcMainWindow::avatarAction_triggered() { setAvatar(); }

void lmcMainWindow::avatarBrowseAction_triggered() {
  QString dir = pSettings->value(IDS_OPENPATH, StdLocation::getDocumentsPath())
                    .toString();
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Select avatar picture"), dir,
      "Images (*.bmp *.gif *.jpg *.jpeg *.png *.tif *.tiff)");
  if (!fileName.isEmpty()) {
    pSettings->setValue(IDS_OPENPATH, QFileInfo(fileName).dir().absolutePath());
    setAvatar(fileName);
  }
}

void lmcMainWindow::buttonStartChat_clicked() { startChatRoom(); }

void lmcMainWindow::buttonStartPublicChat_clicked() { emit showPublicChat(); }

void lmcMainWindow::refreshAction_triggered() {
  QString szUserId;
  QString szMessage;

  sendMessage(MT_Refresh, &szUserId, &szMessage);
}

void lmcMainWindow::helpAction_triggered() {
  QRect rect = geometry();
  emit showHelp(&rect);
}

void lmcMainWindow::homePageAction_triggered() {
  QDesktopServices::openUrl(QUrl(IDA_DOMAIN));
}

void lmcMainWindow::updateAction_triggered() {
  QRect rect = geometry();
  emit showUpdate(&rect);
}

void
lmcMainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
  case QSystemTrayIcon::Trigger:
    if (singleClickActivation)
      processTrayIconTrigger();
    break;
  case QSystemTrayIcon::DoubleClick:
    if (!singleClickActivation)
      processTrayIconTrigger();
    break;
  default:
    break;
  }
}

void lmcMainWindow::trayMessage_clicked() {
  switch (lastTrayMessageType) {
  case TM_Status:
    trayShowAction_triggered();
    break;
  case TM_Transfer:
    emit showTransfers(QString());
    break;
  case TM_Message:
      emit showMessage(lastTrayMessageChatRoomId);
      break;
  default:
    break;
  }
}

void lmcMainWindow::treeWidgetUserList_itemActivated(QTreeWidgetItem *pItem,
                                                     int column) {
  Q_UNUSED(column);
  if (pItem->data(0, TypeRole).toString().compare("User") == 0) {
    startChatRoom();
  }
}

void lmcMainWindow::treeWidgetUserList_itemContextMenu(QTreeWidgetItem *pItem,
                                                       QPoint &pos) {
  if (!pItem) {
    _usersListMainMenu->exec(pos);
    return;
  }

  if (pItem->data(0, TypeRole).toString().compare("Group") == 0) {
      _groupSendScreenshotAction->setVisible(_clipboard->mimeData()->hasImage());
      _groupSendFileClipboardAction->setVisible(_clipboard->mimeData()->hasUrls());
    for (int index = 0; index < pGroupMenu->actions().count(); index++)
      pGroupMenu->actions()[index]->setData(pItem->data(0, IdRole));

    bool defaultGroup =
        (pItem->data(0, IdRole).toString().compare(GRP_DEFAULT_ID) == 0);
    groupDeleteAction->setEnabled(!defaultGroup);
    pGroupMenu->exec(pos);
  } else if (pItem->data(0, TypeRole).toString().compare("User") == 0) {
      _userSendScreenshotAction->setVisible(_clipboard->mimeData()->hasImage());
      _userSendFileClipboardAction->setVisible(_clipboard->mimeData()->hasUrls());
      _userBroadcastAction->setVisible((getSelectedUserTreeItems().size() > 1)); // more than one user selected

      for (int index = 0; index < pUserMenu->actions().count(); index++)
          pUserMenu->actions()[index]->setData(pItem->data(0, IdRole));

      pUserMenu->exec(pos);
  }
}

void lmcMainWindow::treeWidgetUserList_itemDragDropped(QTreeWidgetItem *item) {
  if (dynamic_cast<lmcUserTreeWidgetUserItem *>(item)) {
    QString szUserId = item->data(0, IdRole).toString();
    QString szMessage = item->parent()->data(0, IdRole).toString();
    sendMessage(MT_Group, &szUserId, &szMessage);
    QTreeWidgetItem *pGroupItem = item->parent();
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
  } else if (dynamic_cast<lmcUserTreeWidgetGroupItem *>(item)) {
    int index = ui.treeWidgetUserList->indexOfTopLevelItem(item);
    QString groupId = item->data(0, IdRole).toString();
    emit groupUpdated(GO_Move, groupId, index);
  }
}

void lmcMainWindow::treeWidgetUserList_fileDragDropped(QTreeWidgetItem *item,
                                                       QStringList fileNames) {
    if (fileNames.isEmpty ())
        return;

  if (dynamic_cast<lmcUserTreeWidgetUserItem *>(item) ||
      dynamic_cast<lmcUserTreeWidgetGroupItem *>(item))
    sendFile(false, item, fileNames);
}

void lmcMainWindow::treeWidgetUserList_itemSelectionChanged() {
    buttonSendFile->setEnabled(ui.treeWidgetUserList->selectedItems().length());
}

void lmcMainWindow::groupAddAction_triggered() {
  QString groupName = QInputDialog::getText(this, tr("Add New Group"),
                                            tr("Enter a name for the group"));

  if (groupName.isNull())
    return;

  if (getGroupItemByName(&groupName)) {
    QString msg =
        tr("A group named '%1' already exists. Please enter a different name.");
    QMessageBox::warning(this, lmcStrings::appName(), msg.arg(groupName));
    return;
  }

  // generate a group id that is not assigned to any existing group
  QString groupId;
  do {
    groupId = Helper::getUuid();
  } while (getGroupItem(&groupId));

  emit groupUpdated(GO_New, groupId, groupName);
  lmcUserTreeWidgetGroupItem *pItem = new lmcUserTreeWidgetGroupItem();
  pItem->setData(0, IdRole, groupId);
  pItem->setData(0, TypeRole, "Group");
  pItem->setText(0, groupName);
  pItem->setSizeHint(0, QSize(0, 20));
  ui.treeWidgetUserList->addTopLevelItem(pItem);
  //	set the item as expanded after adding it to the treeview, else wont work
  pItem->setExpanded(true);
}

void lmcMainWindow::groupRenameAction_triggered() {
  QTreeWidgetItem *pGroupItem = ui.treeWidgetUserList->currentItem();
  QString groupId = pGroupItem->data(0, IdRole).toString();
  QString oldName = pGroupItem->data(0, Qt::DisplayRole).toString();
  QString newName = QInputDialog::getText(this, tr("Rename Group"),
                                          tr("Enter a new name for the group"),
                                          QLineEdit::Normal, oldName);

  if (newName.isNull() || newName.compare(oldName) == 0)
    return;

  if (getGroupItemByName(&newName)) {
    QString msg =
        tr("A group named '%1' already exists. Please enter a different name.");
    QMessageBox::warning(this, lmcStrings::appName(), msg.arg(newName));
    return;
  }

  emit groupUpdated(GO_Rename, groupId, newName);
  pGroupItem->setText(0, newName);
}

void lmcMainWindow::groupDeleteAction_triggered() {
  QTreeWidgetItem *pGroupItem = ui.treeWidgetUserList->currentItem();
  QString groupId = pGroupItem->data(0, IdRole).toString();
  QString defGroupId = GRP_DEFAULT_ID;
  QTreeWidgetItem *pDefGroupItem = getGroupItem(&defGroupId);
  while (pGroupItem->childCount()) {
    QTreeWidgetItem *pUserItem = pGroupItem->child(0);
    pGroupItem->removeChild(pUserItem);
    pDefGroupItem->addChild(pUserItem);
    QString szUserId = pUserItem->data(0, IdRole).toString();
    QString szMessage = pUserItem->parent()->data(0, IdRole).toString();
    sendMessage(MT_Group, &szUserId, &szMessage);
  }
  pDefGroupItem->sortChildren(0, Qt::AscendingOrder);

  emit groupUpdated(GO_Delete, groupId, QVariant());
  ui.treeWidgetUserList->takeTopLevelItem(
      ui.treeWidgetUserList->indexOfTopLevelItem(pGroupItem));
}

void lmcMainWindow::userConversationAction_triggered() { startChatRoom(); }

void lmcMainWindow::buttonSendBroadcast_clicked() { emit showBroadcast(); }

void lmcMainWindow::actionSendBroadcast_triggered() {
  emit showBroadcast();
}

void lmcMainWindow::buttonSendFile_clicked() { sendFile(); }

void lmcMainWindow::sendFile(bool sendFolder, QTreeWidgetItem *dropTarget,
                             QStringList files) {
  QString dir = pSettings->value(IDS_OPENPATH, StdLocation::getDocumentsPath())
                    .toString();

  bool filedDragDropped = !files.isEmpty();

  QStringList fileNames;
  if (!files.isEmpty())
    fileNames = files;
  else {
    if (!sendFolder)
      fileNames = QFileDialog::getOpenFileNames(this, QString(), dir);
    else {
      QString directoryUrl = QFileDialog::getExistingDirectory(
          this, QString(), dir, QFileDialog::ShowDirsOnly);
      if (!directoryUrl.isEmpty())
        fileNames.append(directoryUrl);
    }
  }

  if (fileNames.isEmpty())
    return;

  if (!filedDragDropped)
    pSettings->setValue(IDS_OPENPATH,
                        QFileInfo(fileNames[0]).dir().absolutePath());

  QList<QTreeWidgetItem *> selectedItems = getSelectedUserTreeItems(dropTarget);

  MessageType messageType = sendFolder ? MT_Folder : MT_File;
  QFileInfo fileInfo;

  QString chatRoomId = pLocalUser->id;
  chatRoomId.append(Helper::getUuid());

  for (QTreeWidgetItem *item : selectedItems) {
    QString userId = item->data(0, IdRole).toString();

    for (QString &fileName : fileNames) {
      if (filedDragDropped) {
        fileInfo.setFile(fileName);
        messageType = fileInfo.isDir() ? MT_Folder : MT_File;
      }

      sendMessage(messageType, &userId, &fileName, chatRoomId);
    }
  }
}

void lmcMainWindow::startChatRoom(bool fromToolbar) {
    QString chatRoomId = pLocalUser->id;
    chatRoomId.append(Helper::getUuid());
    emit chatRoomStarting(chatRoomId, XmlMessage(),
                          !fromToolbar ? getSelectedUserIds() : QStringList());
}

void lmcMainWindow::userFolderAction_triggered() { sendFile(true); }

void lmcMainWindow::userhistoryAction_triggered() {
  QTreeWidgetItem *item = ui.treeWidgetUserList->currentItem ();
  emit showHistory (item->text(0));
}

void lmcMainWindow::usertransferAction_triggered() {
    QTreeWidgetItem *item = ui.treeWidgetUserList->currentItem ();
  emit showTransfers (item->text(0));
}

QList<QTreeWidgetItem *>
lmcMainWindow::getSelectedUserTreeItems(QTreeWidgetItem *dropTarget) {
  QList<QTreeWidgetItem *> selectedItems;
  QList<QTreeWidgetItem *> tempSelectedItems;
  if (!dropTarget) {
    tempSelectedItems = ui.treeWidgetUserList->selectedItems();

    if (tempSelectedItems.size() > 0) {
        QTreeWidgetItem *currentItem = ui.treeWidgetUserList->currentItem();
        if (currentItem && !currentItem->isSelected())
            tempSelectedItems.append(currentItem);
    }
  } else
    tempSelectedItems.append(dropTarget);

  for (QTreeWidgetItem *item : tempSelectedItems) {
    if (item->data(0, TypeRole).toString().compare("Group") == 0)
      for (int index = 0; index < item->childCount(); ++index)
        selectedItems.append(item->child(index));
    else
      selectedItems.append(item);
  }

  return selectedItems;
}

QStringList lmcMainWindow::getSelectedUserIds() {
    if (ui.treeWidgetUserList->selectedItems().size() == 0)
        return QStringList();

    QList<QTreeWidgetItem *> selectedItems = getSelectedUserTreeItems();
    QStringList userIds;

    for (QTreeWidgetItem *item : selectedItems)
        userIds.append(item->data(0, IdRole).toString());

    return userIds;
}

void lmcMainWindow::userInfoAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  QString message;
  sendMessage(MT_Query, &userId, &message);
}

void lmcMainWindow::userSendScreenshotAction_triggered() {
    if (!_clipboard->mimeData()->hasImage())
        return;

    QPixmap imageData = qvariant_cast<QPixmap>(_clipboard->mimeData()->imageData());
    QString screenshotPath = QString("%1screenshot - %2.png").arg(StdLocation::getCacheDir(), pLocalUser->name);
    imageData.save(screenshotPath);
    sendFile(false, nullptr, QStringList() << screenshotPath);
}

void lmcMainWindow::userSendFileClipboard_triggered() {
    if (!_clipboard->mimeData()->hasUrls())
        return;

  QList<QUrl> urls = _clipboard->mimeData()->urls();
  QStringList files;
  for (QUrl url : urls) {
      files.append(url.path().remove(0, 1));
  }

  sendFile(false, nullptr, files);
}

void lmcMainWindow::textBoxNote_returnPressed() {
  //	Shift the focus from textBoxNote to another control
  ui.treeWidgetUserList->setFocus();
}

void lmcMainWindow::textBoxNote_lostFocus() {
  QString note = ui.textBoxNote->text();
  pSettings->setValue(IDS_NOTE, note);
  pLocalUser->note = note;
  sendMessage(MT_Note, NULL, &note);
}

void lmcMainWindow::createMainMenu() {
  pMainMenu = new QMenuBar(this);
  pFileMenu = pMainMenu->addMenu("&Messenger");
  chatRoomAction = pFileMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("newchatroom")),
      "&New Chat Room", this, SLOT(buttonStartChat_clicked()),
      QKeySequence::New);
  publicChatAction = pFileMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("chatroom")),
      "&Public Chat", this, SLOT(buttonStartPublicChat_clicked()));
  pFileMenu->addSeparator();
  refreshAction = pFileMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("refresh")),
      "&Refresh contacts list", this, SLOT(refreshAction_triggered()),
      QKeySequence::Refresh);
  pFileMenu->addSeparator();
  exitAction = pFileMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("close")),
      "E&xit", this, SLOT(trayExitAction_triggered()));
  pToolsMenu = pMainMenu->addMenu("&Tools");
  historyAction = pToolsMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("history")),
      "&History", this, SLOT(trayHistoryAction_triggered()),
      QKeySequence(Qt::CTRL + Qt::Key_H));
  transferAction = pToolsMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("transfer")),
      "File &Transfers", this, SLOT(trayFileAction_triggered()),
      QKeySequence(Qt::CTRL + Qt::Key_J));
  pToolsMenu->addSeparator();
  settingsAction = pToolsMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("tools")),
      "&Preferences", this, SLOT(traySettingsAction_triggered()),
      QKeySequence::Preferences);
  pHelpMenu = pMainMenu->addMenu("&Help");
  helpAction = pHelpMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("question")),
      "&Help", this, SLOT(helpAction_triggered()), QKeySequence::HelpContents);
  pHelpMenu->addSeparator();
  QString text = "%1 &online";
  onlineAction = pHelpMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("web")),
      text.arg(lmcStrings::appName()), this, SLOT(homePageAction_triggered()));
  updateAction = pHelpMenu->addAction("Check for &Updates", this,
                                      SLOT(updateAction_triggered()));
  aboutAction = pHelpMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("info")),
      "&About", this, SLOT(trayAboutAction_triggered()));

  layout()->setMenuBar(pMainMenu);
}

void lmcMainWindow::createTrayMenu() {
  pTrayMenu = new QMenu(this);

  QString text = "&Show %1";
  trayShowAction = pTrayMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("messenger")),
      text.arg(lmcStrings::appName()), this, SLOT(trayShowAction_triggered()));
  pTrayMenu->addSeparator();
  trayStatusAction = pTrayMenu->addMenu(pStatusMenu);
  trayStatusAction->setText("&Change Status");
  pTrayMenu->addSeparator();
  trayHistoryAction = pTrayMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("chatroom")),
      "&History", this, SLOT(trayHistoryAction_triggered()));
  trayTransferAction = pTrayMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("transfer")),
      "File &Transfers", this, SLOT(trayFileAction_triggered()));
  pTrayMenu->addSeparator();
  traySettingsAction = pTrayMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("tools")),
      "&Preferences", this, SLOT(traySettingsAction_triggered()));
  trayAboutAction = pTrayMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("info")),
      "&About", this, SLOT(trayAboutAction_triggered()));
  pTrayMenu->addSeparator();
  trayExitAction = pTrayMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("close")),
      "E&xit", this, SLOT(trayExitAction_triggered()));

  pTrayMenu->setDefaultAction(trayShowAction);
}

void lmcMainWindow::createTrayIcon() {
  pTrayIcon = new QSystemTrayIcon(this);
  pTrayIcon->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon("logosmall")));
  pTrayIcon->setContextMenu(pTrayMenu);

  connect(pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
          SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));
  connect(pTrayIcon, SIGNAL(messageClicked()), this,
          SLOT(trayMessage_clicked()));
}

void lmcMainWindow::changeTrayIcon(QString icon) {
  pTrayIcon->setIcon(
      QIcon(icon));
}

void lmcMainWindow::createStatusMenu() {
  pStatusMenu = new QMenu(this);
  statusGroup = new QActionGroup(this);
  connect(statusGroup, SIGNAL(triggered(QAction *)), this,
          SLOT(statusAction_triggered(QAction *)));

  std::vector<StatusStruct> statuses = Globals::getInstance().getStatuses();
  for (unsigned index = 0; index < statuses.size(); index++) {
    QAction *pAction = new QAction(
        QIcon(ThemeManager::getInstance().getAppIcon(
            statuses[index].icon)),
        statuses[index].uiDescription, this);
    pAction->setData(statuses[index].description);
    pAction->setCheckable(true);
    statusGroup->addAction(pAction);
    pStatusMenu->addAction(pAction);
  }

  btnStatus->setMenu(pStatusMenu);
}

void lmcMainWindow::createAvatarMenu() {
  if (pAvatarMenu) {
    pAvatarMenu->clear();
    pAvatarMenu->deleteLater();
  }
  pAvatarMenu = new QMenu(this);

  lmcImagePickerAction *pAction = new lmcImagePickerAction(
      pAvatarMenu, ImagesList::getInstance().getAvatars(),
      ImagesList::getInstance().getTabs(ImagesList::Avatars), 60, 60, 6,
      &nAvatar);
  connect(pAction, &lmcImagePickerAction::imageSelected, this, &lmcMainWindow::avatarAction_triggered);
  pAvatarMenu->addAction(pAction);
  pAvatarMenu->addSeparator();
  _avatarBrowseAction = pAvatarMenu->addAction(
      "&Select picture...", this, SLOT(avatarBrowseAction_triggered()));

  ui.buttonAvatar->setMenu(pAvatarMenu);
}

void lmcMainWindow::createGroupMenu() {
  pGroupMenu = new QMenu(this);

  groupChatAction = pGroupMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("newchatroom")),
      "Start &Conversation", this, SLOT(userConversationAction_triggered()));
  groupFileAction = pGroupMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfile")),
      "Send &File", this, SLOT(buttonSendFile_clicked()));
  groupFolderAction = pGroupMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfolder")),
      "Send a Fol&der", this, SLOT(userFolderAction_triggered()));
  groupBroadcastAction = pGroupMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("broadcastmsg")),
      "Send &Broadcast Message", this, SLOT(actionSendBroadcast_triggered()));
  pGroupMenu->addSeparator();
  groupAddAction = pGroupMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("add")),
      "Add &New Group", this, SLOT(groupAddAction_triggered()));
  pGroupMenu->addSeparator();
  groupRenameAction = pGroupMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("rename")),
      "&Rename This Group", this, SLOT(groupRenameAction_triggered()));
  groupDeleteAction = pGroupMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("delete")),
      "&Delete This Group", this, SLOT(groupDeleteAction_triggered()));

  pGroupMenu->addSeparator();

  _groupSendScreenshotAction = pGroupMenu->addAction(
              QIcon(
                  ThemeManager::getInstance().getAppIcon("image")),
              "Send copied image", this, SLOT(userSendScreenshotAction_triggered()));
  _groupSendFileClipboardAction = pGroupMenu->addAction(
              QIcon(
                  ThemeManager::getInstance().getAppIcon("file")),
              "Send copied file(s) and/or folder(s)", this, SLOT(userSendFileClipboard_triggered()));
}

void lmcMainWindow::createUserMenu() {
  pUserMenu = new QMenu(this);

  _userChatAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("newchatroom")),
      "Start &Conversation", this, SLOT(userConversationAction_triggered()));
  _userFileAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfile")),
      "Send &File", this, SLOT(buttonSendFile_clicked()));
  _userFolderAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfolder")),
      "Send a Fol&der", this, SLOT(userFolderAction_triggered()));
  pUserMenu->addSeparator();
  _userBroadcastAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("broadcastmsg")),
      "Send &Broadcast Message", this, SLOT(actionSendBroadcast_triggered()));
  pUserMenu->addSeparator();
  _userhistoryAction = pUserMenu->addAction(
      QIcon(ThemeManager::getInstance().getAppIcon("history")),
      "Show &History", this, SLOT(userhistoryAction_triggered()));
  _usertransferAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("transfer")),
      "Show File &Transfers", this, SLOT(usertransferAction_triggered()));
  pUserMenu->addSeparator();
  _userInfoAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("userInfo")),
      "Get &Information", this, SLOT(userInfoAction_triggered()));

  pUserMenu->addSeparator();

  _userSendScreenshotAction = pUserMenu->addAction(
              QIcon(
                  ThemeManager::getInstance().getAppIcon("clipboard_up")),
              "Send copied image", this, SLOT(userSendScreenshotAction_triggered()));
  _userSendFileClipboardAction = pUserMenu->addAction(
              QIcon(
                  ThemeManager::getInstance().getAppIcon("clipboard_up")),
              "Send copied file(s) and/or folder(s)", this, SLOT(userSendFileClipboard_triggered()));
}

void lmcMainWindow::createUsersListMainMenu() {
  _usersListMainMenu = new QMenu(this);

  _usersMainAddAction = _usersListMainMenu->addAction(
      "Add &New Group", this, SLOT(groupAddAction_triggered()));
}

void lmcMainWindow::createToolBar() {
    // TODO remove this toolbar
  QToolBar *pStatusBar = new QToolBar(ui.widgetStatus);
  pStatusBar->setStyleSheet("QToolBar { border: 0px; padding: 0px; background: transparent; }");
  ui.widgetstatus_layout->insertWidget(0, pStatusBar);

  btnStatus = new ThemedButton(pStatusBar);
  btnStatus->setPopupMode(QToolButton::InstantPopup);
  btnStatus->setToolButtonStyle(Qt::ToolButtonIconOnly);
  pStatusBar->addWidget(btnStatus);

  QToolBar *pToolBar = new QToolBar(ui.widgetToolBar);
  pToolBar->setIconSize(QSize(40, 20));
  ui.widgetToolBar_layout->addWidget(pToolBar);

  buttonStartChat = new ThemedButton(pToolBar);
  buttonStartChat->setIcon(QIcon(
      ThemeManager::getInstance().getAppIcon(QStringLiteral("chat"))));
  buttonStartChat->setToolButtonStyle(Qt::ToolButtonIconOnly);

  connect(buttonStartChat, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonStartChat_clicked);

  buttonSendFile = new ThemedButton(pToolBar);
  buttonSendFile->setIcon(QIcon(
      ThemeManager::getInstance().getAppIcon(QStringLiteral("file"))));
  buttonSendFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonSendFile->setEnabled(false);

  connect(buttonSendFile, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonSendFile_clicked);

  buttonSendBroadcast = new ThemedButton(pToolBar);
  buttonSendBroadcast->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon(
          QStringLiteral("broadcastmsg"))));
  buttonSendBroadcast->setToolButtonStyle(Qt::ToolButtonIconOnly);

  connect(buttonSendBroadcast, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonSendBroadcast_clicked);

  buttonStartPublicChat = new ThemedButton(pToolBar);
  buttonStartPublicChat->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon(
          QStringLiteral("chatroom"))));
  buttonStartPublicChat->setToolButtonStyle(Qt::ToolButtonIconOnly);

  connect(buttonStartPublicChat, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonStartPublicChat_clicked);

  pToolBar->addWidget(buttonStartChat);
  pToolBar->addWidget(buttonSendFile);
  pToolBar->addSeparator();
  pToolBar->addWidget(buttonSendBroadcast);
  pToolBar->addSeparator();
  pToolBar->addWidget(buttonStartPublicChat);
}

void lmcMainWindow::setUIText() {
  ui.retranslateUi(this);

  setWindowTitle(lmcStrings::appName());

  pFileMenu->setTitle(tr("&Messenger"));
  chatRoomAction->setText(tr("&New Chat Room"));
  publicChatAction->setText(tr("&Public Chat"));
  refreshAction->setText(tr("&Refresh Contacts List"));
  exitAction->setText(tr("E&xit"));
  pToolsMenu->setTitle(tr("&Tools"));
  historyAction->setText(tr("&History"));
  transferAction->setText(tr("File &Transfers"));
  settingsAction->setText(tr("&Preferences"));
  pHelpMenu->setTitle(tr("&Help"));
  helpAction->setText(tr("&Help"));
  QString text = tr("%1 &online");
  onlineAction->setText(text.arg(lmcStrings::appName()));
  updateAction->setText(tr("Check for &Updates..."));
  aboutAction->setText(tr("&About"));
  text = tr("&Show %1");
  trayShowAction->setText(text.arg(lmcStrings::appName()));
  trayStatusAction->setText(tr("&Change Status"));
  trayHistoryAction->setText(tr("&History"));
  trayTransferAction->setText(tr("File &Transfers"));
  traySettingsAction->setText(tr("&Preferences"));
  trayAboutAction->setText(tr("&About"));
  trayExitAction->setText(tr("E&xit"));
  groupAddAction->setText(tr("Add &New Group"));
  groupRenameAction->setText(tr("&Rename This Group"));
  groupDeleteAction->setText(tr("&Delete This Group"));
  _userChatAction->setText(tr("&Conversation"));
  _userBroadcastAction->setText(tr("Send &Broadcast Message"));
  _userFileAction->setText(tr("Send &File"));
  _userFolderAction->setText(tr("Send Fol&der"));
  _userInfoAction->setText(tr("Get &Information"));
  _avatarBrowseAction->setText(tr("&Browse for more pictures..."));
  buttonStartChat->setText("Start &Conversation");
  buttonStartChat->setToolTip("Start Conversation");
  buttonSendFile->setText("Send &File");
  buttonSendFile->setToolTip("Send File");
  buttonSendBroadcast->setText("Send &Broadcast Message");
  buttonSendBroadcast->setToolTip("Send Broadcast Message");
  buttonStartPublicChat->setText("&Public Chat");
  buttonStartPublicChat->setToolTip("Public Chat");

  auto statuses = Globals::getInstance().getStatuses();
  for (int index = 0; index < statusGroup->actions().count(); index++)
    statusGroup->actions()[index]->setText(statuses[index].uiDescription);

  ui.labelUserName->setText(pLocalUser->name); // in case of retranslation
  if (statusGroup->checkedAction())
    ui.labelStatus->setText(statusGroup->checkedAction()->text());

  for (int index = 0; index < ui.treeWidgetUserList->topLevelItemCount();
       index++) {
    QTreeWidgetItem *item = ui.treeWidgetUserList->topLevelItem(index);
    for (int childIndex = 0; childIndex < item->childCount(); childIndex++) {
      QTreeWidgetItem *childItem = item->child(childIndex);
      int statusIndex = childItem->data(0, StatusRole).toInt();
      childItem->setToolTip(0, statuses[statusIndex].description);
    }
  }

  setTrayTooltip();
}

void lmcMainWindow::showMinimizeMessage() {
  if (showMinimizeMsg) {
    QString msg = tr("%1 will continue to run in the background. Activate this "
                     "icon to restore the application window.");
    showTrayMessage(TM_Minimize, msg.arg(lmcStrings::appName()));
    showMinimizeMsg = false;
  }
}

void lmcMainWindow::initGroups(QList<Group> *pGroupList) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.initGroups started"));

  for (int index = 0; index < pGroupList->count(); index++) {
    lmcUserTreeWidgetGroupItem *pItem = new lmcUserTreeWidgetGroupItem();
    pItem->setData(0, IdRole, pGroupList->value(index).id);
    pItem->setData(0, TypeRole, "Group");
    pItem->setText(0, pGroupList->value(index).name);
    pItem->setSizeHint(0, QSize(0, 22));
    ui.treeWidgetUserList->addTopLevelItem(pItem);
  }

  ui.treeWidgetUserList->expandAll();
  // size will be either number of items in group expansion list or number of
  // top level items in
  // treeview control, whichever is less. This is to  eliminate arary out of
  // bounds error.
  int size = qMin(pSettings->beginReadArray(IDS_GROUPEXPHDR),
                  ui.treeWidgetUserList->topLevelItemCount());
  for (int index = 0; index < size; index++) {
    pSettings->setArrayIndex(index);
    ui.treeWidgetUserList->topLevelItem(index)
        ->setExpanded(pSettings->value(IDS_GROUP).toBool());
  }
  pSettings->endArray();

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.initGroups ended"));
}

void lmcMainWindow::updateStatusImage(QTreeWidgetItem *pItem,
                                      QString *lpszStatus) {
  StatusStruct *currentStatus = Globals::getInstance().getStatus(*lpszStatus);

  if (currentStatus)
    pItem->setIcon(0,
                   QIcon(ThemeManager::getInstance().getAppIcon(
                       currentStatus->icon)));
}

void lmcMainWindow::setAvatar(QString fileName) {
  QPixmap avatar;
  QString avatarPath;

  if (!fileName.isEmpty()) {
    avatarPath = ImagesList::getInstance().addAvatar(fileName);
    nAvatar = ImagesList::getInstance().getAvatarIndex(avatarPath);
    createAvatarMenu();
  } else {
    avatarPath = ImagesList::getInstance().getAvatar(nAvatar);
  }

  avatar = QPixmap(avatarPath);

  ui.buttonAvatar->setIcon(QIcon(avatar));
  pLocalUser->avatar = nAvatar;
  pLocalUser->avatarPath = avatarPath;
  sendAvatar(NULL);
}

QTreeWidgetItem *lmcMainWindow::getUserItem(QString *lpszUserId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    for (int index = 0;
         index < ui.treeWidgetUserList->topLevelItem(topIndex)->childCount();
         index++) {
      QTreeWidgetItem *pItem =
          ui.treeWidgetUserList->topLevelItem(topIndex)->child(index);
      if (pItem->data(0, IdRole).toString().compare(*lpszUserId) == 0)
        return pItem;
    }
  }

  return NULL;
}

QTreeWidgetItem *lmcMainWindow::getGroupItem(QString *lpszGroupId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    QTreeWidgetItem *pItem = ui.treeWidgetUserList->topLevelItem(topIndex);
    if (pItem->data(0, IdRole).toString().compare(*lpszGroupId) == 0)
      return pItem;
  }

  return NULL;
}

QTreeWidgetItem *lmcMainWindow::getGroupItemByName(QString *lpszGroupName) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    QTreeWidgetItem *pItem = ui.treeWidgetUserList->topLevelItem(topIndex);
    if (pItem->data(0, Qt::DisplayRole).toString().compare(*lpszGroupName) == 0)
      return pItem;
  }

  return NULL;
}

void lmcMainWindow::sendMessage(MessageType type, QString *lpszUserId,
                                QString *lpszMessage, QString chatRoomId) {
  XmlMessage xmlMessage;

  switch (type) {
  case MT_Status:
    xmlMessage.addData(XN_STATUS, *lpszMessage);
    break;
  case MT_Note:
    xmlMessage.addData(XN_NOTE, *lpszMessage);
    break;
  case MT_Refresh:
    break;
  case MT_Group:
    xmlMessage.addData(XN_GROUP, *lpszMessage);
    break;
  case MT_File:
  case MT_Folder:
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
    xmlMessage.addData(XN_FILEPATH, *lpszMessage);
    xmlMessage.addData(XN_THREAD, chatRoomId);
    break;
  case MT_Avatar:
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Avatar]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
    xmlMessage.addData(XN_FILEPATH, *lpszMessage);
    break;
  case MT_Query:
    xmlMessage.addData(XN_QUERYOP, QueryOpNames[QO_Get]);
    break;
  default:
    break;
  }

  sendMessage(type, lpszUserId, &xmlMessage);
}

void lmcMainWindow::sendAvatar(QString *lpszUserId) {
  QString filePath = ImagesList::getInstance().getAvatar(pLocalUser->avatar);
  if (!QFile::exists(filePath))
    return;

  sendMessage(MT_Avatar, lpszUserId, &filePath);
}

void lmcMainWindow::setUserAvatar(QString *lpszUserId, QString *lpszFilePath) {
  QTreeWidgetItem *pUserItem = getUserItem(lpszUserId);
  if (!pUserItem)
    return;

  QPixmap avatar;
  if (!lpszFilePath || !QFile::exists(*lpszFilePath))
    avatar.load(ImagesList::getInstance().getDefaultAvatar());
  else
    avatar.load(*lpszFilePath);

  if (!avatar.isNull()) {
      avatar = avatar.scaled(QSize(32, 32), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
      pUserItem->setData(0, AvatarRole, QIcon(avatar));
  }
}

void lmcMainWindow::processTrayIconTrigger() {
  // If system tray minimize is disabled, restore() will be called every time.
  // Otherwise, window is restored or minimized
  if (!allowSysTrayMinimize || isHidden() || isMinimized())
    restore();
  else
    minimize();
}

void lmcMainWindow::setTrayTooltip() {
  if (bConnected)
    pTrayIcon->setToolTip(lmcStrings::appName());
  else {
    QString msg = tr("%1 - Not Connected");
    pTrayIcon->setToolTip(msg.arg(lmcStrings::appName()));
  }
}
