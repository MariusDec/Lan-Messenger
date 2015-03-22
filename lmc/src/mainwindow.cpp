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

#include "mainwindow.h"
#include "messagelog.h"
#include "history.h"
#include "loggermanager.h"
#include "imageslist.h"
#include "globals.h"

#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QMimeData>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

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

  _windowLoaded = false;
}

lmcMainWindow::~lmcMainWindow() {}

void lmcMainWindow::init(User *localUser, const QList<Group> &groupList,
                         bool connected) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.init started"));

  setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

  _localUser = localUser;

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

  // get current status struct
  StatusStruct *currentStatus = nullptr;
  int statusIndex;

  pSettings = new lmcSettings();
  if (Globals::getInstance().restoreStatus()) {
      auto statuses = Globals::getInstance().getStatuses();
      for (unsigned index = 0; index < statuses.size(); ++index)
          if (!statuses[index].description.compare(localUser->status)) {
              currentStatus = &statuses[index];
              statusIndex = index;
              break;
          }
  }

  if (!currentStatus) {
      // set available status
      currentStatus = &Globals::getInstance().getStatuses().front();
      statusIndex = 0;
      _localUser->status = currentStatus->description;
  }

  _buttonStatus->setIcon(QIcon(QPixmap(
      ThemeManager::getInstance().getAppIcon(currentStatus->icon))));
  _statusGroup->actions()[statusIndex]->setChecked(true);
  QFont font = ui.labelUserName->font();
  int fontSize = ui.labelUserName->fontInfo().pixelSize();
  fontSize += (fontSize * 0.1);
  font.setPixelSize(fontSize);
  font.setBold(true);
  ui.labelUserName->setFont(font);
  ui.labelStatus->setText(_statusGroup->checkedAction()->text());
  nAvatar = localUser->avatar;
  ui.textBoxNote->setText(localUser->note);

  pSoundPlayer = new lmcSoundPlayer();
  if (!Globals::getInstance().mainWindowGeometry().isEmpty())
      restoreGeometry(Globals::getInstance().mainWindowGeometry());
  else
      move(50, 50);

  //	get saved settings
  settingsChanged(true);
  setUIText();

  initGroups(groupList);

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
  // This method should only be called from here, otherwise an MT_Notify message is sent
  // and the program will connect to the network before start() is called.
  setAvatar();
  _trayIcon->setVisible(Globals::getInstance().sysTray());
  if (Globals::getInstance().autoShow())
    restore();
}

void lmcMainWindow::show() {
  _windowLoaded = true;
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
  //showMinimizeMessage();
}

void lmcMainWindow::stop() {
  //	These settings are saved only if the window was opened at least once by
  // the user
  if (_windowLoaded) {
    Globals::getInstance().setMainWindowGeometry(saveGeometry());
  }

  pSettings->beginWriteArray(IDS_GROUPEXPHDR);
  for (int index = 0; index < ui.treeWidgetUserList->topLevelItemCount();
       index++) {
    pSettings->setArrayIndex(index);
    pSettings->setValue(
        IDS_GROUP, ui.treeWidgetUserList->topLevelItem(index)->isExpanded());
  }
  pSettings->endArray();

  _trayIcon->hide();
}

void lmcMainWindow::addUser(User *user) {
  if (!user)
    return;

  // get current status struct
  int index;
  StatusStruct *currentStatus =
      Globals::getInstance().getStatus(user->status, &index);

  lmcUserTreeWidgetUserItem *item = new lmcUserTreeWidgetUserItem();
  item->setData(0, IdRole, user->id);
  item->setData(0, TypeRole, "User");
  item->setData(0, StatusRole, index);
  item->setData(0, SubtextRole, user->note);
  item->setData(0, CapsRole, user->caps);
  item->setData(0, DataRole, user->lanIndex);
  item->setText(0, user->name);

  if (currentStatus)
    item->setIcon(
        0, QIcon(ThemeManager::getInstance().getAppIcon(
               currentStatus->icon)));

  lmcUserTreeWidgetGroupItem *pGroupItem =
      (lmcUserTreeWidgetGroupItem *)getGroupItem(&user->group);
  pGroupItem->addChild(item);
  pGroupItem->sortChildren(0, Qt::AscendingOrder);

  QString userTooltip = QString("<b>%1</b><br />Name: %2<br />Note: %3<br />IP: %4<br />Computer: %5<br />Version: %6").arg(currentStatus->uiDescription, user->name, user->note, user->address, user->hostName, user->version);
  ui.treeWidgetUserList->setItemTooltip(item, getUserTooltip(userTooltip, user->avatarPath));

  // this should be called after item has been added to tree
  setUserAvatar(user->id, user->avatarPath);

  if (isHidden() || !isActiveWindow()) {
    QString msg = tr("%1 is online.");
    showTrayMessage(TM_Status, msg.arg(item->text(0)));
    pSoundPlayer->play(SE_UserOnline);
  }

  sendAvatar(user->id);
}

