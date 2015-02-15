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
#include "broadcastdisplaywindow.h"

#include <vector>
#include <QMessageBox>
#include <QTranslator>

lmcCore::lmcCore() {
  pMessaging = new lmcMessaging();
  connect(pMessaging, &lmcMessaging::messageReceived, this,
          &lmcCore::receiveMessage);
  connect(pMessaging, &lmcMessaging::connectionStateChanged, this,
          &lmcCore::connectionStateChanged);

  pMainWindow = new lmcMainWindow();
  connect(pMainWindow, &lmcMainWindow::appExiting, this, &lmcCore::exitApp);
  connect(pMainWindow, &lmcMainWindow::chatRoomStarting, this,
          &lmcCore::startChat);
  connect(pMainWindow, &lmcMainWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(pMainWindow, &lmcMainWindow::showTransfers, this,
          &lmcCore::showTransfers);
  connect(pMainWindow, &lmcMainWindow::showMessage, this, &lmcCore::showMessage);
  connect(pMainWindow, &lmcMainWindow::showHistory, this,
          &lmcCore::showHistory);
  connect(pMainWindow, &lmcMainWindow::showSettings, this,
          &lmcCore::showSettings);
  connect(pMainWindow, &lmcMainWindow::showHelp, this, &lmcCore::showHelp);
  connect(pMainWindow, &lmcMainWindow::showUpdate, this, &lmcCore::showUpdate);
  connect(pMainWindow, &lmcMainWindow::showAbout, this, &lmcCore::showAbout);
  connect(pMainWindow, &lmcMainWindow::showBroadcast, this,
          &lmcCore::showBroadcast);
  connect(pMainWindow, &lmcMainWindow::showPublicChat, this,
          &lmcCore::showPublicChat);
  connect(pMainWindow, &lmcMainWindow::groupUpdated, this,
          &lmcCore::updateGroup);

  pPublicChatWindow = new lmcChatRoomWindow(0, true);
  connect(pPublicChatWindow, &lmcChatRoomWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(pPublicChatWindow, &lmcChatRoomWindow::chatStarting, this,
          &lmcCore::startChat);

  chatRoomWindows.clear();
  pTransferWindow = NULL;
  pHistoryWindow = NULL;
  pSettingsDialog = NULL;
  pUserInfoWindow = NULL;
  pHelpWindow = NULL;
  pUpdateWindow = NULL;
  pUserSelectDialog = NULL;
  pAboutDialog = NULL;
  pBroadcastWindow = NULL;
  pTimer = NULL;
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

  pInitParams = new XmlMessage();
  if (arguments.contains("/silent", Qt::CaseInsensitive))
    pInitParams->addData(XN_SILENTMODE, LMC_TRUE);
  if (arguments.contains("/trace", Qt::CaseInsensitive)) {
    pInitParams->addData(XN_TRACEMODE, LMC_TRUE);
    pInitParams->addData(XN_LOGFILE, StdLocation::freeLogFile());
  }
  for (int index = 0; index < arguments.count(); index++) {
    if (arguments.at(index).startsWith("/port=", Qt::CaseInsensitive)) {
      QString port = arguments.at(index).mid(QString("/port=").length());
      pInitParams->addData(XN_PORT, port);
      continue;
    }
    if (arguments.at(index).startsWith("/config=", Qt::CaseInsensitive)) {
      QString configFile =
          arguments.at(index).mid(QString("/config=").length());
      pInitParams->addData(XN_CONFIG, configFile);
      continue;
    }
  }

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init -|- Application initialized"));

  loadSettings();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init -|- Settings loaded"));

  pMessaging->init(pInitParams);
  pMainWindow->init(pMessaging->localUser, &pMessaging->groupList,
                    pMessaging->isConnected());
  pPublicChatWindow->init(pMessaging->localUser, pMessaging->isConnected());

  saveUnsavedChatLog();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.init ended"));
}

bool lmcCore::start() {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.start started"));

  pMessaging->start();

  if (pMessaging->isConnected() && !pMessaging->canReceive()) {
    showPortConflictMessage();
    //	stop the application
    stop();

    LoggerManager::getInstance().writeInfo(
        QString("lmcCore.start-|- A port address conflict has been detected. "
                "%1 will close now.").arg(lmcStrings::appName()));

    return false;
  }

  pMainWindow->start();

  pTimer = new QTimer(this);
  connect(pTimer, SIGNAL(timeout()), this, SLOT(timer_timeout()));
  //	Set the timer to trigger 10 seconds after the application starts. After
  // the
  //	first trigger, the timeout period will be decided by user settings.
  adaptiveRefresh = false;
  pTimer->start(10000);
  bool autoStart =
      pSettings->value(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL).toBool();
  lmcSettings::setAutoStart(autoStart);

  pMainWindow->restore();

  LoggerManager::getInstance().writeInfo(QStringLiteral("lmcCore.start ended"));

  return true;
}

//	This is the initial point where settings are used in the application
void lmcCore::loadSettings() {
  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.loadSettings started"));

  pSettings = new lmcSettings();
  bool silent = Helper::stringToBool(pInitParams->data(XN_SILENTMODE));
  if (pInitParams->dataExists(XN_CONFIG)) {
    QString configFile = pInitParams->data(XN_CONFIG);
    if (!pSettings->loadFromConfig(configFile) && !silent) {
      QString message = tr("Preferences could not be imported from '%1'.\n\n"
                           "File may not exist, or may not be compatible with "
                           "this version of %2.");
      QMessageBox::warning(NULL, lmcStrings::appName(),
                           message.arg(configFile, lmcStrings::appName()));
    }
  }
  lang = pSettings->value(IDS_LANGUAGE, IDS_LANGUAGE_DEFAULT_VAL).toString();
  Application::setLanguage(lang);
  Application::setLayoutDirection(
      tr("LAYOUT_DIRECTION") == RTL_LAYOUT ? Qt::RightToLeft : Qt::LeftToRight);
  messagePop = pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool();
  pubMessagePop =
      pSettings->value(IDS_PUBMESSAGEPOP, IDS_PUBMESSAGEPOP_VAL).toBool();
  refreshTime =
      pSettings->value(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL).toInt() * 1000;

  QString chatThemeName = pSettings->value(IDS_THEME, IDS_THEME_VAL).toString();
  QString appThemeName =
      pSettings->value(IDS_APPTHEME, IDS_APPTHEME_VAL).toString();
  QString buttonThemeName =
      pSettings->value(IDS_BUTTONTHEME, IDS_BUTTONTHEME_VAL).toString();
  QString appIconThemeName =
      pSettings->value(IDS_APPICONTHEME, IDS_APPICONTHEME_VAL).toString();

  ThemeManager::getInstance().changeChatTheme(chatThemeName);
  ThemeManager::getInstance().changeTheme(appThemeName, buttonThemeName);
  ThemeManager::getInstance().setIconTheme(appIconThemeName);

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcCore.loadSettings ended"));
}

void lmcCore::settingsChanged() {
  pMessaging->settingsChanged();
  pMainWindow->settingsChanged();
  if (pPublicChatWindow)
    pPublicChatWindow->settingsChanged();
  for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
    chatRoomWindow->settingsChanged();
  if (pTransferWindow)
    pTransferWindow->settingsChanged();
  if (pUserInfoWindow)
    pUserInfoWindow->settingsChanged();
  if (pHistoryWindow)
    pHistoryWindow->settingsChanged();
  if (pSettingsDialog)
    pSettingsDialog->settingsChanged();
  if (pHelpWindow)
    pHelpWindow->settingsChanged();
  if (pAboutDialog)
    pAboutDialog->settingsChanged();
  if (pBroadcastWindow)
    pBroadcastWindow->settingsChanged();

  messagePop = pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool();
  pubMessagePop =
      pSettings->value(IDS_PUBMESSAGEPOP, IDS_PUBMESSAGEPOP_VAL).toBool();
  refreshTime =
      pSettings->value(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL).toInt() * 1000;
  pTimer->setInterval(refreshTime);
  bool autoStart =
      pSettings->value(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL).toBool();
  lmcSettings::setAutoStart(autoStart);
  QString appLang =
      pSettings->value(IDS_LANGUAGE, IDS_LANGUAGE_DEFAULT_VAL).toString();
  if (appLang.compare(lang) != 0) {
    lang = appLang;
    Application::setLanguage(lang);
    Application::setLayoutDirection(tr("LAYOUT_DIRECTION") == RTL_LAYOUT
                                        ? Qt::RightToLeft
                                        : Qt::LeftToRight);
    lmcStrings::retranslate();
  }
}

void lmcCore::stop() {
    LoggerManager::getInstance().writeInfo("lmcCore.stop started");

    for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows) {
        chatRoomWindow->stop();
        chatRoomWindow->deleteLater();
    }

  if (pTransferWindow) {
    pTransferWindow->stop();
    pTransferWindow->deleteLater();
  }

  if (pHistoryWindow) {
    pHistoryWindow->stop();
    pHistoryWindow->deleteLater();
  }

  if (pUserInfoWindow)
    pUserInfoWindow->deleteLater();

  if (pHelpWindow) {
    pHelpWindow->stop();
    pHelpWindow->deleteLater();
  }

  if (pUpdateWindow) {
    pUpdateWindow->stop();
    pUpdateWindow->deleteLater();
  }

  if (pBroadcastWindow) {
    pBroadcastWindow->stop();
    pBroadcastWindow->deleteLater();
  }

  if (pPublicChatWindow) {
    pPublicChatWindow->stop();
    pPublicChatWindow->deleteLater();
  }

  if (pTimer)
    pTimer->stop();

  pMessaging->stop();
  pMainWindow->stop();

  LoggerManager::getInstance().writeInfo("lmcCore.stop ended");
}

//	This slot handles the exit signal emitted by main window when the user
//	selects quit from the menu.
void lmcCore::exitApp() { saveUnsavedChatLog(); qApp->quit(); }

//	This slot handles the signal emitted by QApplication when the
// application
//	quits either by user interaction or from operating system signal.
void lmcCore::aboutToExit() {
  stop();
  pSettings->setValue(IDS_VERSION, IDA_VERSION);

  LoggerManager::getInstance().writeInfo(QStringLiteral("Application exit"));

  pSettings->sync();
}

void lmcCore::timer_timeout() {
  //	Refresh the contacts list whenever the timer triggers
  pMessaging->update();
  if (adaptiveRefresh) {
    //	The refresh interval is doubled until the refresh time defined by user
    // is reached.
    //	Then keep refreshing at that interval.
    int nextInterval = pTimer->interval() * 2;
    int maxInterval =
        pSettings->value(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL).toInt() * 1000;
    int interval = qMin(nextInterval, maxInterval);
    adaptiveRefresh = (nextInterval >= maxInterval) ? false : true;
    pTimer->setInterval(interval);
  } else if (refreshTime > pTimer->interval()) {
    pTimer->setInterval(refreshTime);
  }
}

void lmcCore::startChat(QString roomId, XmlMessage message, QStringList contacts) {
    lmcChatRoomWindow *chatWindow = nullptr;
    bool alert = pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool();
    if (contacts.size() == 1) {
        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
            if (chatRoomWindow->getUsers().size() == 1 and chatRoomWindow->getUsers().contains(contacts[0]))
                chatWindow = chatRoomWindow;
    }

    if (!chatWindow) {
        createChatRoomWindow(roomId);
        chatWindow = chatRoomWindows.last();
        alert = true;
    }

    showChatRoomWindow(chatWindow, true, alert, true, contacts);

    if (chatWindow->getUsers().size() == 1 && pSettings->value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool()) {
        chatWindow->setHtml(History::getUserMessageHistory(chatWindow->getUsers().keys().first(), QDate::currentDate()));
    }

    if (message.isValid() and (message.dataExists(XN_MESSAGE) or message.dataExists(XN_BROADCAST))
            and contacts.size() == 1) { // used only for broadcast for now
        chatWindow->receiveMessage(MT_Broadcast, &contacts.first(), &message);
    }
}

void lmcCore::sendMessage(MessageType type, QString *lpszUserId,
                          XmlMessage *pMessage) {
  switch (type) {
  case MT_Broadcast:
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
    pMessaging->sendMessage(type, lpszUserId, pMessage);
    break;
  case MT_Status:
  case MT_Note:
    pMessaging->sendMessage(type, lpszUserId, pMessage);
    break;
  case MT_Refresh:
    pMessaging->update();
    break;
  default:
    break;
  }
}

void lmcCore::receiveMessage(MessageType type, QString *lpszUserId,
                             XmlMessage *pMessage) {
  processMessage(type, lpszUserId, pMessage);
}

bool lmcCore::receiveAppMessage(const QString &szMessage) {
  LoggerManager::getInstance().writeInfo(
      QString("lmcCore.receiveAppMessage started -|- messageSize: %1 - %2")
          .arg(QString::number(szMessage.size()), szMessage));

  bool doNotExit = true;

  if (szMessage.isEmpty()) {
    pMainWindow->restore();
    return doNotExit;
  }

  QStringList messageList = szMessage.split("\n", QString::SkipEmptyParts);
  //	remove duplicates
  messageList = messageList.toSet().toList();

  if (messageList.contains("/new", Qt::CaseInsensitive)) {
    if (messageList.contains("/loopback", Qt::CaseInsensitive))
      pMessaging->setLoopback(true);
  }
  if (messageList.contains("/nohistory", Qt::CaseInsensitive)) {
      QDir dir(History::historyFilesDir());
      dir.setNameFilters(QStringList() << "*.xml");
      dir.setFilter(QDir::Files);
      foreach(QString dirFile, dir.entryList())
          dir.remove(dirFile);

    if (pHistoryWindow)
      pHistoryWindow->updateList();
  }
  if (messageList.contains("/nofilehistory", Qt::CaseInsensitive)) {
    // TODO maybe it shouldn't delete the files...
    QFile::remove(StdLocation::transferHistoryFilePath());
    if (pTransferWindow)
      pTransferWindow->updateList();
  }
  if (messageList.contains("/noconfig", Qt::CaseInsensitive)) {
    // TODO this shouldn't remove the avatar file...
    // QFile::remove(StdLocation::avatarFile());
   // QFile::remove(pSettings->fileName());
    pSettings->sync();
    settingsChanged();
  }
  if (messageList.contains("/sync", Qt::CaseInsensitive)) {
    bool autoStart =
        pSettings->value(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL).toBool();
    lmcSettings::setAutoStart(autoStart);
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
  bool connected = pMessaging->isConnected();

  pMainWindow->connectionStateChanged(connected);
  if (pPublicChatWindow)
    pPublicChatWindow->connectionStateChanged(connected);
  for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
    chatRoomWindow->connectionStateChanged(connected);
  if (pBroadcastWindow)
    pBroadcastWindow->connectionStateChanged(connected);

  if (pMessaging->isConnected() && !pMessaging->canReceive()) {
    showPortConflictMessage();
    exitApp();
  }
}

void lmcCore::showTransfers(QString userId) {
  createTransferWindow();
  showTransferWindow(true, userId);
}

void lmcCore::showMessage(QString chatRoomId) {
    if (!chatRoomId.isEmpty()) {
        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
            if (chatRoomWindow->getRoomId().compare(chatRoomId) == 0)
                showChatRoomWindow(chatRoomWindow, true, true);
    }
}

void lmcCore::showHistory(QString userId) {
  if (!pHistoryWindow) {
    pHistoryWindow = new lmcHistoryWindow();
    pHistoryWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (pHistoryWindow->windowState().testFlag(Qt::WindowMinimized))
    pHistoryWindow->setWindowState(pHistoryWindow->windowState() &
                                   ~Qt::WindowMinimized);
  pHistoryWindow->setWindowState(pHistoryWindow->windowState() |
                                 Qt::WindowActive);
  pHistoryWindow->raise(); // make window the top most window of the application
  pHistoryWindow->show();
  pHistoryWindow->activateWindow(); // bring window to foreground
  pHistoryWindow->setUserFilter(userId);
}

void lmcCore::showSettings() {
  if (!pSettingsDialog) {
    pSettingsDialog = new lmcSettingsDialog();
    connect(pSettingsDialog, SIGNAL(historyCleared()), this,
            SLOT(historyCleared()));
    connect(pSettingsDialog, SIGNAL(fileHistoryCleared()), this,
            SLOT(fileHistoryCleared()));
    pSettingsDialog->init();
  }

  if (pSettingsDialog->exec())
    settingsChanged();
}

void lmcCore::showHelp(QRect *pRect) {
  if (!pHelpWindow) {
    pHelpWindow = new lmcHelpWindow(pRect);
    pHelpWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (pHelpWindow->windowState().testFlag(Qt::WindowMinimized))
    pHelpWindow->setWindowState(pHelpWindow->windowState() &
                                ~Qt::WindowMinimized);
  pHelpWindow->setWindowState(pHelpWindow->windowState() | Qt::WindowActive);
  pHelpWindow->raise(); // make window the top most window of the application
  pHelpWindow->show();
  pHelpWindow->activateWindow(); // bring window to foreground
}

void lmcCore::showUpdate(QRect *pRect) {
  if (!pUpdateWindow) {
    pUpdateWindow = new lmcUpdateWindow(pRect);
    connect(pUpdateWindow,
            &lmcUpdateWindow::messageSent, this, &lmcCore::sendMessage);
    pUpdateWindow->init();
  }

  //	if window is minimized it, restore it to previous state
  if (pUpdateWindow->windowState().testFlag(Qt::WindowMinimized))
    pUpdateWindow->setWindowState(pUpdateWindow->windowState() &
                                  ~Qt::WindowMinimized);
  pUpdateWindow->setWindowState(pUpdateWindow->windowState() |
                                Qt::WindowActive);
  pUpdateWindow->raise(); // make window the top most window of the application
  pUpdateWindow->show();
  pUpdateWindow->activateWindow(); // bring window to foreground
}

void lmcCore::showAbout() {
  if (!pAboutDialog) {
    pAboutDialog = new lmcAboutDialog(pMainWindow);
    pAboutDialog->init();
  }

  pAboutDialog->exec();
}

void lmcCore::showBroadcast() {
  if (!pBroadcastWindow) {
    pBroadcastWindow = new lmcBroadcastWindow(pMessaging->localUser);
    connect(pBroadcastWindow,
            &lmcBroadcastWindow::messageSent, this,
            &lmcCore::sendMessage);
    pBroadcastWindow->init(pMessaging->isConnected());
  }

  if (pBroadcastWindow->isHidden()) {
    QList<QTreeWidgetItem*> contactsList = pMainWindow->getContactsList();
    QStringList users = pMainWindow->getSelectedUserIds();

    pBroadcastWindow->show(contactsList, users);
  } else {
    pBroadcastWindow->show();
  }
}

void lmcCore::showPublicChat() {
  //	Show public chat window
  showPublicChatWindow(true);
}

void lmcCore::historyCleared() {
  if (pHistoryWindow)
    pHistoryWindow->updateList();
}

void lmcCore::fileHistoryCleared() {
  if (pTransferWindow)
    pTransferWindow->updateList();
}

void lmcCore::showTrayMessage(TrayMessageType type, QString szMessage, QString chatRoomId,
                              QString szTitle, TrayMessageIcon icon) {
  pMainWindow->showTrayMessage(type, szMessage, chatRoomId, szTitle, icon);
}

void lmcCore::updateGroup(GroupOp op, QVariant value1, QVariant value2) {
  pMessaging->updateGroup(op, value1, value2);
}

void lmcCore::addContacts(QStringList *pExcludList) {
  lmcChatRoomWindow *chatRoomWindow =
      static_cast<lmcChatRoomWindow *>(sender());
  QStringList selectedContacts =
      showSelectContacts(chatRoomWindow, UC_GroupMessage, pExcludList);

  std::vector<User *> users;

  for (QString &userId : selectedContacts) {
      User *tempUser = pMessaging->getUser(&userId);
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
    bool saveHistory = pSettings->value(IDS_HISTORY, IDS_HISTORY_VAL).toBool();

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

void lmcCore::chatRoomWindow_closed(QString *lpszThreadId) {
  for (int index = chatRoomWindows.count() - 1; index >= 0; --index) {
    if (chatRoomWindows[index]->getRoomId().compare(*lpszThreadId) == 0) {
      lmcChatRoomWindow *chatWindow = chatRoomWindows[index];
      saveChatLog(chatWindow->getOriginalRoomId());
      chatRoomWindows.removeAt(index);
      chatWindow->deleteLater();
      break;
    }
  }
}

void lmcCore::processMessage(MessageType type, QString *lpszUserId,
                             XmlMessage *pMessage) {
  switch (type) {
  case MT_Announce:
    pMainWindow->addUser(pMessaging->getUser(lpszUserId));
    processPublicMessage(type, lpszUserId, pMessage);
    routeMessage(type, lpszUserId, pMessage);
    break;
  case MT_Depart:
    pMainWindow->removeUser(lpszUserId);
    processPublicMessage(type, lpszUserId, pMessage);
    routeMessage(type, lpszUserId, pMessage);
    break;
  case MT_Status:
  case MT_UserName:
  case MT_Note:
    pMainWindow->updateUser(pMessaging->getUser(lpszUserId));
    processPublicMessage(type, lpszUserId, pMessage);
    routeMessage(type, lpszUserId, pMessage);
    break;
  case MT_Avatar:
    pMainWindow->receiveMessage(type, lpszUserId, pMessage);
    processPublicMessage(type, lpszUserId, pMessage);
    routeMessage(type, lpszUserId, pMessage);
    break;
  case MT_Message:
  case MT_GroupMessage:
  case MT_Broadcast:
  case MT_UserList:
  case MT_ChatState:
  case MT_Failed:
    routeMessage(type, lpszUserId, pMessage);
    break;
  case MT_PublicMessage:
    processPublicMessage(type, lpszUserId, pMessage);
    break;
  case MT_Error:
    break;
  case MT_Query:
    showUserInfo(pMessage);
    break;
  case MT_File:
  case MT_Folder:
    processFile(type, lpszUserId, pMessage);
    break;
  case MT_Version:
  case MT_WebFailed:
    pUpdateWindow->receiveMessage(type, lpszUserId, pMessage);
    break;
  default:
    break;
  }
}

void lmcCore::processFile(MessageType type, QString *lpszUserId,
                          XmlMessage *pMessage) {
  int fileOp = Helper::indexOf(FileOpNames, FO_Max, pMessage->data(XN_FILEOP));
  int fileMode =
      Helper::indexOf(FileModeNames, FM_Max, pMessage->data(XN_MODE));
  switch (fileOp) {
  case FO_Accept:
    initFileTransfer(type, (FileMode)fileMode, lpszUserId, pMessage);
    showTransferWindow();
    break;
  default:
    break;
  }
  if (fileOp != FO_Request && pTransferWindow)
    pTransferWindow->receiveMessage(type, lpszUserId, pMessage);

  routeMessage(type, lpszUserId, pMessage);
}

void lmcCore::routeMessage(MessageType type, QString *lpszUserId,
                           XmlMessage *pMessage) {
    bool windowExists = false;
    bool createWindow = type == MT_Message
            || type == MT_Broadcast
            || type == MT_Failed
            || type == MT_GroupMessage
            || ((type == MT_File || type == MT_Folder) && pMessage && pMessage->data(XN_FILEOP) == FileOpNames[FO_Request]);
    bool setWindowToForeground = (pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool() && createWindow);

  if (!lpszUserId) {
      for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows) {
      if (type == MT_Status || type == MT_Note)
        chatRoomWindow->updateUser(pMessaging->localUser);
      chatRoomWindow->receiveMessage(type, lpszUserId, pMessage);
    }
  } else {
    QString chatRoomId = pMessage ? pMessage->data(XN_THREAD) : "";

    switch (type) {
    case MT_Announce:
        for(lmcChatRoomWindow* chatWindow : chatRoomWindows)
            if(!chatWindow->getLastUserId().compare(*lpszUserId) && chatWindow->getUsers().size() == 0) {
                User *user = pMessaging->getUser(lpszUserId);
                if (user)
                    chatWindow->addUser(user);
            }
        break;
    case MT_Depart:
      // If pMessage is NULL, the user is actually offline. Remove user
      // from all chat rooms.
      if (!pMessage) {
          for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
              if (chatRoomWindow->getUsers().contains(*lpszUserId))
                  chatRoomWindow->removeUser(lpszUserId);
      }
      break;
    case MT_Status:
    case MT_UserName:
    case MT_Note:
        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
            if (chatRoomWindow->getUsers().contains(*lpszUserId)) {
                chatRoomWindow->updateUser(pMessaging->getUser(lpszUserId));
                chatRoomWindow->receiveMessage(type, lpszUserId, pMessage);
        }
      break;
    case MT_Avatar:
        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
        if (chatRoomWindow->getUsers().contains(*lpszUserId))
          chatRoomWindow->receiveMessage(type, lpszUserId, pMessage);
      break;
    case MT_UserList:
    {
        QString userList = pMessage ? pMessage->data(XN_USERID) : "";

        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows)
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

                    sendMessage(MT_UserList, lpszUserId, &xmlMessage);
                } else {
                    // add the users to the chat room

                    QStringList userIds = userList.split(" | ", QString::SkipEmptyParts);
                    for (QString &userId : userIds) {
                        User *user = pMessaging->getUser(&userId);
                        if (user)
                            chatRoomWindow->addUser(user);
                    }
                }
            }
    }
        break;
    case MT_Broadcast:
    {
        broadcastDisplayWindow *broadcastWindow = new broadcastDisplayWindow(pMessaging->localUser);
        broadcastWindow->init(*lpszUserId, pMessage->data(XN_NAME), *pMessage);
        connect(broadcastWindow, &broadcastDisplayWindow::chatStarting, this, &lmcCore::startChat);
    }
        break;
    default:
    {
        if (pMessage) {
            int op = Helper::indexOf(GroupMsgOpNames, GMO_Max, pMessage->data(XN_GROUPMSGOP));
            if (op == GMO_Request or op == GMO_Join or op == GMO_Leave) {
                QString userId = pMessage && !pMessage->data(XN_USERID).isEmpty() ? pMessage->data(XN_USERID) : *lpszUserId;

                for(int index = 0; index < chatRoomWindows.count(); index++) {
                    if(!chatRoomWindows[index]->getRoomId().compare(chatRoomId)) {
                        switch(op) {
                        case GMO_Request:
                        case GMO_Join:
                            chatRoomWindows[index]->addUser(pMessaging->getUser(&userId));
                            return;
                            break;
                        case GMO_Leave:
                            chatRoomWindows[index]->removeUser(&userId);
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
        }

        //	Check if a chat room with the thread id already exists
        for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows) {
            if (chatRoomWindow->getRoomId().compare(chatRoomId) == 0) {
                if (!chatRoomWindow->getUsers().contains(*lpszUserId)) {
                    User *user = pMessaging->getUser(lpszUserId);
                    if (user)
                        chatRoomWindow->addUser(user);
                }
                chatRoomWindow->receiveMessage(type, lpszUserId, pMessage);
                if((!chatRoomWindow->isPublicChat() or pubMessagePop) or !chatRoomWindow->isClosed())
                    if (createWindow)
                        showChatRoomWindow(chatRoomWindow, (messagePop and setWindowToForeground), setWindowToForeground);
                windowExists = true;
                break;
            }
        }

        if (!windowExists and (type == MT_Message or type == MT_Broadcast)) {
            for (lmcChatRoomWindow *chatRoomWindow : chatRoomWindows) {
                if (chatRoomWindow->getUsers().size() == 1)
                if (chatRoomWindow->getUsers().size() == 1 and chatRoomWindow->getUsers().contains(*lpszUserId)) {
                    if (type == MT_Message)
                        chatRoomWindow->changeRoomId(chatRoomId);

                    chatRoomWindow->receiveMessage(type, lpszUserId, pMessage);
                    if((!chatRoomWindow->isPublicChat() or pubMessagePop) or !chatRoomWindow->isClosed())
                        if (createWindow)
                            showChatRoomWindow(chatRoomWindow, (messagePop and setWindowToForeground), setWindowToForeground);

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

                sendMessage(MT_UserList, lpszUserId, &xmlMessage);
            }

            showChatRoomWindow(chatRoomWindows.last(), (messagePop and setWindowToForeground), setWindowToForeground);

            if (type == MT_Message && pSettings->value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool()) {
                chatRoomWindows.last()->setHtml(History::getUserMessageHistory(*lpszUserId, QDate::currentDate()));
            }

            User *user = pMessaging->getUser(lpszUserId);
            if (user)
                chatRoomWindows.last()->addUser(user);

            chatRoomWindows.last()->receiveMessage(type, lpszUserId, pMessage);
        }

    }
        break;
    }
  }
}

void lmcCore::processPublicMessage(MessageType type, QString *lpszUserId,
                                   XmlMessage *pMessage) {
  if (!pPublicChatWindow)
    return;

  switch (type) {
  case MT_Announce:
    pPublicChatWindow->addUser(pMessaging->getUser(lpszUserId));
    break;
  case MT_Depart:
    pPublicChatWindow->removeUser(lpszUserId);
    break;
  case MT_Status:
  case MT_UserName:
  case MT_Note:
    // lpszUserId can be NULL if sent by local user
    if (lpszUserId)
      pPublicChatWindow->updateUser(pMessaging->getUser(lpszUserId));
    else
      pPublicChatWindow->updateUser(pMessaging->localUser);
    break;
  case MT_PublicMessage:
  {
      bool showWindow = type == MT_Message || type == MT_Broadcast || type == MT_Failed || type == MT_GroupMessage
          || ((type == MT_File || type == MT_Folder) && pMessage->data(XN_FILEOP) == FileOpNames[FO_Request]);

    pPublicChatWindow->receiveMessage(type, lpszUserId, pMessage);
    showPublicChatWindow((pubMessagePop && messagePop && showWindow), pubMessagePop, pubMessagePop);
  }
    break;
  case MT_Avatar:
    pPublicChatWindow->receiveMessage(type, lpszUserId, pMessage);
    break;
  default:
    break;
  }
}

void lmcCore::createTransferWindow() {
  if (!pTransferWindow) {
    pTransferWindow = new lmcTransferWindow();
    connect(pTransferWindow,
            &lmcTransferWindow::messageSent, this,
            &lmcCore::sendMessage);
    connect(pTransferWindow, &lmcTransferWindow::showTrayMessage,
            this, &lmcCore::showTrayMessage);
    pTransferWindow->init();
  }
}

void lmcCore::showTransferWindow(bool show, QString userId) {
  bool autoShow =
      pSettings->value(IDS_AUTOSHOWFILE, IDS_AUTOSHOWFILE_VAL).toBool();
  bool bringToForeground =
      pSettings->value(IDS_FILETOP, IDS_FILETOP_VAL).toBool();

  if ((autoShow && bringToForeground) || show) {
    //	if window is minimized it, restore it to previous state
    if (pTransferWindow->windowState().testFlag(Qt::WindowMinimized))
      pTransferWindow->setWindowState(pTransferWindow->windowState() &
                                      ~Qt::WindowMinimized);
    pTransferWindow->setWindowState(pTransferWindow->windowState() |
                                    Qt::WindowActive);
    pTransferWindow
        ->raise(); // make main window the top most window of the application
    pTransferWindow->show();
    pTransferWindow->activateWindow(); // bring window to foreground
  } else if (autoShow && !bringToForeground) {
    if (pTransferWindow->isHidden())
      pTransferWindow->setWindowState(pTransferWindow->windowState() |
                                      Qt::WindowMinimized);
    pTransferWindow->setWindowState(pTransferWindow->windowState() |
                                    Qt::WindowActive);
    pTransferWindow->show();
    qApp->alert(pTransferWindow);
  }

  pTransferWindow->setUserFilter(userId);
}

void lmcCore::initFileTransfer(MessageType type, FileMode mode,
                               QString *lpszUserId, XmlMessage *pMessage) {
  createTransferWindow();

  User *pUser = pMessaging->getUser(lpszUserId);
  if (!pUser)
      return;

  pTransferWindow->createTransfer(type, mode, lpszUserId, &pUser->name,
                                  pMessage);
}

void lmcCore::showUserInfo(XmlMessage *pMessage) {
  if (!pUserInfoWindow) {
    pUserInfoWindow = new lmcUserInfoWindow();
    pUserInfoWindow->init();
  }

  pUserInfoWindow->setInfo(pMessage);

  //	if window is minimized it, restore it to previous state
  if (pUserInfoWindow->windowState().testFlag(Qt::WindowMinimized))
    pUserInfoWindow->setWindowState(pUserInfoWindow->windowState() &
                                    ~Qt::WindowMinimized);
  pUserInfoWindow->setWindowState(pUserInfoWindow->windowState() |
                                  Qt::WindowActive);
  pUserInfoWindow
      ->raise(); // make window the top most window of the application
  pUserInfoWindow->show();
  pUserInfoWindow->activateWindow(); // bring window to foreground
}

void lmcCore::createChatRoomWindow(const QString &lpszThreadId) {
  //	create a new chat room with the specified thread id
  lmcChatRoomWindow *pChatRoomWindow = new lmcChatRoomWindow();
  chatRoomWindows.append(pChatRoomWindow);

  connect(pChatRoomWindow, &lmcChatRoomWindow::messageSent, this,
          &lmcCore::sendMessage);
  connect(pChatRoomWindow, &lmcChatRoomWindow::contactsAdding, this,
          &lmcCore::addContacts);
  connect(pChatRoomWindow, &lmcChatRoomWindow::chatStarting, this,
          &lmcCore::startChat);
  connect(pChatRoomWindow, &lmcChatRoomWindow::showHistory, this,
          &lmcCore::showHistory);
  connect(pChatRoomWindow, &lmcChatRoomWindow::showTransfers, this,
          &lmcCore::showTransfers);
  connect(pChatRoomWindow, &lmcChatRoomWindow::showTrayMessage,
          this, &lmcCore::showTrayMessage);
  connect(pChatRoomWindow, &lmcChatRoomWindow::closed, this,
          &lmcCore::chatRoomWindow_closed);

  User *pLocalUser = pMessaging->localUser;
  pChatRoomWindow->init(pLocalUser, pMessaging->isConnected(), lpszThreadId);
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
          QStringList excludeList(pMessaging->localUser->id);
          selectedContacts =
                  showSelectContacts(chatRoomWindow, UC_GroupMessage, &excludeList);
      }

      // if no contacts were selected, close the chat room window
      if (selectedContacts.count() == 0) {
          chatRoomWindows.removeOne(chatRoomWindow);
          chatRoomWindow->close();
          chatRoomWindow->deleteLater();
      } else {
          std::vector<User *> users;

          for (QString &userId : selectedContacts) {
              User *tempUser = pMessaging->getUser(&userId);
              if (tempUser)
                  users.push_back(tempUser);
          }
          chatRoomWindow->selectContacts(users);
      }
  }
}

void lmcCore::showPublicChatWindow(bool show, bool alert, bool open) {
  if (show) {
    if (pPublicChatWindow->windowState().testFlag(Qt::WindowMinimized))
      pPublicChatWindow->setWindowState(pPublicChatWindow->windowState() &
                                        ~Qt::WindowMinimized);
    pPublicChatWindow->show();
    pPublicChatWindow->activateWindow();
  } else {
    if (open) {
      if (pPublicChatWindow->isHidden())
        pPublicChatWindow->setWindowState(pPublicChatWindow->windowState() |
                                          Qt::WindowMinimized);
      pPublicChatWindow->show();
    }
    if (alert)
      qApp->alert(pPublicChatWindow);
  }
}

QStringList lmcCore::showSelectContacts(QWidget *parent, uint caps,
                                        QStringList *excludeList) {
  QStringList selectedContacts;
  pUserSelectDialog = new lmcUserSelectDialog(parent);

  QList<QTreeWidgetItem *> contactsList = pMainWindow->getContactsList();
  for (int index = 0; index < contactsList.count(); index++) {
    QTreeWidgetItem *pItem = contactsList.value(index);
    for (int childIndex = 0; childIndex < pItem->childCount(); childIndex++) {
      QTreeWidgetItem *pChildItem = pItem->child(childIndex);
      QString userId = pChildItem->data(0, IdRole).toString();
      User *pUser = pMessaging->getUser(&userId);
      if (!pUser)
          continue;
      if ((pUser->caps & caps) != caps) {
        pItem->removeChild(pChildItem);
        childIndex--;
        continue;
      }
      if (excludeList->contains(userId)) {
        pItem->removeChild(pChildItem);
        childIndex--;
        continue;
      }
    }
  }

  pUserSelectDialog->init(&contactsList);
  if (pUserSelectDialog->exec() != QDialog::Rejected)
    selectedContacts = pUserSelectDialog->selectedContacts;
  pUserSelectDialog->deleteLater();

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
