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

#include "chatroomwindow.h"
#include "loggermanager.h"
#include "imageslist.h"
#include "globals.h"

#include <QMimeData>
#include <QShortcut>
#include <QTimer>

const qint64 pauseTime = 5000;

QString GroupId = "PARTICIPANTS";

lmcChatRoomWindow::lmcChatRoomWindow(QWidget *parent, bool isPublicChat)
    : QWidget(parent), _isPublicChat(isPublicChat) {
  ui.setupUi(this);
  setAcceptDrops(true);
  setProperty("isWindow", true);

  ui.buttonLeave->setVisible(false);

  ui.textBoxMessage->setProperty("light", true);

  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemActivated, this,
          &lmcChatRoomWindow::treeWidgetUserList_itemActivated);
  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemContextMenu, this,
          &lmcChatRoomWindow::treeWidgetUserList_itemContextMenu);
  connect(ui.buttonAddUsers, &ThemedButton::clicked, this,
          &lmcChatRoomWindow::buttonAddUsers_clicked);
  connect(ui.buttonLeave, &ThemedButton::clicked, this,
          &lmcChatRoomWindow::buttonLeaveChat_clicked);

  pMessageLog = new lmcMessageLog(ui.wgtLog);
  ui.logLayout->addWidget(pMessageLog);
  pMessageLog->setAcceptDrops(false);
  connect(pMessageLog,
          &lmcMessageLog::messageSent, this,
          &lmcChatRoomWindow::log_sendMessage);

  int bottomPanelHeight = ui.textBoxMessage->minimumHeight() +
                          ui.labelDividerBottom->minimumHeight() +
                          ui.labelDividerTop->minimumHeight() +
                          ui.widgetToolBar->minimumHeight();
  QList<int> sizes;
  sizes.append(height() - bottomPanelHeight - ui.hSplitter->handleWidth());
  sizes.append(bottomPanelHeight);
  ui.hSplitter->setSizes(sizes);
  ui.hSplitter->setStyleSheet(QString("QSplitter::handle { image: url(%1); }").arg(ThemeManager::getInstance().getAppIcon(QStringLiteral("vgrip"))));
  sizes.clear();
  sizes.append(width() * 0.7);
  sizes.append(width() - width() * 0.7 - ui.vSplitter->handleWidth());
  ui.vSplitter->setSizes(sizes);
  ui.vSplitter->setStyleSheet(QString("QSplitter::handle { image: url(%1); }").arg(ThemeManager::getInstance().getAppIcon(QStringLiteral("hgrip"))));

  ui.labelInfo->setBackgroundRole(QPalette::Base);
  ui.labelInfo->setAutoFillBackground(true);
  ui.labelInfo->setVisible(false);
  pMessageLog->installEventFilter(this);
  ui.treeWidgetUserList->installEventFilter(this);
  ui.textBoxMessage->installEventFilter(this);
  infoFlag = IT_Ok;
  dataSaved = false;
  windowLoaded = false;

  localId = QString::null;
  localName = QString::null;

  chatState = CS_Blank;
  keyStroke = 0;

  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this, SLOT(buttonHistory_clicked()));
}

lmcChatRoomWindow::~lmcChatRoomWindow() {
    pSmileyMenu->clear();
    delete pSmileyMenu;

    pEmojiMenu->clear();
    delete pEmojiMenu;

    pUserMenu->clear();
    delete pUserMenu;

    delete pSoundPlayer;
    delete pSettings;
}

void lmcChatRoomWindow::init(User *pLocalUser, bool connected, const QString &thread) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcChatRoomWindow.init started"));

  localId = pLocalUser->id;
  localName = pLocalUser->name;

  this->pLocalUser = pLocalUser;

  pMessageLog->localId = localId;
  pMessageLog->savePath = QDir(StdLocation::getWritableCacheDir()).absoluteFilePath(QString("msg_%1.tmp").arg(thread));

  //	get the avatar image for the user
  pMessageLog->participantAvatars.insert(localId, pLocalUser->avatarPath);

  _chatRoomId = thread;
  _originalChatRoomId = thread;

  createUserMenu();
  createSmileyMenu();
  createEmojiMenu();
  createToolBar();

  bConnected = connected;
  if (!bConnected)
    showStatus(IT_Disconnected, true);

  pSoundPlayer = new lmcSoundPlayer();

  ui.treeWidgetUserList->setIconSize(QSize(16, 16));
  ui.treeWidgetUserList->header()->setDragEnabled(false);
  ui.treeWidgetUserList->header()->setStretchLastSection(false);
  ui.treeWidgetUserList->header()->setSectionResizeMode(0,
                                                        QHeaderView::Stretch);

  lmcUserTreeWidgetGroupItem *pItem = new lmcUserTreeWidgetGroupItem();
  pItem->setData(0, IdRole, GroupId);
  pItem->setData(0, TypeRole, "Group");
  pItem->setText(0, "Participants");
  pItem->setSizeHint(0, QSize(0, 22));
  ui.treeWidgetUserList->addTopLevelItem(pItem);
  ui.treeWidgetUserList->expandAll();

  pSettings = new lmcSettings();
  showSmiley = pSettings->value(IDS_EMOTICON, IDS_EMOTICON_VAL).toBool();
  pMessageLog->showSmiley = showSmiley;
  pMessageLog->autoFile =
      pSettings->value(IDS_AUTOFILE, IDS_AUTOFILE_VAL).toBool();
  pMessageLog->messageTime =
      pSettings->value(IDS_MESSAGETIME, IDS_MESSAGETIME_VAL).toBool();
  pMessageLog->messageDate =
      pSettings->value(IDS_MESSAGEDATE, IDS_MESSAGEDATE_VAL).toBool();
  pMessageLog->allowLinks =
      pSettings->value(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL).toBool();
  pMessageLog->pathToLink =
      pSettings->value(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL).toBool();
  pMessageLog->trimMessage =
      pSettings->value(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL).toBool();
  pMessageLog->overrideIncomingStyle = pSettings->value(IDS_OVERRIDEINMSG, IDS_OVERRIDEINMSG_VAL).toBool();
  pMessageLog->defaultFont.fromString(pSettings->value(IDS_FONT, IDS_FONT_VAL).toString());
  pMessageLog->defaultColor = pSettings->value(IDS_COLOR, IDS_COLOR_VAL).toString();

  QFont font;
  font.fromString(pSettings->value(IDS_FONT, IDS_FONT_VAL).toString());
  messageColor = QApplication::palette().text().color();
  messageColor.setNamedColor(
      pSettings->value(IDS_COLOR, IDS_COLOR_VAL).toString());
  sendKeyMod = pSettings->value(IDS_SENDKEYMOD, IDS_SENDKEYMOD_VAL).toBool();

  if (_isPublicChat) {
    restoreGeometry(pSettings->value(IDS_WINDOWPUBLICCHAT).toByteArray());
    ui.vSplitter->restoreState(
        pSettings->value(IDS_SPLITTERPUBLICCHATV).toByteArray());
    ui.hSplitter->restoreState(
        pSettings->value(IDS_SPLITTERPUBLICCHATH).toByteArray());
  } else {
      restoreGeometry(pSettings->value(IDS_WINDOWCHATROOM).toByteArray());
      ui.vSplitter->restoreState(
          pSettings->value(IDS_SPLITTERCHATROOMV).toByteArray());
      ui.hSplitter->restoreState(
          pSettings->value(IDS_SPLITTERCHATROOMH).toByteArray());
  }

  setUIText();

  setMessageFont(font);
  ui.textBoxMessage->setStyleSheet(QString("QTextEdit { color: %1; }").arg(messageColor.name()));
  ui.textBoxMessage->setFocus();

  int viewType =
      pSettings->value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt();
  ui.treeWidgetUserList->setView((UserListView)viewType);

  appendHistory =
      pSettings->value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool();

  pMessageLog->initMessageLog();

  ThemeManager::getInstance().reloadStyleSheet();

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcChatRoomWindow.init ended"));
}