QWidget *lmcMainWindow::getUserTooltip(QString userDetails, QString imagePath) {
  QWidget *toolTipWidget = new QWidget();
  toolTipWidget->setProperty("isWindow", true);
  toolTipWidget->setStyleSheet("border: 1px outset gray; border-radius: 1px;");

  toolTipWidget->setObjectName (QStringLiteral("toolTipWidget"));
  QHBoxLayout *layout = new QHBoxLayout(toolTipWidget);
  layout->setObjectName (QStringLiteral("layout"));
  layout->setMargin(6);


  QLabel *labelDescription = new QLabel(toolTipWidget);
  labelDescription->setObjectName (QStringLiteral("labelDescription"));
  labelDescription->setText(userDetails);
  labelDescription->setWordWrap(true);
  labelDescription->setMaximumWidth(220);
  labelDescription->setStyleSheet("border: none;");
  layout->addWidget(labelDescription, 0);

  QLabel *labelImage = new QLabel(toolTipWidget);
  labelImage->setObjectName (QStringLiteral("labelAvatar"));
  labelImage->setPixmap(QPixmap(imagePath).scaled(64, 64, Qt::KeepAspectRatio, Qt::FastTransformation));
  layout->addWidget(labelImage, 0, Qt::AlignTop);
  labelImage->setStyleSheet("border: 1px solid gray; border-radius: 2px;");

  QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
  effect->setOffset(1, 1);
  effect->setBlurRadius(3);
  effect->setColor(qRgba(101, 101, 101, 10));
  labelImage->setGraphicsEffect(effect);

  return toolTipWidget;
}

void lmcMainWindow::updateUser(User *user) {
  if (!user)
    return;

  lmcUserTreeWidgetItem *item = getUserItem(user->id);
  if (item) {
    updateStatusImage(item, &user->status);

    // get current status struct
    int index;
    StatusStruct *currentStatus =
        Globals::getInstance().getStatus(user->status, &index);

    item->setData(0, StatusRole, index);
    item->setData(0, SubtextRole, user->note);
    item->setText(0, user->name);

    QString userTooltip = QString("<b>%1</b><br />Name: %2<br />Note: %3<br />IP: %4<br />Computer: %5<br />Version: %6").arg(currentStatus->uiDescription, user->name, user->note, user->address, user->hostName, user->version);
    ui.treeWidgetUserList->setItemTooltipDetails(item, userTooltip);

    QTreeWidgetItem *pGroupItem = item->parent();
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
  }
}

void lmcMainWindow::removeUser(const QString &userId) {
  lmcUserTreeWidgetItem *item = getUserItem(userId);
  if (!item)
    return;

  ui.treeWidgetUserList->removeItemTooltip(item);

  QTreeWidgetItem *pGroup = item->parent();
  pGroup->removeChild(item);

  if (isHidden() || !isActiveWindow()) {
    QString msg = tr("%1 is offline.");
    showTrayMessage(TM_Status, msg.arg(item->text(0)));
    pSoundPlayer->play(SE_UserOffline);
  }
}

