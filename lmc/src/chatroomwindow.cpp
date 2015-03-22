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
#include <QDesktopWidget>
#include <QMessageBox>
#include <QCheckBox>

const qint64 pauseTime = 5000;

QString GroupId = "PARTICIPANTS";

lmcChatRoomWindow::lmcChatRoomWindow(QWidget *parent, bool isPublicChat)
    : QWidget(parent), _isPublicChat(isPublicChat) {
  ui.setupUi(this);
  setAcceptDrops(true);
  setProperty("isWindow", true);
  ui.textBoxMessage->setProperty("light", true);
  ui.labelSendKey->setProperty("subNote", true);
  ui.labelCountChars->setProperty("subNote", true);
  installEventFilter(this);

  ui.buttonLeave->setVisible(false);

  ui.labelCountChars->setVisible(Globals::getInstance().showCharacterCount());
  ui.labelSendKey->setVisible(Globals::getInstance().showCharacterCount());

  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemActivated, this,
          &lmcChatRoomWindow::treeWidgetUserList_itemActivated);
  connect(ui.treeWidgetUserList, &lmcUserTreeWidget::itemContextMenu, this,
          &lmcChatRoomWindow::treeWidgetUserList_itemContextMenu);
  connect(ui.buttonAddUsers, &ThemedButton::clicked, this,
          &lmcChatRoomWindow::buttonAddUsers_clicked);
  connect(ui.buttonLeave, &ThemedButton::clicked, this,
          &lmcChatRoomWindow::buttonLeaveChat_clicked);
  connect (ui.textBoxMessage, &QTextEdit::textChanged, this, &lmcChatRoomWindow::textChanged);

  _messageLog = new lmcMessageLog(ui.widgetLog);
  ui.widgetLog_layout->addWidget(_messageLog);
  _messageLog->setAcceptDrops(false);
  connect(_messageLog,
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
  _messageLog->installEventFilter(this);
  ui.treeWidgetUserList->installEventFilter(this);
  ui.textBoxMessage->installEventFilter(this);
  infoFlag = IT_Ok;
  dataSaved = false;
  windowLoaded = false;

  _localId = QString::null;
  _localName = QString::null;

  chatState = CS_Blank;
  keyStroke = 0;

  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this, SLOT(buttonHistory_clicked()));
}

lmcChatRoomWindow::~lmcChatRoomWindow() {
    _smileyMenu->clear();
    delete _smileyMenu;

    _emojiMenu->clear();
    delete _emojiMenu;

    _userMenu->clear();
    delete _userMenu;

    delete _soundPlayer;
}

void lmcChatRoomWindow::init(User *pLocalUser, bool connected, const QString &thread) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcChatRoomWindow.init started"));

  _localId = pLocalUser->id;
  _localName = pLocalUser->name;

  this->_localUser = pLocalUser;

  _messageLog->localId = _localId;
  _messageLog->savePath = QDir(StdLocation::getWritableCacheDir()).absoluteFilePath(QString("msg_%1.tmp").arg(thread));

  //	get the avatar image for the user
  _messageLog->participantAvatars.insert(_localId, pLocalUser->avatarPath);

  _chatRoomId = thread;
  _originalChatRoomId = thread;

  createUserMenu();
  createSmileyMenu();
  createEmojiMenu();
  createToolBar();

  bConnected = connected;
  if (!bConnected)
    showStatus(IT_Disconnected, true);

  _soundPlayer = new lmcSoundPlayer();

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

  QFont font;
  font.fromString(Globals::getInstance().messagesFontString());
  _messageColor = QApplication::palette().text().color();
  _messageColor.setNamedColor(Globals::getInstance().messagesColor());

  setMessageFont(font);
  ui.textBoxMessage->setStyleSheet(QString("QTextEdit { color: %1; }").arg(_messageColor.name()));
  ui.textBoxMessage->setFocus();

  if (_isPublicChat) {
      if (!Globals::getInstance().publicChatWindowGeometry().isEmpty())
          restoreGeometry(Globals::getInstance().publicChatWindowGeometry());
      else
          move(50, 50);

      if (!Globals::getInstance().publicChatVSplitterGeometry().isEmpty())
          ui.vSplitter->restoreState(Globals::getInstance().publicChatVSplitterGeometry());
      if (!Globals::getInstance().publicChatHSplitterGeometry().isEmpty())
          ui.hSplitter->restoreState(Globals::getInstance().publicChatHSplitterGeometry());
  } else {
      if (!Globals::getInstance().chatWindowGeometry().isEmpty())
          restoreGeometry(Globals::getInstance().chatWindowGeometry());
      else
          move(50, 50);

      if (!Globals::getInstance().chatVSplitterGeometry().isEmpty())
          ui.vSplitter->restoreState(Globals::getInstance().chatVSplitterGeometry());
      if (!Globals::getInstance().chatHSplitterGeometry().isEmpty())
          ui.hSplitter->restoreState(Globals::getInstance().chatHSplitterGeometry());
  }

  setUIText();

  ui.treeWidgetUserList->setView(Globals::getInstance().userListView());

  _messageLog->initMessageLog();

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
    Globals::getInstance().setPublicChatWindowGeometry(saveGeometry());
    Globals::getInstance().setPublicChatVSplitterGeometry(ui.vSplitter->saveState());
    Globals::getInstance().setPublicChatHSplitterGeometry(ui.hSplitter->saveState());
  } else {
      Globals::getInstance().setChatWindowGeometry(saveGeometry());
      Globals::getInstance().setChatVSplitterGeometry(ui.vSplitter->saveState());
      Globals::getInstance().setChatHSplitterGeometry(ui.hSplitter->saveState());
  }
}