void lmcChatRoomWindow::show() {
  windowLoaded = true;
  QWidget::show();
}

void lmcChatRoomWindow::stop() {
  if (_isPublicChat && windowLoaded) {
    pSettings->setValue(IDS_WINDOWPUBLICCHAT, saveGeometry());
    pSettings->setValue(IDS_SPLITTERPUBLICCHATV, ui.vSplitter->saveState());
    pSettings->setValue(IDS_SPLITTERPUBLICCHATH, ui.hSplitter->saveState());
  } else {
      pSettings->setValue(IDS_WINDOWCHATROOM, saveGeometry());
      pSettings->setValue(IDS_SPLITTERCHATROOMV, ui.vSplitter->saveState());
      pSettings->setValue(IDS_SPLITTERCHATROOMH, ui.hSplitter->saveState());
  }
}

void lmcChatRoomWindow::addUser(User *pUser) {
  LoggerManager::getInstance().writeInfo(
      QString("lmcChatRoomWindow.addUser started -|- User: %1")
          .arg(pUser ? pUser->name : "no user"));
  if (!pUser || peerIds.contains(pUser->id))
    return;

  _lastUserId = QString::null;

  // Do not add user if user's version is 1.2.35 or less. These versions do not
  // support Public Chat feature.
  if (Helper::compareVersions(pUser->version, "1.2.35") <= 0)
    return;

  //	Do not add user if user is already in the list of participants
  if (peerIds.contains(pUser->id))
    return;

  peerIds.insert(pUser->id, pUser->id);
  _peerNames.insert(pUser->id, pUser->name);

  int index;
  StatusStruct *status =
      Globals::getInstance().getStatus(pUser->status, &index);

  lmcUserTreeWidgetUserItem *pItem = new lmcUserTreeWidgetUserItem();
  pItem->setData(0, IdRole, pUser->id);
  pItem->setData(0, TypeRole, "User");
  pItem->setData(0, StatusRole, index);
  pItem->setData(0, SubtextRole, pUser->note);
  pItem->setText(0, pUser->name);

  if (status)
    pItem->setIcon(0, QIcon(ThemeManager::getInstance().getAppIcon(status->icon)));

  lmcUserTreeWidgetGroupItem *pGroupItem =
      (lmcUserTreeWidgetGroupItem *)getGroupItem(&GroupId);
  pGroupItem->addChild(pItem);
  pGroupItem->sortChildren(0, Qt::AscendingOrder);

  // this should be called after item has been added to tree
  setUserAvatar(&pUser->id, &pUser->avatarPath);

  if (!_isPublicChat) {
    XmlMessage xmlMessage;
    xmlMessage.addData(XN_THREAD, _chatRoomId);
    xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Join]);
    xmlMessage.addData(XN_USERID, pUser->id);

    appendMessageLog(MT_Join, &pUser->id, &pUser->name, &xmlMessage);
    setWindowTitle(getWindowTitle());
    emit messageSent(MT_Message, nullptr, &xmlMessage);

    for (QString &userId : peerIds.keys())
        emit messageSent(MT_Message, &userId, &xmlMessage);
  }

  //	Local user cannot participate in public chat if status is offline
  if (_peerNames.size () <= 1 && !pUser->id.compare(localId)) {
    bool offline = Globals::getInstance().getStatusType(pUser->status) ==
                   StatusTypeEnum::StatusOffline;
    ui.textBoxMessage->setEnabled(!offline);
    ui.textBoxMessage->setFocus();
  }

  if (_peerNames.size() == 1 && pUser->id.compare(localId)) {
      StatusStruct *statusItem = Globals::getInstance().getStatus(pUser->status);
      if (statusItem) {
          if (statusItem->statusType == StatusTypeEnum::StatusOffline)
              showStatus(IT_Offline, false);
          else if (statusItem->statusType == StatusTypeEnum::StatusBusy)
              showStatus(IT_Busy, false);
          else if (statusItem->statusType == StatusTypeEnum::StatusAway)
              showStatus(IT_Away, false);
          else
              showStatus(IT_Ok, false);
      }
  } else if (_peerNames.size() == 2) {
      showStatus(IT_Ok, false);
  }

  if (!_isPublicChat && _peerNames.size() == 1) {
      toggleSideBar(false, false);

      _buttonHistory->setVisible(true);
      _buttonTransfer->setVisible(true);

      ui.buttonLeave->setVisible(false);
  } else if (_peerNames.size () == 2) {
      toggleSideBar(false, true);

      _buttonHistory->setVisible(false);
      _buttonTransfer->setVisible(false);

      if (!_isPublicChat)
          ui.buttonLeave->setVisible(true);
  }

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcChatRoomWindow.addUser ended"));
}

void lmcChatRoomWindow::updateUser(User *pUser) {
  if (!pUser)
    return;

  QTreeWidgetItem *pItem = getUserItem(&pUser->id);
  if (pItem) {
    int index;

   Globals::getInstance().getStatus(pUser->status, &index);

    updateStatusImage(pItem, &pUser->status);
    pItem->setData(0, StatusRole, index);
    pItem->setData(0, SubtextRole, pUser->note);
    pItem->setText(0, pUser->name);
    QTreeWidgetItem *pGroupItem = pItem->parent();
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
  }

  if (_peerNames.size () > 1)
    setWindowTitle(getWindowTitle());

  //	Local user cannot participate in public chat if status is offline
  if (_isPublicChat && pUser->id.compare(localId) == 0) {
    bool offline = Globals::getInstance().getStatusType(pUser->status) ==
                   StatusTypeEnum::StatusOffline;
    ui.textBoxMessage->setEnabled(!offline);
    ui.textBoxMessage->setFocus();
  }

  if (_peerNames.size () != 1) {
      _buttonHistory->setVisible (false);
      _buttonTransfer->setVisible (false);
  } else {
      _buttonHistory->setVisible (true);
      _buttonTransfer->setVisible (true);
  }
}