void lmcMainWindow::receiveMessage(MessageType type, const QString &userId,
                                   const XmlMessage &mssage) {
  QString filePath;

  switch (type) {
  case MT_Avatar:
  {
    filePath = mssage.data(XN_FILEPATH);
    setUserAvatar(userId, filePath);

    ui.treeWidgetUserList->setItemTooltipAvatar(getUserItem(userId), filePath);
  }
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
  //	this operation should not be done when window inits
  if (!init)
    _trayIcon->setVisible(Globals::getInstance().sysTray());

  ui.treeWidgetUserList->setView(Globals::getInstance().userListView());

  pSoundPlayer->settingsChanged();
  ui.labelUserName->setText(_localUser->name); // in case display name has been changed
}

void lmcMainWindow::showTrayMessage(TrayMessageType type, const QString &message, const QString &chatRoomId,
                                    QString title, TrayMessageIcon icon) {
  if (!Globals::getInstance().sysTray() || !Globals::getInstance().sysTrayMessages())
    return;

  switch (type) {
  case TM_Status:
    if (!Globals::getInstance().enableAlerts() || (_localUser->status == "Busy" && Globals::getInstance().noBusyAlerts()) ||
        (_localUser->status == "NoDisturb" && Globals::getInstance().noDNDAlerts()))
      return;
    break;
  default:
    break;
  }

  if (title.isNull())
    title = lmcStrings::appName();

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

  if (Globals::getInstance().sysTray()) {
    lastTrayMessageType = type;
    lastTrayMessageChatRoomId = chatRoomId;
    _trayIcon->showMessage(title, message, trayIcon);
  }
}

QList<QTreeWidgetItem *> lmcMainWindow::getContactsList() {
  QList<QTreeWidgetItem *> contactsList;
  for (int index = 0; index < ui.treeWidgetUserList->topLevelItemCount();
       index++)
    contactsList.append(ui.treeWidgetUserList->topLevelItem(index)->clone());

  return contactsList;
}

bool lmcMainWindow::eventFilter(QObject *object, QEvent *event) {
  Q_UNUSED(object);

  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(event);
    if (pKeyEvent->key() == Qt::Key_Escape) {
      close();
      return true;
    }
  }

  return false;
}

void lmcMainWindow::closeEvent(QCloseEvent *event) {
  //	close main window to system tray
  event->ignore();
  minimize();
}

void lmcMainWindow::changeEvent(QEvent *event) {
  switch (event->type()) {
  case QEvent::WindowStateChange:
    if (Globals::getInstance().minimizeToTray()) {
      QWindowStateChangeEvent *e = (QWindowStateChangeEvent *)event;
      if (isMinimized() && e->oldState() != Qt::WindowMinimized) {
        QTimer::singleShot(0, this, SLOT(hide()));
        event->ignore();
      }
    }
    break;
  case QEvent::LanguageChange:
    setUIText();
    break;
  default:
    break;
  }

  QWidget::changeEvent(event);
}

void lmcMainWindow::moveEvent(QMoveEvent *event)
{
    if (!Globals::getInstance().windowSnapping()) {
        QWidget::moveEvent(event);
        return;
    }

    const QRect screen = QApplication::desktop()->availableGeometry(this);

    bool windowSnapped = false;

    if (std::abs(frameGeometry().left() - screen.left()) < 25) {
        move(screen.left(), frameGeometry().top());
        windowSnapped = true;
    } else if (std::abs(screen.right() - frameGeometry().right()) < 25) {
        move((screen.right() - frameGeometry().width() + 1), frameGeometry().top());
        windowSnapped = true;
    }

    if (std::abs(frameGeometry().top() - screen.top()) < 25) {
        move(frameGeometry().left(), screen.top());
        windowSnapped = true;
    } else if (std::abs(screen.bottom() - frameGeometry().bottom()) < 25) {
        move(frameGeometry().left(), (screen.bottom() - frameGeometry().height() + 1));
        windowSnapped = true;
    }

    if (!windowSnapped)
        QWidget::moveEvent(event);
}

void lmcMainWindow::sendMessage(MessageType type, QString userId,
                                XmlMessage message) {
  emit messageSent(type, userId, message);
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
    _buttonStatus->setIcon(QIcon(icon));
    changeTrayIcon(icon);
    ui.labelStatus->setText(_statusGroup->checkedAction()->text());
    _localUser->status = currentStatus->description;
    Globals::getInstance().setUserStatus(_localUser->status);

    sendMessage(MT_Status, QString(), status);
  }
}

