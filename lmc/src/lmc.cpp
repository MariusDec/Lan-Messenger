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

#include "lmc.h"
#include "loggermanager.h"
#include "globals.h"

#include <vector>
#include <QMessageBox>
#include <QTranslator>

lmcCore::lmcCore() {
  _messaging = new lmcMessaging();
  connect(_messaging, &lmcMessaging::messageReceived, this,
          &lmcCore::receiveMessage);
  connect(_messaging, &lmcMessaging::connectionStateChanged, this,
          &lmcCore::connectionStateChanged);

  _mainWindow = new lmcMainWindow();
  connect(_mainWindow, &lmcMainWindow::appExiting, this, &lmcCore::exitApp);
  connect(_mainWindow, &lmcMainWindow::chatRoomStarting, this,
          &lmcCore::startChat);
  connect(_mainWindow, &lmcMainWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(_mainWindow, &lmcMainWindow::showTransfers, this,
          &lmcCore::showTransfers);
  connect(_mainWindow, &lmcMainWindow::showMessage, this, &lmcCore::showMessage);
  connect(_mainWindow, &lmcMainWindow::showHistory, this,
          &lmcCore::showHistory);
  connect(_mainWindow, &lmcMainWindow::showSettings, this,
          &lmcCore::showSettings);
  connect(_mainWindow, &lmcMainWindow::showHelp, this, &lmcCore::showHelp);
  connect(_mainWindow, &lmcMainWindow::showUpdate, this, &lmcCore::showUpdate);
  connect(_mainWindow, &lmcMainWindow::showAbout, this, &lmcCore::showAbout);
  connect(_mainWindow, &lmcMainWindow::showBroadcast, this,
          &lmcCore::showBroadcast);
  connect(_mainWindow, &lmcMainWindow::showPublicChat, this,
          &lmcCore::showPublicChat);
  connect(_mainWindow, &lmcMainWindow::groupUpdated, this,
          &lmcCore::updateGroup);
  connect(_mainWindow, &lmcMainWindow::sendInstantMessage, this, &lmcCore::showInstantMessage);

  _publicChatWindow = new lmcChatRoomWindow(0, true);
  connect(_publicChatWindow, &lmcChatRoomWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(_publicChatWindow, &lmcChatRoomWindow::chatStarting, this,
          &lmcCore::startChat);
  connect(_publicChatWindow, &lmcChatRoomWindow::showTrayMessage,
          this, &lmcCore::showTrayMessage);

  _chatRoomWindows.clear();
  _transferWindow = NULL;
  _historyWindow = NULL;
  _settingsDialog = NULL;
  _userInfoWindow = NULL;
  _helpWindow = NULL;
  _updateWindow = NULL;
  _userSelectDialog = NULL;
  _aboutDialog = NULL;
  _broadcastWindow = nullptr;
  _timer.setParent(this);
}

lmcCore::~lmcCore() {}

void lmcCore::init(const QString &szCommandArgs) {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.init started"));

  //	prevent auto app exit when last visible window is closed
  qApp->setQuitOnLastWindowClosed(false);

  QStringList arguments = szCommandArgs.split("\n", QString::SkipEmptyParts);
  //	remove duplicates
  arguments = arguments.toSet().toList();

  _initParams.clear();
  if (arguments.contains("/silent", Qt::CaseInsensitive))
    _initParams.addData(XN_SILENTMODE, LMC_TRUE);
  if (arguments.contains("/trace", Qt::CaseInsensitive)) {
    _initParams.addData(XN_TRACEMODE, LMC_TRUE);
  }
  for (int index = 0; index < arguments.count(); index++) {
    if (arguments.at(index).startsWith("/port=", Qt::CaseInsensitive)) {
      QString port = arguments.at(index).mid(QString("/port=").length());
      _initParams.addData(XN_PORT, port);
      continue;
    }
    if (arguments.at(index).startsWith("/config=", Qt::CaseInsensitive)) {
      QString configFile =
          arguments.at(index).mid(QString("/config=").length());
      _initParams.addData(XN_CONFIG, configFile);
      continue;
    }
  }

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init -|- Application initialized"));

  loadSettings();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init -|- Settings loaded"));

  _messaging->init(_initParams);
  _mainWindow->init(_messaging->localUser, _messaging->groupList,
                    _messaging->isConnected());
  _publicChatWindow->init(_messaging->localUser, _messaging->isConnected());

  saveUnsavedChatLog();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init ended"));
}

bool lmcCore::start() {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.start started"));

  _messaging->start();

  if (_messaging->isConnected() && !_messaging->canReceive()) {
    showPortConflictMessage();
    //	stop the application
    stop();

    LoggerManager::getInstance().writeInfo(
        QString("lmcCore.start-|- A port address conflict has been detected. "
                "%1 will close now.").arg(lmcStrings::appName()));

    return false;
  }

  _mainWindow->start();

  connect(&_timer, &QTimer::timeout, this, &lmcCore::timer_timeout);
  //	Set the timer to trigger 10 seconds after the application starts. After the
  //	first trigger, the timeout period will be decided by user settings.
  _adaptiveRefresh = false;
  _timer.start(10000);
  lmcSettings::setAutoStart(Globals::getInstance().autoStart());
  if (Globals::getInstance().autoShow())
      _mainWindow->restore();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.start ended"));
  return true;
}

//	This is the initial point where settings are used in the application
void lmcCore::loadSettings() {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.loadSettings started"));

  bool silent = Helper::stringToBool(_initParams.data(XN_SILENTMODE));
  if (_initParams.dataExists(XN_CONFIG)) {
    QString configFile = _initParams.data(XN_CONFIG);
    if (!Globals::getInstance().loadSettingsFromConfig(configFile) && !silent) {
      QString message = tr("Preferences could not be imported from '%1'.\n\n"
                           "File may not exist, or may not be compatible with "
                           "this version of %2.");
      QMessageBox::warning(0, lmcStrings::appName(),
                           message.arg(configFile, lmcStrings::appName()));
    }
  }
  _language = Globals::getInstance().language();
  Application::setLanguage(_language);
  Application::setLayoutDirection(
      tr("LAYOUT_DIRECTION") == RTL_LAYOUT ? Qt::RightToLeft : Qt::LeftToRight);

  ThemeManager::getInstance().changeChatTheme(Globals::getInstance().chatTheme());
  ThemeManager::getInstance().changeTheme(Globals::getInstance().applicationTheme(), Globals::getInstance().buttonsTheme());
  ThemeManager::getInstance().setIconTheme(Globals::getInstance().iconTheme());

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.loadSettings ended"));
}

void lmcCore::settingsChanged() {
    // TODO only call Globals::settingsChanged

    Globals::getInstance().loadSettings();
  _messaging->settingsChanged();
  _mainWindow->settingsChanged();
  if (_publicChatWindow)
    _publicChatWindow->settingsChanged();
  for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
    chatRoomWindow->settingsChanged();
  if (_transferWindow)
    _transferWindow->settingsChanged();
  if (_userInfoWindow)
    _userInfoWindow->settingsChanged();
  if (_historyWindow)
    _historyWindow->settingsChanged();
  if (_settingsDialog)
    _settingsDialog->settingsChanged();
  if (_helpWindow)
    _helpWindow->settingsChanged();
  if (_aboutDialog)
    _aboutDialog->settingsChanged();
  if (_broadcastWindow)
    _broadcastWindow->settingsChanged();
  for (InstantMessageWindow *window : _instantMessageWindows)
      window->settingsChanged();

  _timer.setInterval(Globals::getInstance().refreshInterval());
  lmcSettings::setAutoStart(Globals::getInstance().autoStart());
  QString appLang = Globals::getInstance().language();
  if (appLang.compare(_language) != 0) {
    _language = appLang;
    Application::setLanguage(_language);
    Application::setLayoutDirection(tr("LAYOUT_DIRECTION") == RTL_LAYOUT
                                        ? Qt::RightToLeft
                                        : Qt::LeftToRight);
    lmcStrings::retranslate();
  }
}

void lmcCore::stop() {
    LoggerManager::getInstance().writeInfo("lmcCore.stop started");

    for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows) {
        chatRoomWindow->stop();
        chatRoomWindow->deleteLater();
    }

    for (InstantMessageWindow *messageWindow : _instantMessageWindows) {
        messageWindow->stop();
        messageWindow->deleteLater();
    }

  if (_transferWindow) {
    _transferWindow->stop();
    _transferWindow->deleteLater();
  }

  if (_historyWindow) {
    _historyWindow->stop();
    _historyWindow->deleteLater();
  }

  if (_userInfoWindow)
    _userInfoWindow->deleteLater();

  if (_helpWindow) {
    _helpWindow->stop();
    _helpWindow->deleteLater();
  }

  if (_updateWindow) {
    _updateWindow->stop();
    _updateWindow->deleteLater();
  }

  if (_broadcastWindow) {
    _broadcastWindow->stop();
    _broadcastWindow->deleteLater();
  }

  if (_publicChatWindow) {
    _publicChatWindow->stop();
    _publicChatWindow->deleteLater();
  }

  if (_timer.isActive())
    _timer.stop();

  _messaging->stop();
  _mainWindow->stop();

  LoggerManager::getInstance().writeInfo("lmcCore.stop ended");
}

//	This slot handles the exit signal emitted by main window when the user
//	selects quit from the menu.
void lmcCore::exitApp() {
    saveUnsavedChatLog();
    qApp->quit();
}

//	This slot handles the signal emitted by QApplication when the
// application
//	quits either by user interaction or from operating system signal.
void lmcCore::aboutToExit() {
  stop();
  Globals::getInstance().setVersion(IDA_VERSION);

  LoggerManager::getInstance().writeInfo(QStringLiteral("Application exit"));

  Globals::getInstance().syncSettings();
}

void lmcCore::timer_timeout() {
  //	Refresh the contacts list whenever the timer triggers
  _messaging->update();
  if (_adaptiveRefresh) {
    //	The refresh interval is doubled until the refresh time defined by user
    // is reached.
    //	Then keep refreshing at that interval.
    int nextInterval = _timer.interval() * 2;
    int maxInterval = Globals::getInstance().refreshInterval() * 1000;
    int interval = qMin(nextInterval, maxInterval);
    _adaptiveRefresh = (nextInterval >= maxInterval) ? false : true;
    _timer.setInterval(interval);
  } else if (Globals::getInstance().refreshInterval() > _timer.interval()) {
    _timer.setInterval(Globals::getInstance().refreshInterval());
  }
}

void lmcCore::startChat(QString roomId, XmlMessage message, QStringList contacts) {
    lmcChatRoomWindow *chatWindow = nullptr;
    bool alert = Globals::getInstance().popOnNewMessage();
    if (contacts.size() == 1) {
        for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
            if (chatRoomWindow->getUsers().size() == 1 and chatRoomWindow->getUsers().contains(contacts[0]))
                chatWindow = chatRoomWindow;
    }

    if (!chatWindow) {
        createChatRoomWindow(roomId);
        chatWindow = _chatRoomWindows.last();
        alert = true;
    }

    showChatRoomWindow(chatWindow, true, alert, true, contacts);

    if (chatWindow->getUsers().size() == 1 && Globals::getInstance().appendHistory()) {
        chatWindow->setHtml(History::getUserMessageHistory(chatWindow->getUsers().keys().first(), QDate::currentDate()));
    }

    if (message.isValid() and message.dataExists(XN_MESSAGE)
            and contacts.size() == 1) { // used only for broadcast for now - should be used for MT_InstantMessage as well
        chatWindow->receiveMessage(MT_Broadcast, contacts.first(), message);
    }
}

void lmcCore::sendMessage(MessageType type, QString userId,
                          XmlMessage message) {
  switch (type) {
  case MT_Broadcast:
  case MT_InstantMessage:
  case MT_UserName:
  case MT_Message:
  case MT_PublicMessage:
  case MT_GroupMessage:
  case MT_Query:
  case MT_Group:
  case MT_ChatState:
  case MT_Version:
  case MT_File:
  case MT_Folder:
  case MT_Avatar:
  case MT_UserList:
  case MT_Status:
  case MT_Note:
    _messaging->sendMessage(type, userId, message);
    break;
  case MT_Refresh:
    _messaging->update();
    break;
  default:
    break;
  }
}

void lmcCore::receiveMessage(MessageType type, QString userId,
                             XmlMessage message) {
  processMessage(type, userId, message);
}

bool lmcCore::receiveAppMessage(const QString &szMessage) {
  LoggerManager::getInstance().writeInfo(
      QString("lmcCore.receiveAppMessage started -|- messageSize: %1 - %2")
          .arg(QString::number(szMessage.size()), szMessage));

  bool doNotExit = true;

  if (szMessage.isEmpty()) {
    _mainWindow->restore();
    return doNotExit;
  }

  QStringList messageList = szMessage.split("\n", QString::SkipEmptyParts);
  //	remove duplicates
  messageList = messageList.toSet().toList();

  if (messageList.contains("/new", Qt::CaseInsensitive)) {
    if (messageList.contains("/loopback", Qt::CaseInsensitive))
      _messaging->setLoopback(true);
  }
  if (messageList.contains("/nohistory", Qt::CaseInsensitive)) {
      QDir dir(History::historyFilesDir());
      dir.setNameFilters(QStringList() << "*.xml");
      dir.setFilter(QDir::Files);
      foreach(QString dirFile, dir.entryList())
          dir.remove(dirFile);

    if (_historyWindow)
      _historyWindow->updateList();
  }
//  if (messageList.contains("/nofilehistory", Qt::CaseInsensitive)) {
//    // TODO maybe it shouldn't delete the files...
//    QFile::remove(StdLocation::defaultTransferHistorySavePath());
//    if (pTransferWindow)
//      pTransferWindow->updateList();
//  }
  if (messageList.contains("/noconfig", Qt::CaseInsensitive)) {
    // TODO this shouldn't remove the avatar file...
    // QFile::remove(StdLocation::avatarFile());
   // QFile::remove(pSettings->fileName());
    Globals::getInstance().syncSettings();
    settingsChanged();
  }
  if (messageList.contains("/sync", Qt::CaseInsensitive)) {
    lmcSettings::setAutoStart(Globals::getInstance().autoStart());
  }
  if (messageList.contains("/unsync", Qt::CaseInsensitive)) {
    lmcSettings::setAutoStart(false);
  }
  if (messageList.contains("/term", Qt::CaseInsensitive)) {
    doNotExit = false;
    exitApp();
  }
  if (messageList.contains("/quit", Qt::CaseInsensitive)) {
    doNotExit = false;
  }

  LoggerManager::getInstance().writeInfo(
      QString("lmcCore.receiveAppMessage ended -|- doNotExit: %1")
          .arg(doNotExit ? "true" : "false"));

  return doNotExit;
}

void lmcCore::connectionStateChanged() {
  bool connected = _messaging->isConnected();
  Globals::getInstance().setIsConnected(connected);

  _mainWindow->connectionStateChanged(connected);
  if (_publicChatWindow)
    _publicChatWindow->connectionStateChanged(connected);
  for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
    chatRoomWindow->connectionStateChanged(connected);
  if (_broadcastWindow)
    _broadcastWindow->connectionStateChanged(connected);

  if (_messaging->isConnected() && !_messaging->canReceive()) {
    showPortConflictMessage();
    exitApp();
  }

  for (InstantMessageWindow *window : _instantMessageWindows)
      window->connectionChanged(connected);
}

void lmcCore::showTransfers(QString userId) {
  createTransferWindow();
  showTransferWindow(true, userId);
}

void lmcCore::showMessage(QString chatRoomId) {
    if (!_publicChatWindow.data()->getRoomId().compare(chatRoomId))
        showPublicChat();
    else if (!chatRoomId.isEmpty()) {
        for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
            if (!chatRoomWindow->getRoomId().compare(chatRoomId))
                showChatRoomWindow(chatRoomWindow, true, true);
    }
}

void lmcCore::showHistory(QString userId) {
  if (!_historyWindow) {
    _historyWindow = new lmcHistoryWindow();
    _historyWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (_historyWindow->windowState().testFlag(Qt::WindowMinimized))
    _historyWindow->setWindowState(_historyWindow->windowState() &
                                   ~Qt::WindowMinimized);
  _historyWindow->setWindowState(_historyWindow->windowState() |
                                 Qt::WindowActive);
  _historyWindow->raise(); // make window the top most window of the application
  _historyWindow->show();
  _historyWindow->activateWindow(); // bring window to foreground
  _historyWindow->setUserFilter(userId);
}

void lmcCore::showSettings() {
  if (!_settingsDialog) {
    _settingsDialog = new lmcSettingsDialog();
    connect(_settingsDialog, &lmcSettingsDialog::historyCleared, this,
            &lmcCore::historyCleared);
    connect(_settingsDialog, &lmcSettingsDialog::fileHistoryCleared, this,
            &lmcCore::fileHistoryCleared);
  }

  if (_settingsDialog->exec())
    settingsChanged();
}

void lmcCore::showHelp(QRect rect) {
  if (!_helpWindow) {
    _helpWindow = new lmcHelpWindow(rect);
    _helpWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (_helpWindow->windowState().testFlag(Qt::WindowMinimized))
    _helpWindow->setWindowState(_helpWindow->windowState() &
                                ~Qt::WindowMinimized);
  _helpWindow->setWindowState(_helpWindow->windowState() | Qt::WindowActive);
  _helpWindow->raise(); // make window the top most window of the application
  _helpWindow->show();
  _helpWindow->activateWindow(); // bring window to foreground
}

void lmcCore::showUpdate(QRect rect) {
  if (!_updateWindow) {
    _updateWindow = new lmcUpdateWindow(rect);
    connect(_updateWindow,
            &lmcUpdateWindow::messageSent, this, &lmcCore::sendMessage);
    _updateWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (_updateWindow->windowState().testFlag(Qt::WindowMinimized))
    _updateWindow->setWindowState(_updateWindow->windowState() &
                                  ~Qt::WindowMinimized);
  _updateWindow->setWindowState(_updateWindow->windowState() |
                                Qt::WindowActive);
  _updateWindow->raise(); // make window the top most window of the application
  _updateWindow->show();
  _updateWindow->activateWindow(); // bring window to foreground
}

void lmcCore::showAbout() {
  if (!_aboutDialog) {
    _aboutDialog = new lmcAboutDialog(_mainWindow);
    _aboutDialog->init();
  }

  _aboutDialog->exec();
}

void lmcCore::showBroadcast() {
  if (!_broadcastWindow) {
    _broadcastWindow = new lmcBroadcastWindow(_messaging->localUser);
    connect(_broadcastWindow,
            &lmcBroadcastWindow::messageSent, this,
            &lmcCore::sendMessage);
    _broadcastWindow->init(_messaging->isConnected());
  }

  if (_broadcastWindow->isHidden()) {
    QList<QTreeWidgetItem*> contactsList = _mainWindow->getContactsList();
    QStringList users = _mainWindow->getSelectedUserIds();

    _broadcastWindow->show(contactsList, users);
  } else {
    _broadcastWindow->show();
  }
}

void lmcCore::showPublicChat() {
  //	Show public chat window
  showPublicChatWindow(true);
}

void lmcCore::showInstantMessage(const QString &userId) {
    User *user = _messaging->getUser(userId);

    if (user) {
        InstantMessageWindow *instantMessageWindow = new InstantMessageWindow(_messaging->localUser);
        XmlMessage message;
        instantMessageWindow->init(user->id, user->name, MT_InstantMessage, message);
        connect(instantMessageWindow, &InstantMessageWindow::chatStarting, this, &lmcCore::startChat);
        connect(instantMessageWindow, &InstantMessageWindow::messageSent, this, &lmcCore::sendMessage);
        connect(instantMessageWindow, &InstantMessageWindow::closed, this, &lmcCore::instantMessageWindow_closed);

        _instantMessageWindows.append(instantMessageWindow);
    }
}

void lmcCore::historyCleared() {
  if (_historyWindow)
    _historyWindow->updateList();
}

void lmcCore::fileHistoryCleared() {
  if (_transferWindow)
    _transferWindow->updateList();
}

void lmcCore::showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId,
                              QString szTitle, TrayMessageIcon icon) {
  _mainWindow->showTrayMessage(type, szMessage, chatRoomId, szTitle, icon);
}

void lmcCore::updateGroup(GroupOp op, QVariant value1, QVariant value2) {
  _messaging->updateGroup(op, value1, value2);
}

void lmcCore::addContacts(QStringList excludeList) {
  lmcChatRoomWindow *chatRoomWindow =
      static_cast<lmcChatRoomWindow *>(sender());
  QStringList selectedContacts =
      showSelectContacts(chatRoomWindow, UC_GroupMessage, excludeList);

  std::vector<User *> users;

  for (QString &userId : selectedContacts) {
      User *tempUser = _messaging->getUser(userId);
      if (tempUser)
          users.push_back(tempUser);
  }
  chatRoomWindow->selectContacts(users);
}

void lmcCore::saveUnsavedChatLog() {
    QString cachePath = StdLocation::getWritableCacheDir();
    if (cachePath.isEmpty())
      return;

    QDir cacheDir(cachePath);

    QString filter = "msg_*.tmp";
    QStringList fileNames =
        cacheDir.entryList(QStringList() << filter, QDir::Files | QDir::Readable, QDir::Name);
    foreach (QString fileName, fileNames) {
      QString filePath = cacheDir.absoluteFilePath(fileName);
      saveChatLog(filePath, true);
    }
}

void lmcCore::saveChatLog(QString chatRoomId, bool isPath)
{
    bool saveHistory = Globals::getInstance().saveHistory();

    if (!saveHistory) {
        QString path = chatRoomId;

        if (!isPath) {
            path = StdLocation::getWritableCacheDir();
            path.append(QString("msg_%1.tmp").arg(chatRoomId));
        }

        QFile file(path);
        if (file.exists())
            file.remove();
        return;
    }

    QList<SingleMessage> messages;
    QString userName;
    QString userId;
    QDateTime date;
    qint64 timeSinceEpoch;
    QList<QString> peersList;

    QString path = chatRoomId;
    if (!isPath) {
        path = StdLocation::getWritableCacheDir();
        path.append(QString("msg_%1.tmp").arg(chatRoomId));
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QDataStream reader(&file);

    reader >> userName >> userId >> timeSinceEpoch >> peersList;
    date.setMSecsSinceEpoch(timeSinceEpoch);

    while (!reader.atEnd())
    {
        SingleMessage message;
        reader >> message;
        messages.append(message);
    }

    History::save(userName, userId, date, peersList, messages);

    file.close();
    file.remove();
}

void lmcCore::chatRoomWindow_closed(QString chatRoomId) {
  for (int index = _chatRoomWindows.count() - 1; index >= 0; --index) {
    if (_chatRoomWindows[index]->getRoomId().compare(chatRoomId) == 0) {
      lmcChatRoomWindow *chatWindow = _chatRoomWindows[index];
      saveChatLog(chatWindow->getOriginalRoomId());
      _chatRoomWindows.removeAt(index);
      chatWindow->deleteLater();
      break;
    }
  }
}

void lmcCore::instantMessageWindow_closed()
{
    InstantMessageWindow *window = static_cast<InstantMessageWindow *> (sender());
    _instantMessageWindows.removeAll(window);
}

void lmcCore::processMessage(MessageType type, const QString &userId,
                             const XmlMessage &message) {
  switch (type) {
  case MT_Announce:
    _mainWindow->addUser(_messaging->getUser(userId));
    processPublicMessage(type, userId, message);
    routeMessage(type, userId, message);
    break;
  case MT_Depart:
    _mainWindow->removeUser(userId);
    processPublicMessage(type, userId, message);
    routeMessage(type, userId, message);
    break;
  case MT_Status:
  case MT_UserName:
  case MT_Note:
    _mainWindow->updateUser(_messaging->getUser(userId));
    processPublicMessage(type, userId, message);
    routeMessage(type, userId, message);
    break;
  case MT_Avatar:
    _mainWindow->receiveMessage(type, userId, message);
    processPublicMessage(type, userId, message);
    routeMessage(type, userId, message);
    break;
  case MT_Message:
  case MT_GroupMessage:
  case MT_Broadcast:
  case MT_InstantMessage:
  case MT_UserList:
  case MT_ChatState:
  case MT_Failed:
    routeMessage(type, userId, message);
    break;
  case MT_PublicMessage:
    processPublicMessage(type, userId, message);
    break;
  case MT_Error:
    break;
  case MT_Query:
    showUserInfo(message);
    break;
  case MT_File:
  case MT_Folder:
    processFile(type, userId, message);
    break;
  case MT_Version:
  case MT_WebFailed:
    _updateWindow->receiveMessage(type, message);
    break;
  default:
    break;
  }
}

void lmcCore::processFile(MessageType type, const QString &userId,
                          const XmlMessage &message) {
  int fileOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
  int fileMode =
      Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));
  switch (fileOp) {
  case FO_Accept:
    initFileTransfer(type, static_cast<FileMode>(fileMode), userId, message);
    showTransferWindow();
    break;
  default:
    break;
  }
  if (fileOp != FO_Request && _transferWindow)
    _transferWindow->receiveMessage(message);

  routeMessage(type, userId, message);
}

void lmcCore::routeMessage(MessageType type, const QString &userId,
                           const XmlMessage &message) {
    bool windowExists = false;
    bool createWindow = type == MT_Message
            || type == MT_Broadcast
            || type == MT_InstantMessage
            || type == MT_Failed
            || type == MT_GroupMessage
            || ((type == MT_File || type == MT_Folder) && message.isValid() && message.data(XN_FILEOP) == FileOpNames[FO_Request]);
    bool setWindowToForeground = (Globals::getInstance().popOnNewMessage() && createWindow);

  if (userId.isEmpty()) {
      for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows) {
      if (type == MT_Status || type == MT_Note)
        chatRoomWindow->updateUser(_messaging->localUser);
      chatRoomWindow->receiveMessage(type, userId, message);
    }
  } else {
    QString chatRoomId = message.data(XN_THREAD);

    switch (type) {
    case MT_Announce:
        for(lmcChatRoomWindow* chatWindow : _chatRoomWindows)
            if(!chatWindow->getLastUserId().compare(userId) && chatWindow->getUsers().size() == 0) {
                User *user = _messaging->getUser(userId);
                if (user)
                    chatWindow->addUser(user);
            }

        for (InstantMessageWindow *window : _instantMessageWindows)
            if (!window->getPeerId().compare(userId))
                window->peerConnectionChanged(true);
        break;
    case MT_Depart:
      // If pMessage is NULL, the user is actually offline. Remove user
      // from all chat rooms.
      if (!message.isValid()) {
          for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
              if (chatRoomWindow->getUsers().contains(userId)) {
                  chatRoomWindow->removeUser(userId);
              }
      }

      for (InstantMessageWindow *window : _instantMessageWindows)
          if (!window->getPeerId().compare(userId))
              window->peerConnectionChanged(false);
      break;
    case MT_Status:
    case MT_UserName:
    case MT_Note:
        for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
            if (chatRoomWindow->getUsers().contains(userId)) {
                chatRoomWindow->updateUser(_messaging->getUser(userId));
                chatRoomWindow->receiveMessage(type, userId, message);
        }
      break;
    case MT_Avatar:
        for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
        if (chatRoomWindow->getUsers().contains(userId))
          chatRoomWindow->receiveMessage(type, userId, message);
      break;
    case MT_UserList:
    {
        QString userList = message.data(XN_USERID);

        for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows)
            if (!chatRoomWindow->getRoomId().compare(chatRoomId)) {
                if (userList.isEmpty()) {
                    // Send the current list of users in the chat room

                    QString userIds;
                    if (!chatRoomWindow->getUsers().isEmpty()) {
                        QHash<QString, QString>::const_iterator it = chatRoomWindow->getUsers().constBegin();
                        userIds = it.key();
                        ++it;

                        while (it != chatRoomWindow->getUsers().constEnd())
                            (userIds +=  QString(" | %1").arg(it.key()), ++it);
                    }

                    XmlMessage xmlMessage;
                    xmlMessage.addData(XN_THREAD, chatRoomId);
                    xmlMessage.addData(XN_USERID, userIds);

                    sendMessage(MT_UserList, userId, xmlMessage);
                } else {
                    // add the users to the chat room

                    QStringList userIds = userList.split(" | ", QString::SkipEmptyParts);
                    for (QString &userId : userIds) {
                        User *user = _messaging->getUser(userId);
                        if (user)
                            chatRoomWindow->addUser(user);
                    }
                }
            }
    }
        break;
    case MT_Broadcast:
    case MT_InstantMessage:
    {
        InstantMessageWindow *instantMessageWindow = new InstantMessageWindow(_messaging->localUser);
        instantMessageWindow->init(userId, message.data(XN_NAME), type, message);
        connect(instantMessageWindow, &InstantMessageWindow::chatStarting, this, &lmcCore::startChat);
        connect(instantMessageWindow, &InstantMessageWindow::messageSent, this, &lmcCore::sendMessage);
        connect(instantMessageWindow, &InstantMessageWindow::closed, this, &lmcCore::instantMessageWindow_closed);

        _instantMessageWindows.append(instantMessageWindow);
    }
        break;
    default:
    {
        if (message.isValid()) {
            QString tempUserId = !message.data(XN_USERID).isEmpty() ? message.data(XN_USERID) : userId;

            int op = Helper::indexOf(GroupMsgOpNames, GMO_Max, message.data(XN_GROUPMSGOP));
            if (op == GMO_Request or op == GMO_Join or op == GMO_Leave) {

                for(int index = 0; index < _chatRoomWindows.count(); index++) {
                    if(!_chatRoomWindows[index]->getRoomId().compare(chatRoomId)) {
                        switch(op) {
                        case GMO_Request:
                        case GMO_Join:
                            _chatRoomWindows[index]->addUser(_messaging->getUser(tempUserId));
                            return;
                            break;
                        case GMO_Leave:
                            _chatRoomWindows[index]->removeUser(tempUserId);
                            return;
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }

                if (op == GMO_Join or op == GMO_Leave)
                    return;
            }

            //	Check if a chat room with the thread id already exists
            for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows) {
                if (chatRoomWindow->getRoomId().compare(chatRoomId) == 0) {
                    if (!chatRoomWindow->getUsers().contains(tempUserId)) {
                        User *user = _messaging->getUser(tempUserId);
                        if (user)
                            chatRoomWindow->addUser(user);
                    }
                    chatRoomWindow->receiveMessage(type, tempUserId, message);
                    if((!chatRoomWindow->isPublicChat() or Globals::getInstance().popOnNewPublicMessage()) or !chatRoomWindow->isClosed())
                        if (createWindow)
                            showChatRoomWindow(chatRoomWindow, (Globals::getInstance().popOnNewMessage() and setWindowToForeground), setWindowToForeground);
                    windowExists = true;
                    break;
                }
            }

            if (!windowExists && (type == MT_Message || type == MT_Broadcast || type == MT_InstantMessage || type == MT_ChatState)) {
                for (lmcChatRoomWindow *chatRoomWindow : _chatRoomWindows) {
                    if (chatRoomWindow->getUsers().size() == 1 and chatRoomWindow->getUsers().contains(tempUserId)) {
                        if (type == MT_Message && chatRoomWindow->getRoomId().compare(chatRoomId))
                            chatRoomWindow->changeRoomId(chatRoomId);

                        chatRoomWindow->receiveMessage(type, tempUserId, message);
                        if((!chatRoomWindow->isPublicChat() or Globals::getInstance().popOnNewPublicMessage()) or !chatRoomWindow->isClosed())
                            if (createWindow)
                                showChatRoomWindow(chatRoomWindow, (Globals::getInstance().popOnNewMessage() and setWindowToForeground), setWindowToForeground);

                        windowExists = true;
                        break;
                    }
                }
            }

            if(!windowExists and createWindow and !chatRoomId.isEmpty()) {
                createChatRoomWindow(chatRoomId);

                if (type == MT_GroupMessage) {
                    XmlMessage xmlMessage;
                    xmlMessage.addData(XN_THREAD, chatRoomId);
                    xmlMessage.addData(XN_USERID, "");

                    sendMessage(MT_UserList, userId, xmlMessage);
                }

                showChatRoomWindow(_chatRoomWindows.last(), (Globals::getInstance().popOnNewMessage() and setWindowToForeground), setWindowToForeground);

                User *user = _messaging->getUser(tempUserId);
                if (user)
                    _chatRoomWindows.last()->addUser(user);


                if (type == MT_Message && Globals::getInstance().appendHistory())
                    _chatRoomWindows.last()->setHtml(History::getUserMessageHistory(tempUserId, QDate::currentDate()));

                _chatRoomWindows.last()->receiveMessage(type, tempUserId, message);
            }

        }
    }
        break;
    }
  }
}

void lmcCore::processPublicMessage(MessageType type, const QString &userId,
                                   const XmlMessage &message) {
  if (!_publicChatWindow)
    return;

  switch (type) {
  case MT_Announce:
    _publicChatWindow->addUser(_messaging->getUser(userId));
    break;
  case MT_Depart:
    _publicChatWindow->removeUser(userId);
    break;
  case MT_Status:
  case MT_UserName:
  case MT_Note:
    // lpszUserId can be NULL if sent by local user
    if (!userId.isEmpty())
      _publicChatWindow->updateUser(_messaging->getUser(userId));
    else
      _publicChatWindow->updateUser(_messaging->localUser);
    break;
  case MT_PublicMessage:
  {
      bool showWindow = type == MT_Message || type == MT_Broadcast || type == MT_InstantMessage || type == MT_Failed || type == MT_GroupMessage
          || ((type == MT_File || type == MT_Folder) && message.data(XN_FILEOP) == FileOpNames[FO_Request]);

    _publicChatWindow->receiveMessage(type, userId, message);
    showPublicChatWindow((Globals::getInstance().popOnNewPublicMessage() && Globals::getInstance().popOnNewMessage() && showWindow), Globals::getInstance().popOnNewPublicMessage(), Globals::getInstance().popOnNewPublicMessage());
  }
    break;
  case MT_Avatar:
    _publicChatWindow->receiveMessage(type, userId, message);
    break;
  default:
    break;
  }
}

void lmcCore::createTransferWindow() {
  if (!_transferWindow) {
    _transferWindow = new lmcTransferWindow();
    connect(_transferWindow,
            &lmcTransferWindow::messageSent, this,
            &lmcCore::sendMessage);
    connect(_transferWindow, &lmcTransferWindow::showTrayMessage,
            this, &lmcCore::showTrayMessage);
    _transferWindow->init();
  }
}

void lmcCore::showTransferWindow(bool show, QString userId) {
  bool autoShow = Globals::getInstance().openNewTransfers();
  bool bringToForeground = Globals::getInstance().popupNewTransfers();

  if ((autoShow && bringToForeground) || show) {
    //	if window is minimized it, restore it to previous state
    if (_transferWindow->windowState().testFlag(Qt::WindowMinimized))
      _transferWindow->setWindowState(_transferWindow->windowState() &
                                      ~Qt::WindowMinimized);
    _transferWindow->setWindowState(_transferWindow->windowState() |
                                    Qt::WindowActive);
    _transferWindow
        ->raise(); // make main window the top most window of the application
    _transferWindow->show();
    _transferWindow->activateWindow(); // bring window to foreground
  } else if (autoShow && !bringToForeground) {
    if (_transferWindow->isHidden())
      _transferWindow->setWindowState(_transferWindow->windowState() |
                                      Qt::WindowMinimized);
    _transferWindow->setWindowState(_transferWindow->windowState() |
                                    Qt::WindowActive);
    _transferWindow->show();
    qApp->alert(_transferWindow);
  }

  _transferWindow->setUserFilter(userId);
}

void lmcCore::initFileTransfer(MessageType type, FileMode mode,
                               const QString &userId, const XmlMessage &message) {
  createTransferWindow();

  User *pUser = _messaging->getUser(userId);
  if (!pUser)
      return;

  _transferWindow->createTransfer(type, mode, userId, pUser->name,
                                  message);
}

void lmcCore::showUserInfo(const XmlMessage &message) {
  if (!_userInfoWindow) {
    _userInfoWindow = new lmcUserInfoWindow();
    _userInfoWindow->init();
  }

  _userInfoWindow->setInfo(message);

  //	if window is minimized it, restore it to previous state
  if (_userInfoWindow->windowState().testFlag(Qt::WindowMinimized))
    _userInfoWindow->setWindowState(_userInfoWindow->windowState() &
                                    ~Qt::WindowMinimized);
  _userInfoWindow->setWindowState(_userInfoWindow->windowState() |
                                  Qt::WindowActive);
  _userInfoWindow
      ->raise(); // make window the top most window of the application
  _userInfoWindow->show();
  _userInfoWindow->activateWindow(); // bring window to foreground
}

void lmcCore::createChatRoomWindow(const QString &chatRoomId) {
  LoggerManager::getInstance().writeInfo(QString("lmcCore.createChatRoomWindow started-|- ChatRoomId: %1").arg(chatRoomId));

  //	create a new chat room with the specified thread id
  lmcChatRoomWindow *chatRoomWindow = new lmcChatRoomWindow();
  _chatRoomWindows.append(chatRoomWindow);

  connect(chatRoomWindow, &lmcChatRoomWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(chatRoomWindow, &lmcChatRoomWindow::contactsAdding, this,
          &lmcCore::addContacts);
  connect(chatRoomWindow, &lmcChatRoomWindow::chatStarting, this,
          &lmcCore::startChat);
  connect(chatRoomWindow, &lmcChatRoomWindow::showHistory, this,
          &lmcCore::showHistory);
  connect(chatRoomWindow, &lmcChatRoomWindow::showTransfers, this,
          &lmcCore::showTransfers);
  connect(chatRoomWindow, &lmcChatRoomWindow::showTrayMessage,
          this, &lmcCore::showTrayMessage);
  connect(chatRoomWindow, &lmcChatRoomWindow::closed, this,
          &lmcCore::chatRoomWindow_closed);

  User *pLocalUser = _messaging->localUser;
  chatRoomWindow->init(pLocalUser, _messaging->isConnected(), chatRoomId);

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.createChatRoomWindow ended"));
}

void lmcCore::showChatRoomWindow(lmcChatRoomWindow *chatRoomWindow, bool show,
                                 bool alert, bool add,
                                 QStringList selectedContacts) {
  // if show or add is specified, bring to top
  if (show || add) {
    if (chatRoomWindow->windowState().testFlag(Qt::WindowMinimized))
      chatRoomWindow->setWindowState(chatRoomWindow->windowState() &
                                     ~Qt::WindowMinimized);
    chatRoomWindow->show();
    chatRoomWindow->activateWindow();
  } else {
    if (chatRoomWindow->isHidden())
      chatRoomWindow->setWindowState(chatRoomWindow->windowState() |
                                     Qt::WindowMinimized);
    chatRoomWindow->show();
    if (alert) {
        chatRoomWindow->setWindowState((chatRoomWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        chatRoomWindow->raise();
        chatRoomWindow->activateWindow();
    }
  }

  if (add) {
      if (selectedContacts.isEmpty()) {
          QStringList excludeList(_messaging->localUser->id);
          selectedContacts =
                  showSelectContacts(chatRoomWindow, UC_GroupMessage, excludeList);
      }

      // if no contacts were selected, close the chat room window
      if (selectedContacts.count() == 0) {
          _chatRoomWindows.removeOne(chatRoomWindow);
          chatRoomWindow->close();
          chatRoomWindow->deleteLater();
      } else {
          std::vector<User *> users;

          for (QString &userId : selectedContacts) {
              User *tempUser = _messaging->getUser(userId);
              if (tempUser)
                  users.push_back(tempUser);
          }
          chatRoomWindow->selectContacts(users);
      }
  }
}

void lmcCore::showPublicChatWindow(bool show, bool alert, bool open) {
  if (show) {
    if (_publicChatWindow->windowState().testFlag(Qt::WindowMinimized))
      _publicChatWindow->setWindowState(_publicChatWindow->windowState() &
                                        ~Qt::WindowMinimized);
    _publicChatWindow->show();
    _publicChatWindow->activateWindow();
  } else {
    if (open) {
      if (_publicChatWindow->isHidden())
        _publicChatWindow->setWindowState(_publicChatWindow->windowState() |
                                          Qt::WindowMinimized);
      _publicChatWindow->show();
    }
    if (alert)
      qApp->alert(_publicChatWindow);
  }
}

QStringList lmcCore::showSelectContacts(QWidget *parent, uint caps,
                                        const QStringList &excludeList) {
  QStringList selectedContacts;
  _userSelectDialog = new lmcUserSelectDialog(parent);

  QList<QTreeWidgetItem *> contactsList = _mainWindow->getContactsList();
  for (int index = 0; index < contactsList.count(); index++) {
    QTreeWidgetItem *item = contactsList.value(index);
    for (int childIndex = 0; childIndex < item->childCount(); childIndex++) {
      QTreeWidgetItem *pChildItem = item->child(childIndex);
      QString userId = pChildItem->data(0, IdRole).toString();
      User *pUser = _messaging->getUser(userId);
      if (!pUser)
          continue;
      if ((pUser->caps & caps) != caps) {
        item->removeChild(pChildItem);
        childIndex--;
        continue;
      }
      if (excludeList.contains(userId)) {
        item->removeChild(pChildItem);
        childIndex--;
        continue;
      }
    }
  }

  _userSelectDialog->init(contactsList);
  if (_userSelectDialog->exec() != QDialog::Rejected)
    selectedContacts = _userSelectDialog->selectedContacts;
  _userSelectDialog->deleteLater();

  return selectedContacts;
}

void lmcCore::showPortConflictMessage() {
  //	show message box
  QMessageBox msgBox;
  msgBox.setWindowTitle(lmcStrings::appName());
  msgBox.setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));
  msgBox.setIcon(QMessageBox::Critical);
  QString msg =
      tr("A port address conflict has been detected. %1 will close now.").arg(lmcStrings::appName());
  msgBox.setText(msg);
  QString detail =
      tr("%1 cannot start because another application is using the port "
         "configured for use with %1.").arg(lmcStrings::appName());
  msgBox.setDetailedText(detail);
  msgBox.exec();
}