void lmcChatRoomWindow::removeUser(QString *lpszUserId) {
  QTreeWidgetItem *pItem = getUserItem(lpszUserId);
  if (!pItem)
    return;

  if (peerIds.size() == 1)
      _lastUserId = *lpszUserId; // last user can only disconnect by going offline
  else
      _lastUserId = QString::null;

  QTreeWidgetItem *pGroup = pItem->parent();
  pGroup->removeChild(pItem);

  QString userId = peerIds.value(*lpszUserId);
  QString userName = _peerNames.value(*lpszUserId);

  peerIds.remove(*lpszUserId);
  _peerNames.remove(*lpszUserId);

  XmlMessage xmlMessage;
  xmlMessage.addData(XN_THREAD, _chatRoomId);
  xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Leave]);

  appendMessageLog(MT_Leave, &userId, &userName, &xmlMessage);
  setWindowTitle(getWindowTitle());

  if (!_isPublicChat && _peerNames.size() == 1) {
      toggleSideBar(false, false);

      _buttonHistory->setVisible(true);
      _buttonTransfer->setVisible(true);

      ui.buttonLeave->setVisible(false);
  } else if (_peerNames.size () == 0) {
      toggleSideBar(false, true);

      _buttonHistory->setVisible(false);
      _buttonTransfer->setVisible(false);
  }

  // If the local user is removed for some reason, prevent sending any further
  // messages
  if (userId.compare(localId) == 0)
    ui.textBoxMessage->setEnabled(false);
}

void lmcChatRoomWindow::setHtml(const QString &html) {
    pMessageLog->prependHtml(html);
}

void lmcChatRoomWindow::receiveMessage(MessageType type, QString *lpszUserId,
                                       XmlMessage *pMessage) {
  QString title;

  //	if lpszUserId is NULL, the message was sent locally
  QString senderId = lpszUserId ? *lpszUserId : localId;
  QString senderName = lpszUserId ? _peerNames.value(senderId) : localName;
  QString data;

  StatusStruct *statusItem = Globals::getInstance().getStatus(pMessage->data(XN_STATUS));

  switch (type) {
  case MT_PublicMessage:
    appendMessageLog(type, lpszUserId, &senderName, pMessage);
    if (isVisible() && !isActiveWindow()) {
        pSoundPlayer->play(SE_NewPubMessage);
    }

    if (!isActiveWindow()) {
        if (pSettings->value(IDS_SYSTRAYPUBNEWMSG, IDS_SYSTRAYPUBNEWMSG_VAL).toBool())
            emit showTrayMessage(TM_Message, pMessage->data(XN_MESSAGE), _chatRoomId, QString("%1 messaged you").arg(senderName), TMI_Info);
    }
    break;
  case MT_Message:
  case MT_GroupMessage:
    appendMessageLog(type, lpszUserId, &senderName, pMessage);
    if (isHidden() || !isActiveWindow()) {
      pSoundPlayer->play(SE_NewMessage);
      title = tr("%1 says...");
      setWindowTitle(title.arg(senderName));

      if (!isActiveWindow()) {
          if (pSettings->value(IDS_SYSTRAYNEWMSG, IDS_SYSTRAYNEWMSG_VAL).toBool())
              emit showTrayMessage(TM_Message, pMessage->data(XN_MESSAGE), _chatRoomId, QString("%1 messaged you").arg(senderName), TMI_Info);
      }
    }
    break;
  case MT_Broadcast:
      appendMessageLog(type, lpszUserId, &senderName, pMessage);
      if(isHidden() || !isActiveWindow()) {
          pSoundPlayer->play(SE_NewMessage);
          title = tr("Broadcast from %1");
          setWindowTitle(title.arg(senderName));
      }

      if (!isActiveWindow()) {
          if (pSettings->value(IDS_SYSTRAYNEWMSG, IDS_SYSTRAYNEWMSG_VAL).toBool())
              emit showTrayMessage(TM_Message, pMessage->data(XN_BROADCAST), _chatRoomId, QString("Received broadcast from %1").arg(senderName), TMI_Info);
      }
      break;
  case MT_ChatState:
    appendMessageLog(type, lpszUserId, &senderName, pMessage);
    break;
  case MT_Status:
  {
    data = pMessage->data(XN_STATUS);
    bool isLocalUser = (pLocalUser->id == senderId);
    if (statusItem) {
      setWindowIcon(
          QIcon(ThemeManager::getInstance().getAppIcon(statusItem->icon)));
      if (statusItem->statusType == StatusTypeEnum::StatusOffline)
          showStatus(IT_Offline, isLocalUser);
      else if (statusItem->statusType == StatusTypeEnum::StatusBusy)
          showStatus(IT_Busy, isLocalUser);
      else if (statusItem->statusType == StatusTypeEnum::StatusAway)
          showStatus(IT_Away, isLocalUser);
      else
          showStatus(IT_Ok, isLocalUser);
    }
  }
    break;
  case MT_Avatar:
    data = pMessage->data(XN_FILEPATH);
    // this message may come with or without user id. NULL user id means avatar
    // change by local user, while non NULL user id means avatar change by a peer.
    setUserAvatar(&senderId, &data);
    break;
  case MT_UserName:
    data = pMessage->data(XN_NAME);
    if (_peerNames.contains(senderId))
      _peerNames[senderId] = data; // TODO should this be insert ?

    pMessageLog->updateUserName(&senderId, &data);
    break;
  case MT_File:
  case MT_Folder:
      if(pMessage->data(XN_FILEOP) == FileOpNames[FO_Request]) {
          //	a file request has been received
          appendMessageLog(type, lpszUserId, &senderName, pMessage);
          if(pMessage->data(XN_MODE) == FileModeNames[FM_Receive] && (isHidden() || !isActiveWindow())) {
              pSoundPlayer->play(SE_NewFile);
              if(type == MT_File)
                  title = tr("%1 sends a file...");
              else
                  title = tr("%1 sends a folder...");
              setWindowTitle(title.arg(senderName));
          }
      } else {
          // a file message of op other than request has been received
          processFileOp(pMessage);
      }
      break;
  case MT_Failed:
    appendMessageLog(type, lpszUserId, &senderName, pMessage);
    break;
  default:
    break;
  }
}

void lmcChatRoomWindow::connectionStateChanged(bool connected) {
  bConnected = connected;
  if (!bConnected)
      showStatus(IT_Disconnected,true);
}