void lmcMainWindow::avatarAction_triggered() { setAvatar(); }

void lmcMainWindow::avatarBrowseAction_triggered() {
  QString dir = Globals::getInstance().fileOpenPath();
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Select avatar picture"), dir,
      "Images (*.bmp *.gif *.jpg *.jpeg *.png *.tif *.tiff)");
  if (!fileName.isEmpty()) {
    Globals::getInstance().setFileOpenPath(QFileInfo(fileName).dir().absolutePath());
    setAvatar(fileName);
  }
}

void lmcMainWindow::buttonStartChat_clicked() { startChatRoom(); }

void lmcMainWindow::buttonStartGroupChat_clicked() { startChatRoom(true); }

void lmcMainWindow::buttonStartPublicChat_clicked() { emit showPublicChat(); }

void lmcMainWindow::refreshAction_triggered() {
  QString szUserId;
  QString szMessage;

  sendMessage(MT_Refresh, szUserId, szMessage);
}

void lmcMainWindow::helpAction_triggered() {
  QRect rect = geometry();
  emit showHelp(rect);
}

void lmcMainWindow::homePageAction_triggered() {
  QDesktopServices::openUrl(QUrl(IDA_DOMAIN));
}

void lmcMainWindow::updateAction_triggered() {
  QRect rect = geometry();
  emit showUpdate(rect);
}

void
lmcMainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
  case QSystemTrayIcon::Trigger:
    if (Globals::getInstance().singleClickTray())
      processTrayIconTrigger();
    break;
  case QSystemTrayIcon::DoubleClick:
    if (!Globals::getInstance().singleClickTray())
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

void lmcMainWindow::treeWidgetUserList_itemActivated(QTreeWidgetItem *item,
                                                     int column) {
    Q_UNUSED(column);
    if (item->data(0, TypeRole).toString().compare("User") == 0) {
        if (Globals::getInstance().defaultNewMessageAction() == 1)
            startChatRoom();
        else { // if (Globals::getInstance().defaultNewMessageAction() == 2)
            emit sendInstantMessage(item->data(0, IdRole).toString());
        }
    }
}

void lmcMainWindow::treeWidgetUserList_itemContextMenu(QTreeWidgetItem *pItem,
                                                       QPoint pos) {
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
    sendMessage(MT_Group, szUserId, szMessage);
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
    _buttonSendFile->setEnabled(ui.treeWidgetUserList->selectedItems().length());
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
    sendMessage(MT_Group, szUserId, szMessage);
  }
  pDefGroupItem->sortChildren(0, Qt::AscendingOrder);

  emit groupUpdated(GO_Delete, groupId, QVariant());
  ui.treeWidgetUserList->takeTopLevelItem(
      ui.treeWidgetUserList->indexOfTopLevelItem(pGroupItem));
}

void lmcMainWindow::userConversationAction_triggered() { startChatRoom(); }

void lmcMainWindow::userMessageAction_triggered() {
    QTreeWidgetItem *currentItem = ui.treeWidgetUserList->currentItem();
    if (currentItem && !currentItem->data(0, TypeRole).toString().compare("User"))
        emit sendInstantMessage(currentItem->data(0, IdRole).toString());
}

void lmcMainWindow::buttonSendBroadcast_clicked() { emit showBroadcast(); }

void lmcMainWindow::actionSendBroadcast_triggered() {
  emit showBroadcast();
}

void lmcMainWindow::buttonSendFile_clicked() { sendFile(); }

void lmcMainWindow::sendFile(bool sendFolder, QTreeWidgetItem *dropTarget,
                             QStringList files) {
  QString dir = Globals::getInstance().fileOpenPath();
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
   Globals::getInstance().setFileOpenPath(QFileInfo(fileNames[0]).dir().absolutePath());

  QList<QTreeWidgetItem *> selectedItems = getSelectedUserTreeItems(dropTarget);

  MessageType messageType = sendFolder ? MT_Folder : MT_File;
  QFileInfo fileInfo;

  QString chatRoomId = _localUser->id;
  chatRoomId.append(Helper::getUuid());

  for (QTreeWidgetItem *item : selectedItems) {
    QString userId = item->data(0, IdRole).toString();

    for (QString &fileName : fileNames) {
      if (filedDragDropped) {
        fileInfo.setFile(fileName);
        messageType = fileInfo.isDir() ? MT_Folder : MT_File;
      }

      sendMessage(messageType, userId, fileName, chatRoomId);
    }
  }
}