void lmcChatRoomWindow::addUser(User *user) {
  LoggerManager::getInstance().writeInfo(
      QString("lmcChatRoomWindow.addUser started -|- User: %1")
          .arg(user ? user->name : "no user"));
  if (!user || _peerIds.contains(user->id))
    return;

  _lastUserId = QString::null;

  // Do not add user if user's version is 1.2.35 or less. These versions do not
  // support Public Chat feature.
  if (Helper::compareVersions(user->version, "1.2.35") <= 0)
    return;

  //	Do not add user if user is already in the list of participants
  if (_peerIds.contains(user->id))
    return;

  _peerIds.insert(user->id, user->id);
  _peerNames.insert(user->id, user->name);

  int index;
  StatusStruct *status =
      Globals::getInstance().getStatus(user->status, &index);

  lmcUserTreeWidgetUserItem *pItem = new lmcUserTreeWidgetUserItem();
  pItem->setData(0, IdRole, user->id);
  pItem->setData(0, TypeRole, "User");
  pItem->setData(0, StatusRole, index);
  pItem->setData(0, SubtextRole, user->note);
  pItem->setText(0, user->name);

  if (status)
    pItem->setIcon(0, QIcon(ThemeManager::getInstance().getAppIcon(status->icon)));

  lmcUserTreeWidgetGroupItem *pGroupItem =
      (lmcUserTreeWidgetGroupItem *)getGroupItem(GroupId);
  pGroupItem->addChild(pItem);
  pGroupItem->sortChildren(0, Qt::AscendingOrder);

  // this should be called after item has been added to tree
  setUserAvatar(user->id, user->avatarPath);

  if (!_isPublicChat) {
    XmlMessage xmlMessage;
    xmlMessage.addData(XN_THREAD, _chatRoomId);
    xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Join]);
    xmlMessage.addData(XN_USERID, user->id);

    appendMessageLog(MT_Join, user->id, user->name, xmlMessage);
    setWindowTitle(getWindowTitle());
    emit messageSent(MT_Message, QString::null, xmlMessage);

    for (QString &userId : _peerIds.keys())
        emit messageSent(MT_Message, userId, xmlMessage);
  }

  //	Local user cannot participate in public chat if status is offline
  if (_peerNames.size () <= 1 && !user->id.compare(_localId)) {
    bool offline = Globals::getInstance().getStatusType(user->status) ==
                   StatusTypeEnum::StatusOffline;
    ui.textBoxMessage->setEnabled(!offline);
    ui.textBoxMessage->setFocus();
  }

  if (_peerNames.size() == 1 && user->id.compare(_localId)) {
      StatusStruct *statusItem = Globals::getInstance().getStatus(user->status);
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

  QTreeWidgetItem *pItem = getUserItem(pUser->id);
  if (pItem) {
    int index;

   Globals::getInstance().getStatus(pUser->status, &index);

    updateStatusImage(pItem, pUser->status);
    pItem->setData(0, StatusRole, index);
    pItem->setData(0, SubtextRole, pUser->note);
    pItem->setText(0, pUser->name);
    QTreeWidgetItem *pGroupItem = pItem->parent();
    pGroupItem->sortChildren(0, Qt::AscendingOrder);
  }

  if (_peerNames.size () > 1)
    setWindowTitle(getWindowTitle());

  //	Local user cannot participate in public chat if status is offline
  if (_isPublicChat && pUser->id.compare(_localId) == 0) {
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

void lmcChatRoomWindow::removeUser(const QString &userId) {
  QTreeWidgetItem *item = getUserItem(userId);
  if (!item)
    return;

  if (_peerIds.size() == 1)
      _lastUserId = userId; // last user can only disconnect by going offline
  else
      _lastUserId = QString::null;

  QTreeWidgetItem *pGroup = item->parent();
  pGroup->removeChild(item);

  QString validUserId = _peerIds.value(userId);
  QString userName = _peerNames.value(userId);

  _peerIds.remove(userId);
  _peerNames.remove(userId);

  XmlMessage xmlMessage;
  xmlMessage.addData(XN_THREAD, _chatRoomId);
  xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Leave]);

  appendMessageLog(MT_Leave, validUserId, userName, xmlMessage);
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
  if (validUserId.compare(_localId) == 0)
    ui.textBoxMessage->setEnabled(false);
}

