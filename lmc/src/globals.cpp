#include "globals.h"
#include "stdlocation.h"


const QString &Globals::version() const
{
    return _version;
}

void Globals::setVersion(const QString &version)
{
    _version = version;
    _settings.setValue(IDS_VERSION, version);
}

const QString &Globals::fileOpenPath() const
{
    return _fileOpenPath;
}

void Globals::setFileOpenPath(const QString &fileOpenPath)
{
    _fileOpenPath = fileOpenPath;
    _settings.setValue(IDS_OPENPATH, fileOpenPath);
}

const QString &Globals::fileSavePath() const
{
    return _fileSavePath;
}

void Globals::setFileSavePath(const QString &fileSavePath)
{
    _fileSavePath = fileSavePath;
    _settings.setValue(IDS_SAVEPATH, fileSavePath);
}

const QByteArray &Globals::mainWindowGeometry() const
{
    return _mainWindowGeometry;
}

void Globals::setMainWindowGeometry(const QByteArray &mainWindowGeometry)
{
    _mainWindowGeometry = mainWindowGeometry;
    _settings.setValue(IDS_WINDOWMAIN, mainWindowGeometry);
}

const QByteArray &Globals::transferWindowGeometry() const
{
    return _transferWindowGeometry;
}

void Globals::setTransferWindowGeometry(const QByteArray &transferWindowGeometry)
{
    _transferWindowGeometry = transferWindowGeometry;
    _settings.setValue(IDS_WINDOWTRANSFERS, transferWindowGeometry);
}

const QByteArray &Globals::historyWindowGeometry() const
{
    return _historyWindowGeometry;
}

void Globals::setHistoryWindowGeometry(const QByteArray &historyWindowGeometry)
{
    _historyWindowGeometry = historyWindowGeometry;
    _settings.setValue(IDS_WINDOWHISTORY, historyWindowGeometry);
}

const QByteArray &Globals::broadcastWindowGeometry() const
{
    return _broadcastWindowGeometry;
}

void Globals::setBroadcastWindowGeometry(const QByteArray &broadcastWindowGeometry)
{
    _broadcastWindowGeometry = broadcastWindowGeometry;
    _settings.setValue(IDS_WINDOWBROADCAST, broadcastWindowGeometry);
}

QByteArray Globals::instantMessageWindowGeometry() const
{
    return _instantMessageWindowGeometry;
}

void Globals::setInstantMessageWindowGeometry(const QByteArray &instantMessageWindowGeometry)
{
    _instantMessageWindowGeometry = instantMessageWindowGeometry;
    _settings.setValue(IDS_WINDOWINSTANTMSG, instantMessageWindowGeometry);
}

const QByteArray &Globals::helpWindowGeometry() const
{
    return _helpWindowGeometry;
}

void Globals::setHelpWindowGeometry(const QByteArray &helpWindowGeometry)
{
    _helpWindowGeometry = helpWindowGeometry;
    _settings.setValue(IDS_WINDOWHELP, helpWindowGeometry);
}

const QByteArray &Globals::publicChatWindowGeometry() const
{
    return _publicChatWindowGeometry;
}

void Globals::setPublicChatWindowGeometry(const QByteArray &publicChatWindowGeometry)
{
    _publicChatWindowGeometry = publicChatWindowGeometry;
    _settings.setValue(IDS_WINDOWPUBLICCHAT, publicChatWindowGeometry);
}

const QByteArray &Globals::chatWindowGeometry() const
{
    return _chatWindowGeometry;
}

void Globals::setChatWindowGeometry(const QByteArray &chatWindowGeometry)
{
    _chatWindowGeometry = chatWindowGeometry;
    _settings.setValue(IDS_WINDOWCHATROOM, chatWindowGeometry);
}

const QByteArray &Globals::historySplitterGeometry() const
{
    return _historySplitterGeometry;
}

void Globals::setHistorySplitterGeometry(const QByteArray &historySplitterGeometry)
{
    _historySplitterGeometry = historySplitterGeometry;
    _settings.setValue(IDS_SPLITTERHISTORY, historySplitterGeometry);
}

const QByteArray &Globals::broadcastSplitterGeometry() const
{
    return _broadcastSplitterGeometry;
}

void Globals::setBroadcastSplitterGeometry(const QByteArray &broadcastSplitterGeometry)
{
    _broadcastSplitterGeometry = broadcastSplitterGeometry;
    _settings.setValue(IDS_SPLITTERBROADCAST, broadcastSplitterGeometry);
}

QByteArray Globals::instantMessageSplitterGeometry() const
{
    return _instantMessageSplitterGeometry;
}