void lmcChatRoomWindow::settingsChanged() {
  showSmiley = pSettings->value(IDS_EMOTICON, IDS_EMOTICON_VAL).toBool();
  pMessageLog->showSmiley = showSmiley;
  pMessageLog->autoFile =
      pSettings->value(IDS_AUTOFILE, IDS_AUTOFILE_VAL).toBool();
  pMessageLog->overrideIncomingStyle = pSettings->value(IDS_OVERRIDEINMSG, IDS_OVERRIDEINMSG_VAL).toBool();
  pMessageLog->defaultFont.fromString(pSettings->value(IDS_FONT, IDS_FONT_VAL).toString());
  pMessageLog->defaultColor = pSettings->value(IDS_COLOR, IDS_COLOR_VAL).toString();
  sendKeyMod = pSettings->value(IDS_SENDKEYMOD, IDS_SENDKEYMOD_VAL).toBool();
  appendHistory =
      pSettings->value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool();
  pSoundPlayer->settingsChanged();
  if (localName.compare(pLocalUser->name) != 0) {
    localName = pLocalUser->name;
    updateUser(pLocalUser);
    pMessageLog->updateUserName(&localId, &localName);
  }

  bool msgTime =
      pSettings->value(IDS_MESSAGETIME, IDS_MESSAGETIME_VAL).toBool();
  bool msgDate =
      pSettings->value(IDS_MESSAGEDATE, IDS_MESSAGEDATE_VAL).toBool();
  bool allowLinks =
      pSettings->value(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL).toBool();
  bool pathToLink =
      pSettings->value(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL).toBool();
  bool trim = pSettings->value(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL).toBool();
  QString theme = pSettings->value(IDS_THEME, IDS_THEME_VAL).toString();
  if (msgTime != pMessageLog->messageTime ||
      msgDate != pMessageLog->messageDate ||
      allowLinks != pMessageLog->allowLinks ||
      pathToLink != pMessageLog->pathToLink ||
      trim != pMessageLog->trimMessage ||
      theme.compare(pMessageLog->themePath) != 0) {
    pMessageLog->messageTime = msgTime;
    pMessageLog->messageDate = msgDate;
    pMessageLog->allowLinks = allowLinks;
    pMessageLog->pathToLink = pathToLink;
    pMessageLog->trimMessage = trim;
    pMessageLog->themePath = theme;
    pMessageLog->reloadMessageLog();
  }

  int viewType =
      pSettings->value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt();
  ui.treeWidgetUserList->setView((UserListView)viewType);
}

void lmcChatRoomWindow::selectContacts(const std::vector<User *> selectedContacts) {
    for (User *user : selectedContacts) {
        addUser(user);
    }
}

bool lmcChatRoomWindow::eventFilter(QObject *pObject, QEvent *pEvent) {
  if (pEvent->type() == QEvent::KeyPress) {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);
    if (pObject == ui.textBoxMessage) {
      if (pKeyEvent->key() == Qt::Key_Return ||
          pKeyEvent->key() == Qt::Key_Enter) {
        bool keyMod = ((pKeyEvent->modifiers() & Qt::ControlModifier) ==
                       Qt::ControlModifier);
        if (keyMod == sendKeyMod) {
          sendMessage();
          setChatState(CS_Active);
          return true;
        }
        // The TextEdit widget does not insert new line when Ctrl+Enter is
        // pressed
        // So we insert a new line manually
        if (keyMod)
          ui.textBoxMessage->insertPlainText("\n");
      } else if (pKeyEvent->key() == Qt::Key_Escape) {
        close();
        return true;
      }
      keyStroke++;
      setChatState(CS_Composing);
    } else {
      if (pKeyEvent->key() == Qt::Key_Escape) {
        close();
        emit closed(&_originalChatRoomId);
        return true;
      }
    }
  }

  return false;
}

void lmcChatRoomWindow::changeEvent(QEvent *pEvent) {
  switch (pEvent->type()) {
  case QEvent::ActivationChange:
    if (isActiveWindow()) {
      setWindowTitle(getWindowTitle());
      ui.textBoxMessage->setFocus();
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

void lmcChatRoomWindow::closeEvent(QCloseEvent *event) {
    setChatState(CS_Inactive);
    if (!_isPublicChat && _peerNames.size () > 1 && !_leaveChatTriggered) {
        if (windowState() != Qt::WindowMinimized) {
            setWindowState(windowState() | Qt::WindowMinimized);
            event->ignore();
            return;
        } else {
            buttonLeaveChat_clicked();
            return;
        }
    }

    // call stop procedure to save history
    stop();
    emit closed(&_chatRoomId);

    QWidget::closeEvent(event);
}

void lmcChatRoomWindow::dragEnterEvent(QDragEnterEvent *pEvent)
{
    if (pEvent->mimeData()->hasFormat("text/uri-list")) {
      QList<QUrl> urls = pEvent->mimeData()->urls();
      if (urls.isEmpty())
        return;

      QString fileName = urls.first().toLocalFile();
      if (fileName.isEmpty())
        return;

      pEvent->acceptProposedAction();
    }
}

void lmcChatRoomWindow::dropEvent(QDropEvent *pEvent)
{
    QList<QUrl> urls = pEvent->mimeData()->urls();
    if (urls.isEmpty())
      return;

    foreach (QUrl url, urls) {
      QString path = url.toLocalFile();
      if (path.isEmpty())
        continue;

      QFileInfo fileInfo(path);
      if (!fileInfo.exists())
        continue;

      for (const QString &userId : _peerNames.keys ())
          sendObject(userId, fileInfo.isDir() ? MT_Folder : MT_File, &path);
    }
}

void lmcChatRoomWindow::userConversationAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();

  QString threadId = Helper::getUuid();
  threadId.prepend(pLocalUser->id);

  emit chatStarting(threadId, XmlMessage(), QStringList() << userId);
}

void lmcChatRoomWindow::userFileAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  QString dir = pSettings->value(IDS_OPENPATH, StdLocation::getDocumentsPath())
                    .toString();
  QString fileName = QFileDialog::getOpenFileName(this, QString(), dir);
  if (!fileName.isEmpty()) {
      pSettings->setValue(IDS_OPENPATH, QFileInfo(fileName).dir().absolutePath());

      QString chatRoomId = pLocalUser->id;
      chatRoomId.append(Helper::getUuid());

      sendObject(userId, MT_File, &fileName, chatRoomId);
  }
}

void lmcChatRoomWindow::userFolderAction_triggered() {
  QString userId = ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  QString dir = pSettings->value(IDS_OPENPATH, StdLocation::getDocumentsPath())
                    .toString();
  QString fileName = QFileDialog::getExistingDirectory(
              this, QString(), dir, QFileDialog::ShowDirsOnly);
  if (!fileName.isEmpty()) {
      QString chatRoomId = pLocalUser->id;
      chatRoomId.append(Helper::getUuid());

      sendObject(userId, MT_Folder, &fileName, chatRoomId);
  }
}

void lmcChatRoomWindow::userInfoAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  XmlMessage xmlMessage;
  xmlMessage.addData(XN_QUERYOP, QueryOpNames[QO_Get]);
  emit messageSent(MT_Query, &userId, &xmlMessage);
}

void lmcChatRoomWindow::buttonFont_clicked() {
  bool ok;
  QFont font = ui.textBoxMessage->font();
  font.setPointSize(ui.textBoxMessage->fontPointSize());
  QFont newFont = QFontDialog::getFont(&ok, font, this, tr("Select Font"));
  if (ok)
    setMessageFont(newFont);
}

void lmcChatRoomWindow::buttonFontColor_clicked() {
  QColor color = QColorDialog::getColor(messageColor, this, tr("Select Color"));
  if (color.isValid()) {
    messageColor = color;
    ui.textBoxMessage->setStyleSheet("QTextEdit {color: " +
                                     messageColor.name() + ";}");
  }
}