void lmcChatRoomWindow::setHtml(const QString &html) {
    _messageLog->prependHtml(html);
}

void lmcChatRoomWindow::receiveMessage(MessageType type, const QString &userId,
                                       const XmlMessage &message) {
  QString title;

  //	if lpszUserId is NULL, the message was sent locally
  QString senderId = !userId.isEmpty() ? userId : _localId;
  QString senderName = !userId.isEmpty() ? _peerNames.value(senderId) : _localName;
  QString data;

  StatusStruct *statusItem = Globals::getInstance().getStatus(message.data(XN_STATUS));

  switch (type) {
  case MT_PublicMessage:
    appendMessageLog(type, userId, senderName, message);
    if (isVisible() && !isActiveWindow()) {
        _soundPlayer->play(SE_NewPubMessage);
    }

    if (!isActiveWindow()) {
        if (Globals::getInstance().sysTrayNewPublicMessages())
            emit showTrayMessage(TM_Message, message.data(XN_MESSAGE), _chatRoomId, QString("%1 sent a public message").arg(senderName), TMI_Info);
        QApplication::alert(this);
    }
    break;
  case MT_Message:
  case MT_GroupMessage:
    _hasUnreadMessages = true;
    appendMessageLog(type, userId, senderName, message);
    if (isHidden() || !isActiveWindow()) {
      _soundPlayer->play(SE_NewMessage);
      title = tr("%1 says...");
      setWindowTitle(title.arg(senderName));

      if (!isActiveWindow()) {
          if (Globals::getInstance().sysTrayNewMessages())
              emit showTrayMessage(TM_Message, message.data(XN_MESSAGE), _chatRoomId, QString("%1 messaged you").arg(senderName), TMI_Info);
          QApplication::alert(this);
      } else if (Globals::getInstance().informReadMessage()) {
          _hasUnreadMessages = false;
          setChatState(CS_Read);
      }
    }
    break;
  case MT_Broadcast:
  case MT_InstantMessage:
      appendMessageLog(type, userId, senderName, message);
      if(isHidden() || !isActiveWindow()) {
          _soundPlayer->play(SE_NewMessage);
          title = QString("%1").arg(type == MT_Broadcast ? tr("Broadcast from %1") : tr("Message from %1"));
          setWindowTitle(title.arg(senderName));
      }

      if (!isActiveWindow()) {
          if (Globals::getInstance().sysTrayNewMessages())
              emit showTrayMessage(TM_Message, message.data(XN_MESSAGE), _chatRoomId, QString("%1").arg(type == MT_Broadcast ? tr("Received broadcast from %1") : tr("Received instant message from %1")).arg(senderName), TMI_Info);
          QApplication::alert(this);
      }
      break;
  case MT_ChatState:
    appendMessageLog(type, userId, senderName, message);
    break;
  case MT_Status:
  {
    data = message.data(XN_STATUS);
    bool isLocalUser = (_localUser->id == senderId);
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
    data = message.data(XN_FILEPATH);
    // this message may come with or without user id. NULL user id means avatar
    // change by local user, while non NULL user id means avatar change by a peer.
    setUserAvatar(senderId, data);
    break;
  case MT_UserName:
    data = message.data(XN_NAME);
    if (_peerNames.contains(senderId))
      _peerNames[senderId] = data;

    _messageLog->updateUserName(senderId, data);
    break;
  case MT_File:
  case MT_Folder:
      if(message.data(XN_FILEOP) == FileOpNames[FO_Request]) {
          //	a file request has been received
          appendMessageLog(type, userId, senderName, message);
          if(message.data(XN_MODE) == FileModeNames[FM_Receive] && (isHidden() || !isActiveWindow())) {
              _soundPlayer->play(SE_NewFile);
              if(type == MT_File)
                  title = tr("%1 sends a file...");
              else
                  title = tr("%1 sends a folder...");
              setWindowTitle(title.arg(senderName));
          }
      } else {
          // a file message of op other than request has been received
          processFileOp(message);
      }
      break;
  case MT_Failed:
    appendMessageLog(type, userId, senderName, message);
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
  _soundPlayer->settingsChanged();
  if (_localName.compare(_localUser->name) != 0) {
    _localName = _localUser->name;
    updateUser(_localUser);
    _messageLog->updateUserName(_localId, _localName);
  }
  _messageLog->reloadMessageLog();

  ui.labelCountChars->setVisible(Globals::getInstance().showCharacterCount());
  ui.labelSendKey->setVisible(Globals::getInstance().showCharacterCount());
  textChanged();


  ui.treeWidgetUserList->setView(Globals::getInstance().userListView());
  ui.labelSendKey->setText(QString("Send message using %1\t").arg(Globals::getInstance().sendByEnter() ? "Enter" : "Ctrl+Enter"));
}

void lmcChatRoomWindow::selectContacts(const std::vector<User *> selectedContacts) {
    for (User *user : selectedContacts) {
        addUser(user);
    }
}

bool lmcChatRoomWindow::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (object == ui.textBoxMessage) {
      if (keyEvent->key() == Qt::Key_Return ||
          keyEvent->key() == Qt::Key_Enter) {
        bool sendByEnter = ((keyEvent->modifiers() & Qt::ControlModifier) ==
                       Qt::NoModifier);
        if (sendByEnter == Globals::getInstance().sendByEnter()) {
          sendMessage();
          setChatState(CS_Active);
          return true;
        }
        // The TextEdit widget does not insert new line when Ctrl+Enter is pressed
        // So we insert a new line manually
        if (!sendByEnter)
          ui.textBoxMessage->insertPlainText("\n");
      } else if (keyEvent->key() == Qt::Key_Escape) {
        close();
        return true;
      } else if (keyEvent->key() == Qt::Key_B && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
          insertHtmlTag(QStringLiteral("[b]"), QStringLiteral("[/b]"));
          return true;
      } else if (keyEvent->key() == Qt::Key_U && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
          insertHtmlTag(QStringLiteral("[u]"), QStringLiteral("[/u]"));
          return true;
      } else if (keyEvent->key() == Qt::Key_I && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
          insertHtmlTag(QStringLiteral("[i]"), QStringLiteral("[/i]"));
          return true;
      }

      keyStroke++;
      setChatState(CS_Composing);
    } else {
      if (keyEvent->key() == Qt::Key_Escape) {
        close();
        emit closed(_originalChatRoomId);
        return true;
      }
    }
  } else if (event->type() == QEvent::Enter) {
      if (_hasUnreadMessages && ui.textBoxMessage->hasFocus() && Globals::getInstance().informReadMessage()) {
          _hasUnreadMessages = false;
          setChatState(CS_Read);
      }
  }

  return false;
}