void Globals::setInstantMessageSplitterGeometry(const QByteArray &instantMessageSplitterGeometry)
{
    _instantMessageSplitterGeometry = instantMessageSplitterGeometry;
    _settings.setValue(IDS_SPLITTERINSTANTMSG, instantMessageSplitterGeometry);
}

const QByteArray &Globals::publicChatHSplitterGeometry() const
{
    return _publicChatHSplitterGeometry;
}

void Globals::setPublicChatHSplitterGeometry(const QByteArray &publicChatHSplitterGeometry)
{
    _publicChatHSplitterGeometry = publicChatHSplitterGeometry;
    _settings.setValue(IDS_SPLITTERPUBLICCHATH, publicChatHSplitterGeometry);
}

const QByteArray &Globals::publicChatVSplitterGeometry() const
{
    return _publicChatVSplitterGeometry;
}

void Globals::setPublicChatVSplitterGeometry(const QByteArray &publicChatVSplitterGeometry)
{
    _publicChatVSplitterGeometry = publicChatVSplitterGeometry;
    _settings.setValue(IDS_SPLITTERPUBLICCHATV, publicChatVSplitterGeometry);
}

const QByteArray &Globals::chatHSplitterGeometry() const
{
    return _chatHSplitterGeometry;
}

void Globals::setChatHSplitterGeometry(const QByteArray &chatHSplitterGeometry)
{
    _chatHSplitterGeometry = chatHSplitterGeometry;
    _settings.setValue(IDS_SPLITTERCHATROOMH, chatHSplitterGeometry);
}

const QByteArray &Globals::chatVSplitterGeometry() const
{
    return _chatVSplitterGeometry;
}

void Globals::setChatVSplitterGeometry(const QByteArray &chatVSplitterGeometry)
{
    _chatVSplitterGeometry = chatVSplitterGeometry;
    _settings.setValue(IDS_SPLITTERCHATROOMV, chatVSplitterGeometry);
}

UserListView Globals::userListView() const
{
    return _userListView;
}

void Globals::setUserListView(const UserListView &userListView)
{
    _userListView = userListView;
    _settings.setValue(IDS_USERLISTVIEW, userListView);
}

bool Globals::sendByEnter() const
{
    return _sendByEnter;
}

void Globals::setSendByEnter(bool sendByEnter)
{
    _sendByEnter = sendByEnter;
    _settings.setValue(IDS_SENDBYENTER, sendByEnter);
}

const QString &Globals::userStatus() const
{
    return _userStatus;
}

void Globals::setUserStatus(const QString &userStatus)
{
    _userStatus = userStatus;
    _settings.setValue(IDS_STATUS, userStatus);
}

int Globals::userAvatarIndex() const
{
    return _userAvatarIndex;
}

void Globals::setUserAvatarIndex(int userAvatarIndex)
{
    _userAvatarIndex = userAvatarIndex;
    _settings.setValue(IDS_AVATAR, userAvatarIndex);
}

const QString &Globals::userName() const
{
    return _userName;
}

void Globals::setUserName(const QString &userName)
{
    _userName = userName;
    _settings.setValue(IDS_USERNAME, userName);
}

const QString &Globals::userFirstName() const
{
    return _userFirstName;
}

void Globals::setUserFirstName(const QString &userFirstName)
{
    _userFirstName = userFirstName;
    _settings.setValue(IDS_USERFIRSTNAME, userFirstName);
}

const QString &Globals::userLastName() const
{
    return _userLastName;
}

void Globals::setUserLastName(const QString &userLastName)
{
    _userLastName = userLastName;
    _settings.setValue(IDS_USERLASTNAME, userLastName);
}

const QString &Globals::userAbout() const
{
    return _userAbout;
}

void Globals::setUserAbout(const QString &userAbout)
{
    _userAbout = userAbout;
    _settings.setValue(IDS_USERABOUT, userAbout);
}

const QString &Globals::userNote() const
{
    return _userNote;
}

void Globals::setUserNote(const QString &userNote)
{
    _userNote = userNote;
    _settings.setValue(IDS_NOTE, userNote);
}

bool Globals::autoStart() const
{
    return _autoStart;
}

void Globals::setAutoStart(bool autoStart)
{
    _autoStart = autoStart;
    _settings.setValue(IDS_AUTOSTART, autoStart);
}

bool Globals::autoShow() const
{
    return _autoShow;
}

void Globals::setAutoShow(bool autoShow)
{
    _autoShow = autoShow;
    _settings.setValue(IDS_AUTOSHOW, autoShow);
}

bool Globals::windowSnapping() const
{
    return _windowSnapping;
}

void Globals::setWindowSnapping(bool windowSnapping)
{
    _windowSnapping = windowSnapping;
    _settings.setValue(IDS_WINDOWSNAPPING, windowSnapping);
}