void lmcMainWindow::startChatRoom(bool fromToolbar) {
    QString chatRoomId = _localUser->id;
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
  sendMessage(MT_Query, userId, message);
}

void lmcMainWindow::userSendScreenshotAction_triggered() {
    if (!_clipboard->mimeData()->hasImage())
        return;

    QPixmap imageData = qvariant_cast<QPixmap>(_clipboard->mimeData()->imageData());
    QString screenshotPath = QString("%1screenshot - %2.png").arg(StdLocation::getCacheDir(), _localUser->name);
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
  Globals::getInstance().setUserNote(note);
  _localUser->note = note;
  sendMessage(MT_Note, QString(), note);
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
  trayStatusAction = pTrayMenu->addMenu(_statusMenu);
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
  _trayIcon = new QSystemTrayIcon(this);
  _trayIcon->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon("logosmall")));
  _trayIcon->setContextMenu(pTrayMenu);

  connect(_trayIcon, &QSystemTrayIcon::activated, this,
          &lmcMainWindow::trayIcon_activated);
  connect(_trayIcon, &QSystemTrayIcon::messageClicked, this,
          &lmcMainWindow::trayMessage_clicked);
}

void lmcMainWindow::changeTrayIcon(QString icon) {
  _trayIcon->setIcon(
      QIcon(icon));
}

void lmcMainWindow::createStatusMenu() {
  _statusMenu = new QMenu(this);
  _statusGroup = new QActionGroup(this);
  connect(_statusGroup, &QActionGroup::triggered, this,
          &lmcMainWindow::statusAction_triggered);

  std::vector<StatusStruct> statuses = Globals::getInstance().getStatuses();
  for (unsigned index = 0; index < statuses.size(); index++) {
    QAction *pAction = new QAction(
        QIcon(ThemeManager::getInstance().getAppIcon(
            statuses[index].icon)),
        statuses[index].uiDescription, this);
    pAction->setData(statuses[index].description);
    pAction->setCheckable(true);
    _statusGroup->addAction(pAction);
    _statusMenu->addAction(pAction);
  }

  _buttonStatus->setMenu(_statusMenu);
}

void lmcMainWindow::createAvatarMenu() {
  if (pAvatarMenu) {
    pAvatarMenu->clear();
    pAvatarMenu->deleteLater();
  }
  pAvatarMenu = new QMenu(this);

  lmcImagePickerAction *pAction = new lmcImagePickerAction(
      pAvatarMenu, ImagesList::getInstance().getAvatars(),
      ImagesList::getInstance().getTabs(ImagesList::Avatars), 64, 64, 6,
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
  _userMessageAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("chat")),
      "Send &Message", this, SLOT(userMessageAction_triggered()));
  _userFileAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfile")),
      "Send &File", this, SLOT(buttonSendFile_clicked()));
  _userFolderAction = pUserMenu->addAction(
      QIcon(
          ThemeManager::getInstance().getAppIcon("sendfolder")),
      "Send Fol&der", this, SLOT(userFolderAction_triggered()));
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

QFrame *lmcMainWindow::createSeparator(QWidget *parent) {
    QFrame *separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(2);
    separator->setFixedHeight(30);

    return separator;
}