void lmcChatRoomWindow::changeEvent(QEvent *event) {
  switch (event->type()) {
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

  QWidget::changeEvent(event);
}

void lmcChatRoomWindow::closeEvent(QCloseEvent *event) {
    setChatState(CS_Inactive);
    if (!_isPublicChat && !_leaveChatTriggered && _peerNames.size () > 1) {
        if (Globals::getInstance().confirmLeaveChat()) {
            QMessageBox messageBox;

            QCheckBox checkbox;
            checkbox.setText("Do not show this message again");

            messageBox.setCheckBox(&checkbox);
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setWindowTitle(QStringLiteral("Warning"));
            messageBox.setText(QStringLiteral("By closing this window you will leave the group chat. You cannot come back to the group chat if you leave. \nAre you sure you want to close this window?"));
            messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

            if (messageBox.exec() == QMessageBox::No) {
                event->ignore();
            } else {
                if (checkbox.isChecked())
                    Globals::getInstance().setConfirmLeaveChat(false);

                buttonLeaveChat_clicked();
            }
        } else {
            buttonLeaveChat_clicked();
        }
    }

    // call stop procedure to save history
    if (event->isAccepted()) {
        stop();
        emit closed(_chatRoomId);

        QWidget::closeEvent(event);
    }
}

void lmcChatRoomWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
      QList<QUrl> urls = event->mimeData()->urls();
      if (urls.isEmpty())
        return;

      QString fileName = urls.first().toLocalFile();
      if (fileName.isEmpty())
        return;

      event->acceptProposedAction();
    }
}

void lmcChatRoomWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
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

void lmcChatRoomWindow::moveEvent(QMoveEvent *event)
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

void lmcChatRoomWindow::userConversationAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();

  QString threadId = Helper::getUuid();
  threadId.prepend(_localUser->id);

  emit chatStarting(threadId, XmlMessage(), QStringList() << userId);
}

void lmcChatRoomWindow::userFileAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  QString dir = Globals::getInstance().fileOpenPath();
  QString fileName = QFileDialog::getOpenFileName(this, QString(), dir);
  if (!fileName.isEmpty()) {
      Globals::getInstance().setFileOpenPath(QFileInfo(fileName).dir().absolutePath());

      QString chatRoomId = _localUser->id;
      chatRoomId.append(Helper::getUuid());

      sendObject(userId, MT_File, &fileName, chatRoomId);
  }
}

void lmcChatRoomWindow::userFolderAction_triggered() {
  QString userId = ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  QString dir = Globals::getInstance().fileOpenPath();
  QString fileName = QFileDialog::getExistingDirectory(
              this, QString(), dir, QFileDialog::ShowDirsOnly);
  if (!fileName.isEmpty()) {
      QString chatRoomId = _localUser->id;
      chatRoomId.append(Helper::getUuid());

      sendObject(userId, MT_Folder, &fileName, chatRoomId);
  }
}

void lmcChatRoomWindow::userInfoAction_triggered() {
  QString userId =
      ui.treeWidgetUserList->currentItem()->data(0, IdRole).toString();
  XmlMessage xmlMessage;
  xmlMessage.addData(XN_QUERYOP, QueryOpNames[QO_Get]);
  emit messageSent(MT_Query, userId, xmlMessage);
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
  QColor color = QColorDialog::getColor(_messageColor, this, tr("Select Color"));
  if (color.isValid()) {
    _messageColor = color;
    ui.textBoxMessage->setStyleSheet("QTextEdit {color: " +
                                     _messageColor.name() + ";}");
  }
}

void lmcChatRoomWindow::buttonFile_clicked() { sendFile(); }

void lmcChatRoomWindow::buttonFolder_clicked() { sendFile(true); }

void lmcChatRoomWindow::buttonSave_clicked() {
  QString dir = Globals::getInstance().fileSavePath();
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save Conversation"), dir,
                                   "HTML File (*.html);;Text File (*.txt)");
  if (!fileName.isEmpty()) {
    Globals::getInstance().setFileSavePath(QFileInfo(fileName).dir().absolutePath());
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    if (fileName.endsWith(".html", Qt::CaseInsensitive))
      stream << _messageLog->prepareMessageLogForSave(HtmlFormat);
    else
      stream << _messageLog->prepareMessageLogForSave(TextFormat);
    file.close();
  }
}