void lmcChatRoomWindow::buttonFile_clicked() { sendFile(); }

void lmcChatRoomWindow::buttonFolder_clicked() { sendFile(true); }

void lmcChatRoomWindow::buttonSave_clicked() {
  QString dir = pSettings->value(IDS_SAVEPATH, StdLocation::getDocumentsPath())
                    .toString();
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save Conversation"), dir,
                                   "HTML File (*.html);;Text File (*.txt)");
  if (!fileName.isEmpty()) {
    pSettings->setValue(IDS_SAVEPATH, QFileInfo(fileName).dir().absolutePath());
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    if (fileName.endsWith(".html", Qt::CaseInsensitive))
      stream << pMessageLog->prepareMessageLogForSave(HtmlFormat);
    else
      stream << pMessageLog->prepareMessageLogForSave(TextFormat);
    file.close();
  }
}

void lmcChatRoomWindow::buttonSendClipboard_clicked() {
  if (_clipboard->mimeData()->hasImage()) {
      QPixmap imageData = qvariant_cast<QPixmap>(_clipboard->mimeData()->imageData());
      QString screenshotPath = QString("%1screenshot - %2.png").arg(StdLocation::getCacheDir(), pLocalUser->name);
      imageData.save(screenshotPath);

      for (const QString &userId : _peerNames.keys())
          sendObject(userId, MT_File, &screenshotPath);
  } else if (_clipboard->mimeData()->hasUrls()) {
      QFileInfo fileInfo;

      for (QUrl url : _clipboard->mimeData()->urls()) {
          QString filename = url.path();
          filename.remove(0, 1);
          fileInfo.setFile(filename);
          MessageType messageType = fileInfo.isDir() ? MT_Folder : MT_File;

          for (const QString &userId : _peerNames.keys())
              sendObject(userId, messageType, &filename);
      }
  }
}

void lmcChatRoomWindow::buttonHistory_clicked() {
  if (_peerNames.size() == 1)
    emit showHistory(_peerNames.values().first());
}

void lmcChatRoomWindow::buttonTransfers_clicked() {
  if (_peerNames.size() == 1)
    emit showTransfers(_peerNames.values().first());
}

void lmcChatRoomWindow::buttonAddUsers_clicked() {
  QStringList excludeList;

  QHash<QString, QString>::const_iterator index = peerIds.constBegin();
  while (index != peerIds.constEnd()) {
    QString userId = index.value();
    excludeList.append(userId);
    index++;
  }

  emit contactsAdding(&excludeList);
}

void lmcChatRoomWindow::buttonLeaveChat_clicked() {
    XmlMessage xmlMessage;
    xmlMessage.addData(XN_THREAD, _chatRoomId);
    xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Leave]);

    for (QString &peerId : _peerNames.keys())
        emit messageSent(MT_GroupMessage, &peerId, &xmlMessage);

    _leaveChatTriggered = true;
    close();
}

void lmcChatRoomWindow::buttonToggleSideBar_clicked()
{
    toggleSideBar();
}

void lmcChatRoomWindow::smileyAction_triggered() {
  //	nSmiley contains index of smiley
  insertSmileyCode(ImagesList::getInstance().getSmileys()[nSmiley]);
}

void lmcChatRoomWindow::emojiAction_triggered() {
  insertSmileyCode(ImagesList::getInstance().getEmojis()[nEmoji]);
}

void lmcChatRoomWindow::insertSmileyCode(const ImagesStruct &smiley) {
    QString smileyCode = "%1";

    QString text = ui.textBoxMessage->toPlainText();
    QTextCursor cursor = ui.textBoxMessage->textCursor();
    if (!text.isEmpty()) {
        int cursorPos = cursor.anchor();
        if (cursorPos != 0 && text.at(cursorPos - 1) != ' ')
            smileyCode.prepend(' ');
    }

    bool moveCursor = true;
    if (text.length() <= cursor.position() || text.at(cursor.position()) != ' ') {
        moveCursor = false;
        smileyCode.append(' ');
    }

    ui.textBoxMessage->insertPlainText(smileyCode.arg(smiley.code));

    if (moveCursor) {
        cursor.movePosition(QTextCursor::Right); // Move cursor to the right by 1 unit
        ui.textBoxMessage->setTextCursor(cursor);
    }
    ui.textBoxMessage->setFocus();
}

void lmcChatRoomWindow::log_sendMessage(MessageType type, QString *lpszUserId,
                                        XmlMessage *pMessage) {
  Q_UNUSED(type);
  Q_UNUSED(lpszUserId);
  Q_UNUSED(pMessage);
    emit messageSent(type, lpszUserId, pMessage);
}

void lmcChatRoomWindow::treeWidgetUserList_itemActivated(QTreeWidgetItem *pItem,
                                                         int column) {
  Q_UNUSED(column);
  if (pItem->data(0, TypeRole).toString().compare("User") == 0) {
    QString szUserId = pItem->data(0, IdRole).toString();
    if (szUserId.compare(localId) != 0) {
        QString chatRoomId = pLocalUser->id;
        chatRoomId.append(Helper::getUuid());



        emit chatStarting(chatRoomId, XmlMessage(), QStringList() << szUserId);
    }
  }
}

void
lmcChatRoomWindow::treeWidgetUserList_itemContextMenu(QTreeWidgetItem *pItem,
                                                      QPoint &pos) {
  if (!pItem)
    return;

  if (pItem->data(0, TypeRole).toString().compare("User") == 0) {
    for (int index = 0; index < pUserMenu->actions().count(); index++)
      pUserMenu->actions()[index]->setData(pItem->data(0, IdRole));

    userChatAction->setVisible(_peerNames.size() != 1);

    pUserMenu->exec(pos);
  }
}

void lmcChatRoomWindow::checkChatState() {
  if (keyStroke > snapKeyStroke) {
    snapKeyStroke = keyStroke;
    QTimer::singleShot(pauseTime, this, SLOT(checkChatState()));
    return;
  }

  if (ui.textBoxMessage->document()->isEmpty())
    setChatState(CS_Active);
  else
    setChatState(CS_Paused);
}

void lmcChatRoomWindow::createUserMenu() {
  pUserMenu = new QMenu(this);
  userChatAction = pUserMenu->addAction(
      "&Conversation", this, SLOT(userConversationAction_triggered()));
  userFileAction = pUserMenu->addAction("Send &File", this,
                                        SLOT(userFileAction_triggered()));
  userFolderAction = pUserMenu->addAction("Send &Folder", this,
                                        SLOT(userFolderAction_triggered()));
  pUserMenu->addSeparator();
  userInfoAction = pUserMenu->addAction("Get &Information", this,
                                        SLOT(userInfoAction_triggered()));
}