void lmcMainWindow::createToolBar() {
   ui.widgetToolBarContainer->setProperty("isToolbar", true);

  _buttonStatus = new ThemedButton(ui.widgetStatus);
  _buttonStatus->setPopupMode(QToolButton::InstantPopup);
  _buttonStatus->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonStatus->setIconSize(QSize(24, 24));
  _buttonStatus->setAutoRaise(true);
  ui.widgetstatus_layout->insertWidget(0, _buttonStatus);

  _buttonStartChat = new ThemedButton(ui.widgetToolBar);
  _buttonStartChat->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonStartChat->setAutoRaise(true);
  _buttonStartChat->setIconSize(QSize(17, 17));
  _buttonStartChat->setFixedWidth(45);
  _buttonStartChat->setIcon(QIcon(
      ThemeManager::getInstance().getAppIcon(QStringLiteral("chatroom"))));

  connect(_buttonStartChat, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonStartChat_clicked);

  _buttonStartGroupChat = new ThemedButton(ui.widgetToolBar);
  _buttonStartGroupChat->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonStartGroupChat->setAutoRaise(true);
  _buttonStartGroupChat->setIconSize(QSize(17, 17));
  _buttonStartGroupChat->setFixedWidth(45);
  _buttonStartGroupChat->setIcon(QIcon(
      ThemeManager::getInstance().getAppIcon(QStringLiteral("newchatroom"))));

  connect(_buttonStartGroupChat, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonStartGroupChat_clicked);

  _buttonSendFile = new ThemedButton(ui.widgetToolBar);
  _buttonSendFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonSendFile->setAutoRaise(true);
  _buttonSendFile->setIconSize(QSize(17, 17));
  _buttonSendFile->setFixedWidth(45);
  _buttonSendFile->setIcon(QIcon(
      ThemeManager::getInstance().getAppIcon(QStringLiteral("file"))));
  _buttonSendFile->setEnabled(false);

  connect(_buttonSendFile, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonSendFile_clicked);

  _buttonSendBroadcast = new ThemedButton(ui.widgetToolBar);
  _buttonSendBroadcast->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonSendBroadcast->setAutoRaise(true);
  _buttonSendBroadcast->setIconSize(QSize(17, 17));
  _buttonSendBroadcast->setFixedWidth(45);
  _buttonSendBroadcast->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon(
          QStringLiteral("broadcastmsg"))));

  connect(_buttonSendBroadcast, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonSendBroadcast_clicked);

  _buttonStartPublicChat = new ThemedButton(ui.widgetToolBar);
  _buttonStartPublicChat->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonStartPublicChat->setAutoRaise(true);
  _buttonStartPublicChat->setIconSize(QSize(17, 17));
  _buttonStartPublicChat->setFixedWidth(45);
  _buttonStartPublicChat->setIcon(
      QIcon(ThemeManager::getInstance().getAppIcon(
          QStringLiteral("chatroom"))));

  connect(_buttonStartPublicChat, &ThemedButton::clicked, this,
          &lmcMainWindow::buttonStartPublicChat_clicked);

  ui.widgetToolBar_layout->addWidget(_buttonStartChat);
  ui.widgetToolBar_layout->addWidget(_buttonStartGroupChat);
  ui.widgetToolBar_layout->addWidget(_buttonSendFile);
  ui.widgetToolBar_layout->addWidget(createSeparator(ui.widgetToolBar));
  ui.widgetToolBar_layout->addWidget(_buttonSendBroadcast);
  ui.widgetToolBar_layout->addWidget(createSeparator(ui.widgetToolBar));
  ui.widgetToolBar_layout->addWidget(_buttonStartPublicChat);
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
  _userChatAction->setText(tr("Start &Conversation"));
  _userBroadcastAction->setText(tr("Send &Broadcast Message"));
  _userFileAction->setText(tr("Send &File"));
  _userFolderAction->setText(tr("Send Fol&der"));
  _userInfoAction->setText(tr("Get &Information"));
  _avatarBrowseAction->setText(tr("&Browse for more pictures..."));
  _buttonStartChat->setText("Start &Conversation");
  _buttonStartChat->setToolTip("Start Conversation");
  _buttonStartGroupChat->setText("Start Group &Conversation");
  _buttonStartGroupChat->setToolTip("Start Group Conversation");
  _buttonSendFile->setText("Send &File");
  _buttonSendFile->setToolTip("Send File");
  _buttonSendBroadcast->setText("Send &Broadcast Message");
  _buttonSendBroadcast->setToolTip("Send Broadcast Message");
  _buttonStartPublicChat->setText("&Public Chat");
  _buttonStartPublicChat->setToolTip("Public Chat");

  auto statuses = Globals::getInstance().getStatuses();
  for (int index = 0; index < _statusGroup->actions().count(); index++)
    _statusGroup->actions()[index]->setText(statuses[index].uiDescription);

  ui.labelUserName->setText(_localUser->name); // in case of retranslation
  if (_statusGroup->checkedAction())
    ui.labelStatus->setText(_statusGroup->checkedAction()->text());

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