void lmcChatRoomWindow::buttonSendClipboard_clicked() {
  if (_clipboard->mimeData()->hasImage()) {
      QPixmap imageData = qvariant_cast<QPixmap>(_clipboard->mimeData()->imageData());
      QString screenshotPath = QString("%1screenshot - %2.png").arg(StdLocation::getCacheDir(), _localUser->name);
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

  QHash<QString, QString>::const_iterator index = _peerIds.constBegin();
  while (index != _peerIds.constEnd()) {
    QString userId = index.value();
    excludeList.append(userId);
    index++;
  }

  emit contactsAdding(excludeList);
}

void lmcChatRoomWindow::buttonLeaveChat_clicked() {
    XmlMessage xmlMessage;
    xmlMessage.addData(XN_THREAD, _chatRoomId);
    xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Leave]);

    for (QString &peerId : _peerNames.keys())
        emit messageSent(MT_GroupMessage, peerId, xmlMessage);

    _leaveChatTriggered = true;
    close();
}

void lmcChatRoomWindow::buttonToggleSideBar_clicked()
{
    toggleSideBar();
}

void lmcChatRoomWindow::smileyAction_triggered() {
    //	nSmiley contains index of smiley
    if (nSmiley >= 0)
        insertSmileyCode(ImagesList::getInstance().getSmileys()[nSmiley]);
}

void lmcChatRoomWindow::emojiAction_triggered() {
    if (nEmoji >= 0)
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

void lmcChatRoomWindow::insertHtmlTag(const QString &beginTag, const QString &endTag) {
    QTextCursor cursor = ui.textBoxMessage->textCursor();
    QString selectedText = cursor.selectedText();

    bool moveCursor = selectedText.isEmpty();

    selectedText = QString("%1%2%3").arg(beginTag, selectedText, endTag);

    ui.textBoxMessage->insertPlainText(selectedText);

    if (moveCursor) {
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, endTag.length());
        ui.textBoxMessage->setTextCursor(cursor);
    }
    ui.textBoxMessage->setFocus();
}

void lmcChatRoomWindow::log_sendMessage(MessageType type, QString userId,
                                        XmlMessage message) {
    message.removeData(XN_THREAD);
    message.addData(XN_THREAD, _chatRoomId);
    emit messageSent(type, userId, message);
}

void lmcChatRoomWindow::treeWidgetUserList_itemActivated(QTreeWidgetItem *item,
                                                         int column) {
  Q_UNUSED(column);
  if (item->data(0, TypeRole).toString().compare("User") == 0) {
    QString userId = item->data(0, IdRole).toString();
    if (userId.compare(_localId) != 0) {
        QString chatRoomId = _localUser->id;
        chatRoomId.append(Helper::getUuid());



        emit chatStarting(chatRoomId, XmlMessage(), QStringList() << userId);
    }
  }
}