const QString &Globals::language() const
{
    return _language;
}

void Globals::setLanguage(const QString &language)
{
    _language = language;
    _settings.setValue(IDS_LANGUAGE, language);
}

int Globals::refreshInterval() const
{
    return _refreshInterval;
}

void Globals::setRefreshInterval(int refreshInterval)
{
    _refreshInterval = refreshInterval;
    _settings.setValue(IDS_REFRESHTIME, refreshInterval);
}

int Globals::idleTime() const
{
    return _idleTime;
}

void Globals::setIdleTime(int idleTime)
{
    _idleTime = idleTime;
    _settings.setValue(IDS_IDLETIME, idleTime);
}

bool Globals::sysTray() const
{
    return _sysTray;
}

void Globals::setSysTray(bool sysTray)
{
    _sysTray = sysTray;
    _settings.setValue(IDS_SYSTRAY, sysTray);
}

bool Globals::minimizeToTray() const
{
    return _minimizeToTray;
}

void Globals::setMinimizeToTray(bool minimizeToTray)
{
    _minimizeToTray = minimizeToTray;
    _settings.setValue(IDS_MINIMIZETRAY, minimizeToTray);
}

bool Globals::singleClickTray() const
{
    return _singleClickTray;
}

void Globals::setSingleClickTray(bool singleClickTray)
{
    _singleClickTray = singleClickTray;
    _settings.setValue(IDS_SINGLECLICKTRAY, singleClickTray);
}

bool Globals::sysTrayMessages() const
{
    return _sysTray && _sysTrayMessages;
}

void Globals::setSysTrayMessages(bool sysTrayMessages)
{
    _sysTrayMessages = sysTrayMessages;
    _settings.setValue(IDS_SYSTRAYMSG, sysTrayMessages);
}

bool Globals::sysTrayNewPublicMessages() const
{
    return (_sysTray && _sysTrayMessages && _sysTrayNewPublicMessages);
}

void Globals::setSysTrayNewPublicMessages(bool sysTrayNewPublicMessages)
{
    _sysTrayNewPublicMessages = sysTrayNewPublicMessages;
    _settings.setValue(IDS_SYSTRAYPUBNEWMSG, sysTrayNewPublicMessages);
}

bool Globals::sysTrayNewMessages() const
{
    return (_sysTray && _sysTrayMessages && _sysTrayNewMessages);
}

void Globals::setSysTrayNewMessages(bool sysTrayNewMessages)
{
    _sysTrayNewMessages = sysTrayNewMessages;
    _settings.setValue(IDS_SYSTRAYNEWMSG, sysTrayNewMessages);
}

bool Globals::sysTrayMinimize() const
{
    return _sysTrayMinimize;
}

void Globals::setSysTrayMinimize(bool sysTrayMinimize)
{
    _sysTrayMinimize = sysTrayMinimize;
    _settings.setValue(IDS_ALLOWSYSTRAYMIN, sysTrayMinimize);
}

bool Globals::restoreStatus() const
{
    return _restoreStatus;
}

void Globals::setRestoreStatus(bool restoreStatus)
{
    _restoreStatus = restoreStatus;
    _settings.setValue(IDS_RESTORESTATUS, restoreStatus);
}

bool Globals::confirmLeaveChat() const
{
    return _confirmLeaveChat;
}

void Globals::setConfirmLeaveChat(bool confirmLeaveChat)
{
    _confirmLeaveChat = confirmLeaveChat;
    _settings.setValue(IDS_CONFIRMLEAVECHAT, confirmLeaveChat);
}

bool Globals::showEmoticons() const
{
    return _showEmoticons;
}

void Globals::setShowEmoticons(bool showEmoticons)
{
    _showEmoticons = showEmoticons;
    _settings.setValue(IDS_EMOTICON, showEmoticons);
}

bool Globals::showMessageTime() const
{
    return _showMessageTime;
}

void Globals::setShowMessageTime(bool showMessageTime)
{
    _showMessageTime = showMessageTime;
    _settings.setValue(IDS_MESSAGETIME, showMessageTime);
}

bool Globals::showMessageDate() const
{
    return _showMessageDate;
}

void Globals::setShowMessageDate(bool showMessageDate)
{
    _showMessageDate = showMessageDate;
    _settings.setValue(IDS_MESSAGEDATE, showMessageDate);
}

bool Globals::showLinks() const
{
    return _showLinks;
}

void Globals::setShowLinks(bool showLinks)
{
    _showLinks = showLinks;
    _settings.setValue(IDS_ALLOWLINKS, showLinks);
}

bool Globals::showPathsAsLinks() const
{
    return _showLinks && _showPathsAsLinks;
}