void lmcMainWindow::initGroups(const QList<Group> &groupList) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcMainWindow.initGroups started"));

  for (int index = 0; index < groupList.count(); index++) {
    lmcUserTreeWidgetGroupItem *item = new lmcUserTreeWidgetGroupItem();
    item->setData(0, IdRole, groupList.value(index).id);
    item->setData(0, TypeRole, "Group");
    item->setText(0, groupList.value(index).name);
    item->setSizeHint(0, QSize(0, 22));
    ui.treeWidgetUserList->addTopLevelItem(item);
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
  _localUser->avatar = nAvatar;
  Globals::getInstance().setUserAvatarIndex(nAvatar);
  _localUser->avatarPath = avatarPath;
  sendAvatar(QString::null);
}

lmcUserTreeWidgetItem *lmcMainWindow::getUserItem(const QString &userId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    for (int index = 0;
         index < ui.treeWidgetUserList->topLevelItem(topIndex)->childCount();
         index++) {
      QTreeWidgetItem *item =
          ui.treeWidgetUserList->topLevelItem(topIndex)->child(index);
      if (item->data(0, IdRole).toString().compare(userId) == 0)
        return static_cast<lmcUserTreeWidgetItem *> (item);
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

void lmcMainWindow::sendMessage(MessageType type, const QString &userId,
                                const QString &message, const QString &chatRoomId) {
  XmlMessage xmlMessage;

  switch (type) {
  case MT_Status:
    xmlMessage.addData(XN_STATUS, message);
    break;
  case MT_Note:
    xmlMessage.addData(XN_NOTE, message);
    break;
  case MT_Refresh:
    break;
  case MT_Group:
    xmlMessage.addData(XN_GROUP, message);
    break;
  case MT_File:
  case MT_Folder:
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
    xmlMessage.addData(XN_FILEPATH, message);
    xmlMessage.addData(XN_THREAD, chatRoomId);
    break;
  case MT_Avatar:
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Avatar]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
    xmlMessage.addData(XN_FILEPATH, message);
    break;
  case MT_Query:
    xmlMessage.addData(XN_QUERYOP, QueryOpNames[QO_Get]);
    break;
  default:
    break;
  }

  sendMessage(type, userId, xmlMessage);
}

void lmcMainWindow::sendAvatar(const QString &userId) {
  QString filePath = ImagesList::getInstance().getAvatar(_localUser->avatar);
  if (!QFile::exists(filePath))
    return;

  sendMessage(MT_Avatar, userId, filePath);
}

void lmcMainWindow::setUserAvatar(const QString &userId, const QString &filePath) {
  lmcUserTreeWidgetItem *pUserItem = getUserItem(userId);
  if (!pUserItem)
    return;

  QPixmap avatar;
  if (filePath.isEmpty() || !QFile::exists(filePath))
    avatar.load(ImagesList::getInstance().getDefaultAvatar());
  else
    avatar.load(filePath);

  if (!avatar.isNull()) {
      avatar = avatar.scaled(QSize(32, 32), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
      pUserItem->setData(0, AvatarRole, QIcon(avatar));
  }
}

void lmcMainWindow::processTrayIconTrigger() {
  // If system tray minimize is disabled, restore() will be called every time.
  // Otherwise, window is restored or minimized
  if (!Globals::getInstance().sysTrayMinimize() || isHidden() || isMinimized())
    restore();
  else
    minimize();
}

void lmcMainWindow::setTrayTooltip() {
  if (bConnected)
    _trayIcon->setToolTip(lmcStrings::appName());
  else {
    QString msg = tr("%1 - Not Connected");
    _trayIcon->setToolTip(msg.arg(lmcStrings::appName()));
  }
}