void
lmcChatRoomWindow::treeWidgetUserList_itemContextMenu(QTreeWidgetItem *item,
                                                      QPoint pos) {
  if (!item)
    return;

  if (item->data(0, TypeRole).toString().compare("User") == 0) {
    for (int index = 0; index < _userMenu->actions().count(); index++)
      _userMenu->actions()[index]->setData(item->data(0, IdRole));

    _userChatAction->setVisible(_peerNames.size() != 1);
    _userMenu->exec(pos);
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

void lmcChatRoomWindow::textChanged() {
    if (Globals::getInstance().showCharacterCount()) {
        QString text = ui.textBoxMessage->toPlainText();
        text = text.remove(QRegularExpression("(\\[/?(b|u|i|q|quote)\\])", QRegularExpression::DontCaptureOption));
        ui.labelCountChars->setText(QString("%1 character%2").arg(QString::number(text.length()), text.length() != 1 ? "s" : ""));
    }
}

void lmcChatRoomWindow::createUserMenu() {
  _userMenu = new QMenu(this);
  _userChatAction = _userMenu->addAction(
      "&Conversation", this, SLOT(userConversationAction_triggered()));
  _userFileAction = _userMenu->addAction("Send &File", this,
                                        SLOT(userFileAction_triggered()));
  _userFolderAction = _userMenu->addAction("Send &Folder", this,
                                        SLOT(userFolderAction_triggered()));
  _userMenu->addSeparator();
  _userInfoAction = _userMenu->addAction("Get &Information", this,
                                        SLOT(userInfoAction_triggered()));
}

void lmcChatRoomWindow::createSmileyMenu() {
  _smileyMenu = new QMenu(this);

  _smileyAction = new lmcImagePickerAction(
      _smileyMenu, ImagesList::getInstance().getSmileys(),
      ImagesList::getInstance().getTabs(ImagesList::Smileys), 39, 39, 10,
      &nSmiley, true);
  connect(_smileyAction, &lmcImagePickerAction::imageSelected, this,
          &lmcChatRoomWindow::smileyAction_triggered);

  _smileyMenu->addAction(_smileyAction);
}

void lmcChatRoomWindow::createEmojiMenu() {
  _emojiMenu = new QMenu(this);

  _emojiAction = new lmcImagePickerAction(
      _emojiMenu, ImagesList::getInstance().getEmojis(),
      ImagesList::getInstance().getTabs(ImagesList::Emojis), 78, 39, 8, &nEmoji,
      true);
  connect(_emojiAction, &lmcImagePickerAction::imageSelected, this,
          &lmcChatRoomWindow::emojiAction_triggered);

  _emojiMenu->addAction(_emojiAction);
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
  buttonSmiley->setMenu(_smileyMenu);

  ThemedButton *buttonEmoji = new ThemedButton(ui.widgetToolBar);
  buttonEmoji->setPopupMode(QToolButton::InstantPopup);
  buttonEmoji->setToolButtonStyle(Qt::ToolButtonIconOnly);
  buttonEmoji->setAutoRaise(true);
  buttonEmoji->setIconSize(QSize(17, 17));
  buttonEmoji->setFixedWidth(32);
  buttonEmoji->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                         QStringLiteral("emoji"))));
  buttonEmoji->setMenu(_emojiMenu);

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

  QTreeWidgetItem *pGroupItem = getGroupItem(GroupId);
  pGroupItem->setText(0, tr("Participants"));

  _userChatAction->setText(tr("&Conversation"));
  _userFileAction->setText(tr("Send &File"));
  _userInfoAction->setText(tr("Get &Information"));

  showStatus(IT_Ok, false); //	this will force the info label to retranslate
  ui.labelSendKey->setText(QString("Send message using %1\t").arg(Globals::getInstance().sendByEnter() ? "Enter" : "Ctrl+Enter"));
}

void lmcChatRoomWindow::sendMessage() {
  if (ui.textBoxMessage->document()->isEmpty())
    return;

  if (bConnected) {
    QString message(ui.textBoxMessage->toPlainText());

    QFont font = ui.textBoxMessage->font();
    font.setPointSize(ui.textBoxMessage->fontPointSize());  // TODO is this negative ???

    MessageType type = !_isPublicChat ? (_peerNames.size() <= 1 ? MT_Message : MT_GroupMessage) : MT_PublicMessage;
    XmlMessage xmlMessage;
    xmlMessage.addHeader(XN_TIME, QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    xmlMessage.addData(XN_FONT, font.toString());
    xmlMessage.addData(XN_COLOR, _messageColor.name());
    xmlMessage.addData(XN_MESSAGE, message);
    xmlMessage.addData(XN_THREAD, _chatRoomId);

    if (_peerNames.size () > 1) {
        xmlMessage.addData(XN_GROUPMSGOP, GroupMsgOpNames[GMO_Message]);
    }

    appendMessageLog(type, _localId, _localName, xmlMessage);

    QHash<QString, QString>::const_iterator index = _peerNames.constBegin();
    while (index != _peerNames.constEnd()) {
        QString userId = index.key();
        emit messageSent(type, userId, xmlMessage);
        index++;
    }
  } else
    appendMessageLog(MT_Error, QString::null, QString::null, XmlMessage());

  ui.textBoxMessage->clear();
  ui.textBoxMessage->setFocus();
}

void lmcChatRoomWindow::appendMessageLog(MessageType type, const QString &userId,
                                         const QString &userName,
                                         const XmlMessage &message) {
    _messageLog->appendMessageLog(type, userId, userName, message, false, _peerNames.count(), true, _localUser, _peerNames);

  if (!_buttonSaveConversation->isEnabled())
    _buttonSaveConversation->setEnabled(_messageLog->hasData);
}

void lmcChatRoomWindow::showStatus(int flag, bool isLocalUser) {
  infoFlag = flag;
  bool groupMode = _peerNames.size() != 1;

  int relScrollPos =
      _messageLog->page()->mainFrame()->scrollBarMaximum(Qt::Vertical) -
      _messageLog->page()->mainFrame()->scrollBarValue(Qt::Vertical);

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
          _messageLog->page()->mainFrame()->scrollBarMaximum(Qt::Vertical) -
          relScrollPos;
  _messageLog->page()->mainFrame()->setScrollBarValue(Qt::Vertical, scrollPos);
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
                                          const QString &status) {
  StatusStruct *statusStruct = Globals::getInstance().getStatus(status);
  if (statusStruct)
    pItem->setIcon(
        0,
        QIcon(ThemeManager::getInstance().getAppIcon(statusStruct->icon)));
}

QTreeWidgetItem *lmcChatRoomWindow::getUserItem(const QString &userId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    for (int index = 0;
         index < ui.treeWidgetUserList->topLevelItem(topIndex)->childCount();
         index++) {
      QTreeWidgetItem *pItem =
          ui.treeWidgetUserList->topLevelItem(topIndex)->child(index);
      if (pItem->data(0, IdRole).toString().compare(userId) == 0)
        return pItem;
    }
  }

  return NULL;
}