void lmcChatRoomWindow::createSmileyMenu() {
  pSmileyMenu = new QMenu(this);

  pSmileyAction = new lmcImagePickerAction(
      pSmileyMenu, ImagesList::getInstance().getSmileys(),
      ImagesList::getInstance().getTabs(ImagesList::Smileys), 39, 39, 10,
      &nSmiley, true);
  connect(pSmileyAction, &lmcImagePickerAction::imageSelected, this,
          &lmcChatRoomWindow::smileyAction_triggered);

  pSmileyMenu->addAction(pSmileyAction);
}

void lmcChatRoomWindow::createEmojiMenu() {
  pEmojiMenu = new QMenu(this);

  pEmojiAction = new lmcImagePickerAction(
      pEmojiMenu, ImagesList::getInstance().getEmojis(),
      ImagesList::getInstance().getTabs(ImagesList::Emojis), 78, 39, 8, &nEmoji,
      true);
  connect(pEmojiAction, &lmcImagePickerAction::imageSelected, this,
          &lmcChatRoomWindow::emojiAction_triggered);

  pEmojiMenu->addAction(pEmojiAction);
}

QFrame *lmcChatRoomWindow::createSeparator(QWidget *parent) {
    QFrame *separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(2);
    separator->setFixedHeight(30);

    return separator;
}

void lmcChatRoomWindow::createToolBar() {
   ui.widgetToolBar->setProperty("isToolbar", true);

  ThemedButton *buttonFont = new ThemedButton(ui.widgetToolBar);
  buttonFont->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonFont->setAutoRaise(true);
  buttonFont->setIconSize(QSize(17, 17));
  buttonFont->setFixedWidth(32);
  buttonFont->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("font"))));

  connect (buttonFont, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonFont_clicked);

  ThemedButton *buttonFontColor = new ThemedButton(ui.widgetToolBar);
  buttonFontColor->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonFontColor->setAutoRaise(true);
  buttonFontColor->setIconSize(QSize(17, 17));
  buttonFontColor->setFixedWidth(32);
  buttonFontColor->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("fontcolor"))));

  connect (buttonFontColor, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonFontColor_clicked);

  ThemedButton *buttonSmiley = new ThemedButton(ui.widgetToolBar);
  buttonSmiley->setPopupMode(QToolButton::InstantPopup);
  buttonSmiley->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonSmiley->setAutoRaise(true);
  buttonSmiley->setIconSize(QSize(17, 17));
  buttonSmiley->setFixedWidth(32);
  buttonSmiley->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("smiley"))));
  buttonSmiley->setMenu(pSmileyMenu);

  ThemedButton *buttonEmoji = new ThemedButton(ui.widgetToolBar);
  buttonEmoji->setPopupMode(QToolButton::InstantPopup);
  buttonEmoji->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonEmoji->setAutoRaise(true);
  buttonEmoji->setIconSize(QSize(17, 17));
  buttonEmoji->setFixedWidth(32);
  buttonEmoji->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("emoji"))));
  buttonEmoji->setMenu(pEmojiMenu);

  ThemedButton *buttonSendFile = new ThemedButton(ui.widgetToolBar);
  buttonSendFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonSendFile->setAutoRaise(true);
  buttonSendFile->setIconSize(QSize(17, 17));
  buttonSendFile->setFixedWidth(32);
  buttonSendFile->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("file"))));

  connect (buttonSendFile, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonFile_clicked);

  ThemedButton *buttonSendFolder = new ThemedButton(ui.widgetToolBar);
  buttonSendFolder->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonSendFolder->setAutoRaise(true);
  buttonSendFolder->setIconSize(QSize(17, 17));
  buttonSendFolder->setFixedWidth(32);
  buttonSendFolder->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("sendfolder"))));

  connect (buttonSendFolder, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonFolder_clicked);

  _buttonSaveConversation = new ThemedButton(ui.widgetToolBar);
  _buttonSaveConversation->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonSaveConversation->setAutoRaise(true);
  _buttonSaveConversation->setIconSize(QSize(17, 17));
  _buttonSaveConversation->setFixedWidth(32);
  _buttonSaveConversation->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("save"))));
  _buttonSaveConversation->setShortcut(QKeySequence::Save);
  _buttonSaveConversation->setEnabled(false);

  connect (_buttonSaveConversation, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonSave_clicked);

  _buttonSendClipboard = new ThemedButton(ui.widgetToolBar);
  _buttonSendClipboard->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonSendClipboard->setAutoRaise(true);
  _buttonSendClipboard->setIconSize(QSize(17, 17));
  _buttonSendClipboard->setFixedWidth(32);
  setClibpoardIcon();

  connect (_buttonSendClipboard, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonSendClipboard_clicked);

  _buttonHistory = new ThemedButton(ui.widgetToolBar);
  _buttonHistory->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonHistory->setAutoRaise(true);
  _buttonHistory->setIconSize(QSize(17, 17));
  buttonFont->setFixedWidth(32);
  _buttonHistory->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("history"))));
  _buttonHistory->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));

  connect (_buttonHistory, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonHistory_clicked);

  _buttonTransfer = new ThemedButton(ui.widgetToolBar);
  _buttonTransfer->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonTransfer->setAutoRaise(true);
  _buttonTransfer->setIconSize(QSize(17, 17));
  _buttonTransfer->setFixedWidth(32);
  _buttonTransfer->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("transfer"))));
  _buttonTransfer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));

  connect (_buttonTransfer, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonTransfers_clicked);

  _buttonToggleSideBar = new ThemedButton(ui.widgetToolBar);
  _buttonToggleSideBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  _buttonToggleSideBar->setAutoRaise(true);
  _buttonToggleSideBar->setIconSize(QSize(17, 17));
  _buttonToggleSideBar->setFixedWidth(32);
  _buttonToggleSideBar->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("doubleRight"))));

  connect (_buttonToggleSideBar, &ThemedButton::clicked, this, &lmcChatRoomWindow::buttonToggleSideBar_clicked);

  ui.toolBarLayout->addWidget(buttonFont);
  ui.toolBarLayout->addWidget(buttonFontColor);
  ui.toolBarLayout->addWidget(createSeparator(ui.widgetToolBar));
  ui.toolBarLayout->addWidget(buttonSmiley);
  ui.toolBarLayout->addWidget(buttonEmoji);
  ui.toolBarLayout->addWidget(createSeparator(ui.widgetToolBar));
  ui.toolBarLayout->addWidget(buttonSendFile);
  ui.toolBarLayout->addWidget(buttonSendFolder);
  ui.toolBarLayout->addWidget(createSeparator(ui.widgetToolBar));
  ui.toolBarLayout->addWidget(_buttonSaveConversation);

  ui.toolBarLayout->addStretch();

  ui.toolBarLayout->addWidget(_buttonSendClipboard);
  ui.toolBarLayout->addWidget(_buttonTransfer);
  ui.toolBarLayout->addWidget(_buttonHistory);
  ui.toolBarLayout->addWidget(createSeparator(ui.widgetToolBar));
  ui.toolBarLayout->addWidget(_buttonToggleSideBar);

  buttonFont->setText(tr("Change Font..."));
  buttonFont->setToolTip(tr("Change message font"));
  buttonFontColor->setText(tr("Change Color..."));
  buttonFontColor->setToolTip(tr("Change message text color"));
  buttonSmiley->setText(tr("Insert Smiley"));
  buttonSmiley->setToolTip(tr("Insert a smiley into the message"));
  buttonEmoji->setText(tr("Insert Emoji"));
  buttonEmoji->setToolTip(tr("Insert an emoji into the message"));
  buttonSendFile->setText(tr("Send File"));
  buttonSendFile->setToolTip(tr("Send a file from the local file system"));
  buttonSendFolder->setText(tr("Send Folder"));
  buttonSendFolder->setToolTip(tr("Send a folder from the local file system"));
  _buttonSaveConversation->setText(tr("&Save As..."));
  _buttonSaveConversation->setToolTip(tr("Save this conversation"));
  _buttonHistory->setText(tr("Show &History..."));
  _buttonHistory->setToolTip(tr("Show the conversation history with this user"));
  _buttonTransfer->setText(tr("Show &Transfers..."));
  _buttonTransfer->setToolTip(tr("Show the file transfer history with this user"));
  _buttonToggleSideBar->setText(tr("Hide >>"));
  _buttonToggleSideBar->setToolTip(tr("Toggles the user sidebar visibility"));

  ui.labelDividerTop->setBackgroundRole(QPalette::Light);
  ui.labelDividerTop->setAutoFillBackground(true);
  ui.labelDividerBottom->setBackgroundRole(QPalette::Dark);
  ui.labelDividerBottom->setAutoFillBackground(true);

  if (_peerNames.size() != 1) {
    _buttonHistory->setVisible(false);
    _buttonTransfer->setVisible(false);
  }

  if (!_isPublicChat)
    ui.buttonAddUsers->setVisible(true);
  else {
    ui.buttonAddUsers->setVisible(false);
    _buttonSendClipboard->setVisible(false);
  }

  connect(_clipboard, &QClipboard::changed, this, &lmcChatRoomWindow::setClibpoardIcon);
}