void Globals::setShowPathsAsLinks(bool showPathsAsLinks)
{
    _showPathsAsLinks = showPathsAsLinks;
    _settings.setValue(IDS_PATHTOLINK, showPathsAsLinks);
}

bool Globals::trimMessages() const
{
    return _trimMessages;
}

void Globals::setTrimMessages(bool trimMessages)
{
    _trimMessages = trimMessages;
    _settings.setValue(IDS_TRIMMESSAGE, trimMessages);
}

bool Globals::appendHistory() const
{
    return _appendHistory;
}

void Globals::setAppendHistory(bool appendHistory)
{
    _appendHistory = appendHistory;
    _settings.setValue(IDS_APPENDHISTORY, appendHistory);
}

bool Globals::popOnNewMessage() const
{
    return _popOnNewMessage;
}

void Globals::setPopOnNewMessage(bool popOnNewMessage)
{
    _popOnNewMessage = popOnNewMessage;
    _settings.setValue(IDS_MESSAGEPOP, popOnNewMessage);
}

bool Globals::popOnNewPublicMessage() const
{
    return _popOnNewPublicMessage;
}

void Globals::setPopOnNewPublicMessage(bool popOnNewPublicMessage)
{
    _popOnNewPublicMessage = popOnNewPublicMessage;
    _settings.setValue(IDS_PUBMESSAGEPOP, popOnNewPublicMessage);
}

const QString &Globals::messagesFontString() const
{
    return _messagesFontString;
}

const QFont &Globals::messagesFont() const
{
    return _messagesFont;
}

void Globals::setMessagesFont(const QString &messagesFontString)
{
    _messagesFontString = messagesFontString;
    _messagesFont.fromString(messagesFontString);
    _settings.setValue(IDS_FONT, messagesFontString);
}

const QString &Globals::messagesColor() const
{
    return _messagesColor;
}

void Globals::setMessagesColor(const QString &messagesColor)
{
    _messagesColor = messagesColor;
    _settings.setValue(IDS_COLOR, messagesColor);
}

bool Globals::informReadMessage() const
{
    return _informReadMessage;
}

void Globals::setInformReadMessage(bool informReadMessage)
{
    _informReadMessage = informReadMessage;
    _settings.setValue(IDS_INFORMREAD, informReadMessage);
}

const QString &Globals::chatTheme() const
{
    return _chatTheme;
}

void Globals::setChatTheme(const QString &chatTheme)
{
    _chatTheme = chatTheme;
    _settings.setValue(IDS_CHATTHEME, chatTheme);
}

const QString &Globals::applicationTheme() const
{
    return _applicationTheme;
}

void Globals::setApplicationTheme(const QString &applicationTheme)
{
    _applicationTheme = applicationTheme;
    _settings.setValue(IDS_APPTHEME, applicationTheme);
}

const QString &Globals::buttonsTheme() const
{
    return _buttonsTheme;
}

void Globals::setButtonsTheme(const QString &buttonsTheme)
{
    _buttonsTheme = buttonsTheme;
    _settings.setValue(IDS_BUTTONTHEME, buttonsTheme);
}

const QString &Globals::iconTheme() const
{
    return _iconTheme;
}

void Globals::setIconTheme(const QString &iconTheme)
{
    _iconTheme = iconTheme;
    _settings.setValue(IDS_APPICONTHEME, iconTheme);
}

bool Globals::overrideInMessagesStyle() const
{
    return _overrideInMessagesStyle;
}

void Globals::setOverrideInMessagesStyle(bool overrideInMessagesStyle)
{
    _overrideInMessagesStyle = overrideInMessagesStyle;
    _settings.setValue(IDS_OVERRIDEINMSG, overrideInMessagesStyle);
}

bool Globals::showCharacterCount() const
{
    return _showCharacterCount;
}

void Globals::setShowCharacterCount(bool showCharacterCount)
{
    _showCharacterCount = showCharacterCount;
    _settings.setValue(IDS_SHOWCHARCOUNT, showCharacterCount);
}

bool Globals::saveHistory() const
{
    return _saveHistory;
}

void Globals::setSaveHistory(bool saveHistory)
{
    _saveHistory = saveHistory;
    _settings.setValue(IDS_HISTORY, saveHistory);
}

bool Globals::defaultHistorySavePath() const
{
    return _defaultHistorySavePath;
}

void Globals::setDefaultHistorySavePath(bool defaultHistorySavePath)
{
    _defaultHistorySavePath = defaultHistorySavePath;
    _settings.setValue(IDS_SYSHISTORYPATH, defaultHistorySavePath);
}

QString Globals::historySavePath()
{
    if (_defaultHistorySavePath || _historySavePath.isEmpty())
        return StdLocation::defaultHistorySavePath();
    return _historySavePath;
}