QTreeWidgetItem *lmcChatRoomWindow::getGroupItem(const QString &groupId) {
  for (int topIndex = 0; topIndex < ui.treeWidgetUserList->topLevelItemCount();
       topIndex++) {
    QTreeWidgetItem *pItem = ui.treeWidgetUserList->topLevelItem(topIndex);
    if (pItem->data(0, IdRole).toString().compare(groupId) == 0)
      return pItem;
  }

  return NULL;
}

void lmcChatRoomWindow::setUserAvatar(const QString &userId,
                                      const QString &filePath) {
  QTreeWidgetItem *pUserItem = getUserItem(userId);
  if (!pUserItem || filePath.isEmpty())
    return;

  QPixmap avatar(filePath);
  if (!avatar.isNull()) {
      avatar = avatar.scaled(QSize(32, 32), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
      pUserItem->setData(0, AvatarRole, QIcon(avatar));
      _messageLog->updateAvatar(userId, filePath);
  }
}

void lmcChatRoomWindow::sendFile(bool sendFolder) {
  QString dir = Globals::getInstance().fileOpenPath();
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

 Globals::getInstance().setFileOpenPath(QFileInfo(fileNames[0]).absolutePath());

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

    QString userId = _peerIds.value(peerId);
    emit messageSent(type, userId, xmlMessage);
  } else
      appendMessageLog(MT_Error, QString::null, QString::null, XmlMessage());
}

void lmcChatRoomWindow::processFileOp(const XmlMessage &message)
{
    int fileOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
    int fileMode =
        Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));
    QString fileId = message.data(XN_FILEID);

    switch (fileOp) {
    case FO_Cancel:
    case FO_Accept:
    case FO_Decline:
    case FO_Error:
    case FO_Abort:
    case FO_Complete:
      updateFileMessage(static_cast<FileMode>(fileMode), static_cast<FileOp>(fileOp), fileId);
      break;
    default:
      break;
    }
}

void lmcChatRoomWindow::setChatState(ChatState newChatState) {
  if (chatState == newChatState && newChatState != CS_Read)
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
  case CS_Read:
      if (!_isPublicChat && _peerNames.size() == 1) {
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

    QHash<QString, QString>::const_iterator index = _peerIds.constBegin();
    while (index != _peerIds.constEnd()) {
      QString userId = index.value();
      emit messageSent(MT_ChatState, userId, xmlMessage);
      index++;
    }
  }
}

void lmcChatRoomWindow::updateFileMessage(FileMode mode, FileOp op,
                                          QString fileId) {
    _messageLog->updateFileMessage(mode, op, fileId);
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