void lmcChatRoomWindow::setUIText() {
  ui.retranslateUi(this);

  setWindowTitle(getWindowTitle());

  QTreeWidgetItem *pGroupItem = getGroupItem(&GroupId);
  pGroupItem->setText(0, tr("Participants"));

  userChatAction->setText(tr("&Conversation"));
  userFileAction->setText(tr("Send &File"));
  userInfoAction->setText(tr("Get &Information"));

  showStatus(IT_Ok, false); //	this will force the info label to retranslate
}

void lmcChatRoomWindow::sendMessage() {
  if (ui.textBoxMessage->document()->isEmpty())
    return;

  if (bConnected) {
    QString szMessage(ui.textBoxMessage->toPlainText());

    QFont font = ui.textBoxMessage->font();
    font.setPointSize(ui.textBoxMessage->fontPointSize());

    MessageType type = !_isPublicChat ? (_peerNames.size() <= 1 ? MT_Message : MT_GroupMessage) : MT_PublicMessage;
    XmlMessage xmlMessage;
    xmlMessage.addHeader(XN_TIME, QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    xmlMessage.addData(XN_FONT, font.toString());
    xmlMessage.addData(XN_COLOR, messageColor.name());
    xmlMessage.addData(XN_MESSAGE, szMessage);
    xmlMessage.addData(XN_THREAD, _chatRoomId);

    if (_peerNames.size () > 1) {
        xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Message]);
    }

    appendMessageLog(type, &localId, &localName, &xmlMessage);

    QHash<QString, QString>::const_iterator index = _peerNames.constBegin();
    while (index != _peerNames.constEnd()) {
        QString userId = index.key();
        emit messageSent(type, &userId, &xmlMessage);
        index++;
    }
  } else
    appendMessageLog(MT_Error, NULL, NULL, NULL);

  ui.textBoxMessage->clear();
  ui.textBoxMessage->setFocus();
}

void lmcChatRoomWindow::appendMessageLog(MessageType type, QString *lpszUserId,
                                         QString *lpszUserName,
                                         XmlMessage *pMessage) {
    pMessageLog->appendMessageLog(type, lpszUserId, lpszUserName, pMessage, false, _peerNames.count(), true, pLocalUser, _peerNames);

  if (!_buttonSaveConversation->isEnabled())
    _buttonSaveConversation->setEnabled(pMessageLog->hasData);
}

void lmcChatRoomWindow::showStatus(int flag, bool isLocalUser) {
  infoFlag = flag;
  bool groupMode = _peerNames.size() != 1;

  int relScrollPos =
      pMessageLog->page()->mainFrame()->scrollBarMaximum(Qt::Vertical) -
      pMessageLog->page()->mainFrame()->scrollBarValue(Qt::Vertical);

  //if (!groupMode)

  if(infoFlag == IT_Disconnected) {
      ui.labelInfo->setText("<span style='color:rgb(96,96,96);'>" + tr("You are no longer connected.") + "</span>");
      ui.labelInfo->setVisible(true);
  } else if(!groupMode && !isLocalUser && (infoFlag == IT_Offline))  {
      QString msg = tr("%1 is offline.");
      ui.labelInfo->setText("<span style='color:rgb(96,96,96);'>" + msg.arg(_peerNames.values().first()) + "</span>");
      ui.labelInfo->setVisible(true);
  } else if(!groupMode && !isLocalUser && (infoFlag == IT_Away)) {
      QString msg = tr("%1 is away.");
      ui.labelInfo->setText("<span style='color:rgb(255,115,0);'>" + msg.arg(_peerNames.values().first()) + "</span>");
      ui.labelInfo->setVisible(true);
  } else if(!groupMode && !isLocalUser && (infoFlag == IT_Busy)) {
      QString msg = tr("%1 is busy. You may be interrupting.");
      ui.labelInfo->setText("<span style='color:rgb(192,0,0);'>" + msg.arg(_peerNames.values().first()) + "</span>");
      ui.labelInfo->setVisible(true);
  } else if (!isLocalUser) {
      ui.labelInfo->setText(QString::null);
      ui.labelInfo->setVisible(false);
  }

  int scrollPos =
          pMessageLog->page()->mainFrame()->scrollBarMaximum(Qt::Vertical) -
          relScrollPos;
  pMessageLog->page()->mainFrame()->setScrollBarValue(Qt::Vertical, scrollPos);
}

QString lmcChatRoomWindow::getWindowTitle() {
    QString title;

    if (!_isPublicChat) {
        if (!_peerNames.isEmpty()) {
            if (_peerNames.size() < 4) {
                QHash<QString, QString>:: const_iterator index = _peerNames.constBegin();
                while (index != _peerNames.constEnd()) {
                    title.append(index.value() + ", ");
                    index++;
                }
                title.remove(title.length() - 2, 2);
                title.append(" - ");
                title.append(tr("Conversation"));
            } else {
                title = QString("%1 people in this conversation").arg(_peerNames.size());
            }
        } else {
            title = QStringLiteral("Alone in this conversation");
        }
    } else
        title = tr("Public Chat");

    return title;
}