QString Globals::historyFile(const QDate &date)
{
    QString file = historySavePath();
    file.append(QString("conversation %1.xml").arg(date.toString(Qt::ISODate)));
    return file;
}

void Globals::setHistorySavePath(const QString &historySavePath)
{
    _historySavePath = historySavePath;

    if (!_historySavePath.isEmpty() && !_historySavePath.endsWith('/'))
        _historySavePath.append('/');

    _settings.setValue(IDS_HISTORYPATH, _historySavePath);
}

bool Globals::saveFileHistory() const
{
    return _saveFileHistory;
}

void Globals::setSaveFileHistory(bool saveFileHistory)
{
    _saveFileHistory = saveFileHistory;
    _settings.setValue(IDS_FILEHISTORY, saveFileHistory);
}

QString Globals::fileHistorySavePath() const
{
    if (_fileHistorySavePath.isEmpty())
        return StdLocation::defaultTransferHistorySavePath();

    return _fileHistorySavePath;
}

void Globals::setFileHistorySavePath(const QString &fileHistorySavePath)
{
    _fileHistorySavePath = fileHistorySavePath;

    if (!_fileHistorySavePath.isEmpty() && !_fileHistorySavePath.endsWith('/'))
        _fileHistorySavePath.append('/');

    _settings.setValue(IDS_FILEHISTORYPATH, _fileHistorySavePath);
}

bool Globals::autoReceiveFile() const
{
    return _autoReceiveFile;
}

void Globals::setAutoReceiveFile(bool autoReceiveFile)
{
    _autoReceiveFile = autoReceiveFile;
    _settings.setValue(IDS_AUTOFILE, autoReceiveFile);
}

bool Globals::autoShowTransfer() const
{
    return _autoShowTransfer;
}

void Globals::setAutoShowTransfer(bool autoShowTransfer)
{
    _autoShowTransfer = autoShowTransfer;
    _settings.setValue(IDS_AUTOSHOWFILE, autoShowTransfer);
}

bool Globals::displayNewTransfers() const
{
    return _autoShowTransfer && _displayNewTransfers;
}

void Globals::setDisplayNewTransfers(bool displayNewTransfers)
{
    _displayNewTransfers = displayNewTransfers;
    _settings.setValue(IDS_FILETOP, displayNewTransfers);
}

QString Globals::fileStoragePath(const QString &sender) const
{
    QString location = _fileStoragePath;

    if (location.isEmpty())
        location = StdLocation::defaultFileStoragePath(sender);

    if (_createIndividualFolders && !sender.isEmpty())
        location.append(QString("%1/").arg(sender));

    return location;
}

void Globals::setFileStoragePath(const QString &fileStoragePath)
{
    _fileStoragePath = fileStoragePath;

    if (!_fileStoragePath.isEmpty() && !_fileStoragePath.endsWith('/'))
        _fileStoragePath.append('/');

    _settings.setValue(IDS_FILESTORAGEPATH, _fileStoragePath);
}

bool Globals::createIndividualFolders() const
{
    return _createIndividualFolders;
}

void Globals::setCreateIndividualFolders(bool createIndividualFolders)
{
    _createIndividualFolders = createIndividualFolders;
    _settings.setValue(IDS_STORAGEUSERFOLDER, createIndividualFolders);
}

bool Globals::enableAlerts() const
{
    return _enableAlerts;
}

void Globals::setEnableAlerts(bool enableAlerts)
{
    _enableAlerts = enableAlerts;
    _settings.setValue(IDS_ALERT, enableAlerts);
}

bool Globals::noBusyAlerts() const
{
    return !_enableAlerts || _noBusyAlerts;
}

void Globals::setNoBusyAlerts(bool noBusyAlerts)
{
    _noBusyAlerts = noBusyAlerts;
    _settings.setValue(IDS_NOBUSYALERT, noBusyAlerts);
}

bool Globals::noDNDAlerts() const
{
    return !_enableAlerts || _noDNDAlerts;
}

void Globals::setNoDNDAlerts(bool noDNDAlerts)
{
    _noDNDAlerts = noDNDAlerts;
    _settings.setValue(IDS_NODNDALERT, noDNDAlerts);
}

bool Globals::enableSoundAlerts() const
{
    return _enableAlerts && _enableSoundAlerts;
}

void Globals::setEnableSoundAlerts(bool soundAlerts)
{
    _enableSoundAlerts = soundAlerts;
    _settings.setValue(IDS_SOUND, soundAlerts);
}

bool Globals::noBusySounds() const
{
    return !enableSoundAlerts() || _noBusySounds;
}

void Globals::setNoBusySounds(bool noBusySounds)
{
    _noBusySounds = noBusySounds;
    _settings.setValue(IDS_NOBUSYSOUND, noBusySounds);
}

bool Globals::noDNDSounds() const
{
    return !enableSoundAlerts() || _noDNDSounds;
}

void Globals::setNoDNDSounds(bool noDNDSounds)
{
    _noDNDSounds = noDNDSounds;
    _settings.setValue(IDS_NODNDSOUND, noDNDSounds);
}

const QString &Globals::connectionName() const
{
    return _connectionName;
}

void Globals::setConnectionName(const QString &connectionName)
{
    _connectionName = connectionName;
    _settings.setValue(IDS_CONNECTION, connectionName);
}

int Globals::connectionTimeout() const
{
    return _connectionTimeout;
}

void Globals::setConnectionTimeout(int connectionTimeout)
{
    _connectionTimeout = connectionTimeout;
    _settings.setValue(IDS_TIMEOUT, connectionTimeout);
}

int Globals::connectionRetries() const
{
    return _connectionRetries;
}

void Globals::setConnectionRetries(int connectionRetries)
{
    _connectionRetries = connectionRetries;
    _settings.setValue(IDS_MAXRETRIES, connectionRetries);
}

const QString &Globals::multicastAddress() const
{
    return _multicastAddress;
}

void Globals::setMulticastAddress(const QString &multicastAddress)
{
    _multicastAddress = multicastAddress;
    _settings.setValue(IDS_MULTICAST, multicastAddress);
}

const QString &Globals::erpAddress() const
{
    return _erpAddress;
}

void Globals::setErpAddress(const QString &erpAddress)
{
    _erpAddress = erpAddress;
    _settings.setValue(IDS_ERPADDRESS, erpAddress);
}

int Globals::udpPort() const
{
    return _udpPort;
}

void Globals::setUdpPort(int udpPort)
{
    _udpPort = udpPort;
    _settings.setValue(IDS_UDPPORT, udpPort);
}

int Globals::tcpPort() const
{
    return _tcpPort;
}

void Globals::setTcpPort(int tcpPort)
{
    _tcpPort = tcpPort;
    _settings.setValue(IDS_TCPPORT, tcpPort);
}


bool Globals::isConnected() const
{
    return _isConnected;
}

void Globals::setIsConnected(bool isConnected)
{
    _isConnected = isConnected;
}


int Globals::defaultNewMessageAction() const
{
    return _defaultNewMessageAction;
}

void Globals::setDefaultNewMessageAction(int defaultNewMessageAction)
{
    _defaultNewMessageAction = defaultNewMessageAction;
    _settings.setValue(IDS_DEFMSGACTION, defaultNewMessageAction);
}

Globals::Globals()
{

}

Globals::~Globals()
{

}

StatusStruct *Globals::getStatus(const QString &description, int *index) // TODO use std::optional
{
    StatusStruct *currentStatus = nullptr;
    int tempIndex = 0;

    for (StatusStruct &statusItem : _statuses) {
        if (!statusItem.description.compare (description)) {
            currentStatus = &statusItem;
            if (index)
                *index = tempIndex;
            break;
        }
        ++tempIndex;
    }
    if (!currentStatus && index)
        *index = -1;

    return currentStatus;
}

StatusTypeEnum Globals::getStatusType(const QString &description)
{
    StatusStruct *currrentStatus = getStatus (description);
    if (!currrentStatus)
        return StatusTypeEnum::StatusOnline;

    return currrentStatus->statusType;
}

bool Globals::statusExists(const QString &description)
{
    for (const StatusStruct &statusItem : _statuses)
        if (statusItem.description.compare (description))
            return true;

    return false;
}