void lmcChatRoomWindow::setMessageFont(QFont &font) {
  ui.textBoxMessage->setFont(font);
  ui.textBoxMessage->setFontPointSize(font.pointSize());
}

void lmcChatRoomWindow::updateStatusImage(QTreeWidgetItem *pItem,
                                          QString *lpszStatus) {
  StatusStruct *status = Globals::getInstance().getStatus(*lpszStatus);
  if (status)
    pItem->setIcon(
        0,
        QIcon(ThemeManager::getInstance().getAppIcon(status->icon)));
}

QTreeWidgetItem *lmcChatRoomWindow::getUserItem(QString *lpszUserId) {
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

QTreeWidgetItem *lmcChatRoomWindow::getGroupItem(QString *lpszGroupId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    QTreeWidgetItem *pItem = ui.treeWidgetUserList->topLevelItem(topIndex);
    if (pItem->data(0, IdRole).toString().compare(*lpszGroupId) == 0)
      return pItem;
  }

  return NULL;
}

void lmcChatRoomWindow::setUserAvatar(QString *lpszUserId,
                                      QString *lpszFilePath) {
  QTreeWidgetItem *pUserItem = getUserItem(lpszUserId);
  if (!pUserItem || !lpszFilePath)
    return;

  QPixmap avatar(*lpszFilePath);
  if (!avatar.isNull()) {
      avatar = avatar.scaled(QSize(32, 32), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
      pUserItem->setData(0, AvatarRole, QIcon(avatar));
      pMessageLog->updateAvatar(lpszUserId, lpszFilePath);
  }
}

void lmcChatRoomWindow::sendFile(bool sendFolder) {
  QString dir = pSettings->value(IDS_OPENPATH, StdLocation::getDocumentsPath())
                    .toString();
  QStringList fileNames;
  if (!sendFolder)
    fileNames = QFileDialog::getOpenFileNames(this, QString(), dir);
  else {
    QString directoryUrl = QFileDialog::getExistingDirectory(
        this, QString(), dir, QFileDialog::ShowDirsOnly);
    if (!directoryUrl.isEmpty())
      fileNames.append(directoryUrl);
  }

  if (fileNames.isEmpty())
    return;

  pSettings->setValue(IDS_OPENPATH, QFileInfo(fileNames[0]).absolutePath());

  MessageType messageType = sendFolder ? MT_Folder : MT_File;
  for (const QString &userId : _peerNames.keys()) {
    for (QString &fileName : fileNames)
      sendObject(userId, messageType, &fileName);
  }
}

void lmcChatRoomWindow::sendObject(const QString &peerId, MessageType type,
                                   QString *lpszPath, const QString &roomId) {
  if (bConnected) {
    XmlMessage xmlMessage;
    xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
    xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
    xmlMessage.addData(XN_FILEPATH, *lpszPath);

    if (roomId.isEmpty())
        xmlMessage.addData(XN_THREAD, _chatRoomId);
    else
        xmlMessage.addData(XN_THREAD, roomId);

    QString userId = peerIds.value(peerId);
    emit messageSent(type, &userId, &xmlMessage);
  } else
      appendMessageLog(MT_Error, NULL, NULL, NULL);
}

void lmcChatRoomWindow::processFileOp(XmlMessage *pMessage)
{
    int fileOp = Helper::indexOf(FileOpNames, FO_Max, pMessage->data(XN_FILEOP));
    int fileMode =
        Helper::indexOf(FileModeNames, FM_Max, pMessage->data(XN_MODE));
    QString fileId = pMessage->data(XN_FILEID);

    switch (fileOp) {
    case FO_Cancel:
    case FO_Accept:
    case FO_Decline:
    case FO_Error:
    case FO_Abort:
    case FO_Complete:
      updateFileMessage((FileMode)fileMode, (FileOp)fileOp, fileId);
      break;
    default:
      break;
    }
}

void lmcChatRoomWindow::setChatState(ChatState newChatState) {
  if (chatState == newChatState)
    return;

  bool bNotify = false;

  switch (newChatState) {
  case CS_Active:
  case CS_Inactive:
    chatState = newChatState;
    bNotify = true;
    break;
  case CS_Composing:
    chatState = newChatState;
    bNotify = true;
    snapKeyStroke = keyStroke;
    QTimer::singleShot(pauseTime, this, SLOT(checkChatState()));
    break;
  case CS_Paused:
    if (chatState != CS_Inactive) {
      chatState = newChatState;
      bNotify = true;
    }
    break;
  default:
    break;
  }

  // send a chat state message
  if (bNotify && bConnected) {
    XmlMessage xmlMessage;
    if (_peerNames.size () > 1)
      xmlMessage.addData(XN_THREAD, _chatRoomId);
    xmlMessage.addData(XN_CHATSTATE, ChatStateNames[chatState]);

    QHash<QString, QString>::const_iterator index = peerIds.constBegin();
    while (index != peerIds.constEnd()) {
      QString userId = index.value();
      emit messageSent(MT_ChatState, &userId, &xmlMessage);
      index++;
    }
  }
}

void lmcChatRoomWindow::updateFileMessage(FileMode mode, FileOp op,
                                          QString fileId) {
    pMessageLog->updateFileMessage(mode, op, fileId);
}

void lmcChatRoomWindow::toggleSideBar(bool toggle, bool toggled)
{
    if (toggle)
        toggled = ui.widgetUsersSidebar->isHidden();

    ui.widgetUsersSidebar->setVisible(toggled);

    if (!toggled) {
        _buttonToggleSideBar->setText(tr("<< Show"));
        _buttonToggleSideBar->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                               QStringLiteral("doubleLeft"))));
    } else {
        _buttonToggleSideBar->setText(tr("Hide >>"));
        _buttonToggleSideBar->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                               QStringLiteral("doubleRight"))));
    }
}

void lmcChatRoomWindow::setClibpoardIcon(QClipboard::Mode mode)
{
    Q_UNUSED(mode);

    if (_clipboard->mimeData()->hasImage()) {
        _buttonSendClipboard->setIcon(QIcon(ThemeManager::getInstance().getAppIcon("clipboard_up")));
        _buttonSendClipboard->setEnabled(true);

        _buttonSendClipboard->setText(tr("Send image from clipboard"));
        _buttonSendClipboard->setToolTip(tr("Send the image copied to clipboard"));
    } else if (_clipboard->mimeData()->hasUrls()) {
        _buttonSendClipboard->setIcon(QIcon(ThemeManager::getInstance().getAppIcon("clipboard_up")));
        _buttonSendClipboard->setEnabled(true);

        _buttonSendClipboard->setText(tr("Send files from clipboard"));
        _buttonSendClipboard->setToolTip(tr("Send the files or folders copied to clipboard"));
    } else {
        _buttonSendClipboard->setIcon(QIcon(ThemeManager::getInstance().getAppIcon("clipboard_empty")));
        _buttonSendClipboard->setEnabled(false);

        _buttonSendClipboard->setText(tr("Empty clipboard"));
        _buttonSendClipboard->setToolTip(tr("There is nothing saved in clipboard"));
    }
}