void Globals::loadSettings()
{
    _version = _settings.value(IDS_VERSION, IDS_VERSION_VAL).toString();
    _fileOpenPath = _settings.value(IDS_OPENPATH, StdLocation::getDocumentsPath()).toString();
    _fileSavePath = _settings.value(IDS_SAVEPATH, StdLocation::getDocumentsPath()).toString();

    _mainWindowGeometry = _settings.value(IDS_WINDOWMAIN).toByteArray();
    _transferWindowGeometry = _settings.value(IDS_WINDOWTRANSFERS).toByteArray();
    _historyWindowGeometry = _settings.value(IDS_WINDOWHISTORY).toByteArray();
    _broadcastWindowGeometry = _settings.value(IDS_WINDOWBROADCAST).toByteArray();
    _instantMessageWindowGeometry = _settings.value(IDS_WINDOWINSTANTMSG).toByteArray();
    _helpWindowGeometry = _settings.value(IDS_WINDOWHELP).toByteArray();
    _publicChatWindowGeometry = _settings.value(IDS_WINDOWPUBLICCHAT).toByteArray();
    _chatWindowGeometry = _settings.value(IDS_WINDOWCHATROOM).toByteArray();

    _historySplitterGeometry = _settings.value(IDS_SPLITTERHISTORY).toByteArray();
    _broadcastSplitterGeometry = _settings.value(IDS_SPLITTERBROADCAST).toByteArray();
    _instantMessageSplitterGeometry = _settings.value(IDS_SPLITTERINSTANTMSG).toByteArray();
    _publicChatHSplitterGeometry = _settings.value(IDS_SPLITTERPUBLICCHATH).toByteArray();
    _publicChatVSplitterGeometry = _settings.value(IDS_SPLITTERPUBLICCHATV).toByteArray();
    _chatHSplitterGeometry = _settings.value(IDS_SPLITTERCHATROOMH).toByteArray();
    _chatVSplitterGeometry = _settings.value(IDS_SPLITTERCHATROOMV).toByteArray();

    _userListView = static_cast<UserListView> (_settings.value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt());
    _sendByEnter = _settings.value(IDS_SENDBYENTER, IDS_SENDBYENTER_VAL).toBool();

    _userStatus = _settings.value(IDS_STATUS, IDS_STATUS_VAL).toString();
    _userAvatarIndex = _settings.value(IDS_AVATAR, IDS_AVATAR_VAL).toInt();
    _userName = _settings.value(IDS_USERNAME, IDS_USERNAME_VAL).toString();
    _userFirstName = _settings.value(IDS_USERFIRSTNAME, IDS_USERFIRSTNAME_VAL).toString();
    _userLastName = _settings.value(IDS_USERLASTNAME, IDS_USERLASTNAME_VAL).toString();
    _userAbout = _settings.value(IDS_USERABOUT, IDS_USERABOUT_VAL).toString();
    _userNote = _settings.value(IDS_NOTE, IDS_NOTE_VAL).toString();

    _autoStart = _settings.value(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL).toBool();
    _autoShow = _settings.value(IDS_AUTOSHOW, IDS_AUTOSHOW_DEFAULT_VAL).toBool();
    _windowSnapping = _settings.value(IDS_WINDOWSNAPPING, IDS_WINDOWSNAPPING_VAL).toBool();
    _language = _settings.value(IDS_LANGUAGE, IDS_LANGUAGE_DEFAULT_VAL).toString();

    _refreshInterval = _settings.value(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL).toInt();
    _idleTime = _settings.value(IDS_IDLETIME, IDS_IDLETIME_VAL).toInt();

    _sysTray = _settings.value(IDS_SYSTRAY, IDS_SYSTRAY_VAL).toBool();
    _minimizeToTray = _settings.value(IDS_MINIMIZETRAY, IDS_MINIMIZETRAY_VAL).toBool();
    _singleClickTray = _settings.value(IDS_SINGLECLICKTRAY, IDS_SINGLECLICKTRAY_VAL).toBool();
    _sysTrayMessages = _settings.value(IDS_SYSTRAYMSG, IDS_SYSTRAYMSG_VAL).toBool();
    _sysTrayNewPublicMessages = _settings.value(IDS_SYSTRAYPUBNEWMSG, IDS_SYSTRAYPUBNEWMSG_VAL).toBool();
    _sysTrayNewMessages = _settings.value(IDS_SYSTRAYNEWMSG, IDS_SYSTRAYNEWMSG_VAL).toBool();
    _sysTrayMinimize = _settings.value(IDS_ALLOWSYSTRAYMIN, IDS_ALLOWSYSTRAYMIN_VAL).toBool();
    _defaultNewMessageAction = _settings.value(IDS_DEFMSGACTION, IDS_DEFMSGACTION_VAL).toInt();

    _restoreStatus = _settings.value(IDS_RESTORESTATUS, IDS_RESTORESTATUS_VAL).toBool();
    _confirmLeaveChat = _settings.value(IDS_CONFIRMLEAVECHAT, IDS_CONFIRMLEAVECHAT_VAL).toBool();
    _showEmoticons = _settings.value(IDS_EMOTICON, IDS_EMOTICON_VAL).toBool();
    _showMessageTime = _settings.value(IDS_MESSAGETIME, IDS_MESSAGETIME_VAL).toBool();
    _showMessageDate = _settings.value(IDS_MESSAGEDATE, IDS_MESSAGEDATE_VAL).toBool();
    _showLinks = _settings.value(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL).toBool();
    _showPathsAsLinks = _settings.value(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL).toBool();
    _trimMessages = _settings.value(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL).toBool();
    _appendHistory = _settings.value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool();
    _popOnNewMessage = _settings.value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool();
    _popOnNewPublicMessage = _settings.value(IDS_PUBMESSAGEPOP, IDS_PUBMESSAGEPOP_VAL).toBool();
    _messagesFontString = _settings.value(IDS_FONT, IDS_FONT_VAL).toString();
    _showCharacterCount = _settings.value(IDS_SHOWCHARCOUNT, IDS_SHOWCHARCOUNT_VAL).toString();
    _messagesFont.fromString(_messagesFontString);
    _messagesColor = _settings.value(IDS_COLOR, IDS_COLOR_VAL).toString();
    _informReadMessage = _settings.value(IDS_INFORMREAD, IDS_INFORMREAD_VAL).toBool();

    _chatTheme = _settings.value(IDS_CHATTHEME, IDS_CHATTHEME_VAL).toString();
    _applicationTheme = _settings.value(IDS_APPTHEME, IDS_APPTHEME_VAL).toString();
    _buttonsTheme = _settings.value(IDS_BUTTONTHEME, IDS_BUTTONTHEME_VAL).toString();
    _iconTheme = _settings.value(IDS_APPICONTHEME, IDS_APPICONTHEME_VAL).toString();

    _overrideInMessagesStyle = _settings.value(IDS_OVERRIDEINMSG, IDS_OVERRIDEINMSG_VAL).toBool();
    _saveHistory = _settings.value(IDS_HISTORY, IDS_HISTORY_VAL).toBool();
    _defaultHistorySavePath = _settings.value(IDS_SYSHISTORYPATH, IDS_SYSHISTORYPATH_VAL).toBool();
    _historySavePath = _settings.value(IDS_HISTORYPATH, IDS_HISTORYPATH_VAL).toString();
    if (!_historySavePath.isEmpty() && !_historySavePath.endsWith('/')) _historySavePath.append('/');
    _saveFileHistory = _settings.value(IDS_FILEHISTORY, IDS_FILEHISTORY_VAL).toBool();
    _fileHistorySavePath = _settings.value(IDS_FILEHISTORYPATH, IDS_FILEHISTORYPATH_VAL).toString();
    if (!_fileHistorySavePath.isEmpty() && !_fileHistorySavePath.endsWith('/')) _fileHistorySavePath.append('/');

    _autoReceiveFile = _settings.value(IDS_AUTOFILE, IDS_AUTOFILE_VAL).toBool();
    _autoShowTransfer = _settings.value(IDS_AUTOSHOWFILE, IDS_AUTOSHOWFILE_VAL).toBool();
    _displayNewTransfers = _settings.value(IDS_FILETOP, IDS_FILETOP_VAL).toBool();
    _fileStoragePath = _settings.value(IDS_FILESTORAGEPATH, IDS_FILESTORAGEPATH).toString();
    if (!_fileStoragePath.isEmpty() && !_fileStoragePath.endsWith('/')) _fileStoragePath.append('/');
    _createIndividualFolders = _settings.value(IDS_STORAGEUSERFOLDER, IDS_STORAGEUSERFOLDER_VAL).toBool();

    _enableAlerts = _settings.value(IDS_ALERT, IDS_ALERT_VAL).toBool();
    _noBusyAlerts = _settings.value(IDS_NOBUSYALERT, IDS_NOBUSYALERT_VAL).toBool();
    _noDNDAlerts = _settings.value(IDS_NODNDALERT, IDS_NODNDALERT_VAL).toBool();
    _enableSoundAlerts = _settings.value(IDS_SOUND, IDS_SOUND_VAL).toBool();
    _noBusySounds = _settings.value(IDS_NOBUSYSOUND, IDS_NOBUSYSOUND_VAL).toBool();
    _noDNDSounds = _settings.value(IDS_NODNDSOUND, IDS_NODNDSOUND_VAL).toBool();

    _connectionName = _settings.value(IDS_CONNECTION, IDS_CONNECTION_VAL).toString();
    _connectionTimeout = _settings.value(IDS_TIMEOUT, IDS_TIMEOUT_VAL).toInt();
    _connectionRetries = _settings.value(IDS_MAXRETRIES, IDS_MAXRETRIES_VAL).toInt();
    _multicastAddress = _settings.value(IDS_MULTICAST, IDS_MULTICAST_VAL).toString();
    _erpAddress = _settings.value(IDS_ERPADDRESS, IDS_ERPADDRESS_VAL).toString();
    _udpPort = _settings.value(IDS_UDPPORT, IDS_UDPPORT_VAL).toInt();
    _tcpPort = _settings.value(IDS_TCPPORT, IDS_TCPPORT_VAL).toInt();
}

bool Globals::loadSettingsFromConfig(const QString &configFile)
{
    bool result = _settings.loadFromConfig(configFile);
    loadSettings();
    return result;
}

void Globals::syncSettings()
{
    _settings.sync();
}

