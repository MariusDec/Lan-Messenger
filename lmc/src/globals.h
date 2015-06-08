#ifndef GLOBALS_H
#define GLOBALS_H

#include "uidefinitions.h"
#include "settings.h"

#include <vector>
#include <QString>
#include <QCoreApplication>
#include <QApplication>
#include <QFont>
#include <QPalette>

enum StatusTypeEnum {
    StatusOnline = 0,
    StatusBusy,
    StatusAway,
    StatusOffline
};

struct StatusStruct {
    QString icon;
    QString description;
    QString uiDescription;
    StatusTypeEnum statusType;

    StatusStruct(const QString &icon, const QString &description, const QString &uiDescription, StatusTypeEnum statusType) : icon(icon), description(description), uiDescription(uiDescription), statusType(statusType) {}
    StatusStruct() {}
};

class Globals
{
    Q_DECLARE_TR_FUNCTIONS(Globals)

    std::vector<StatusStruct> _statuses = { StatusStruct("online", "Available", tr("Available"), StatusTypeEnum::StatusOnline), StatusStruct("busy", "Busy", tr("Busy"), StatusTypeEnum::StatusBusy), StatusStruct("nodisturb", "Do Not Disturb", tr("Do Not Disturb"), StatusTypeEnum::StatusBusy), StatusStruct("away", "Be Right Back", tr("Be Right Back"), StatusTypeEnum::StatusAway), StatusStruct("away", "Away", tr("Away"), StatusTypeEnum::StatusAway), StatusStruct("offline", "Appear Offline", tr("Appear Offline"), StatusTypeEnum::StatusOffline) };

    lmcSettings         _settings;

    QString             _version = "1.3.3.1";
    QString             _fileOpenPath;
    QString             _fileSavePath;

    QByteArray          _mainWindowGeometry;
    QByteArray          _transferWindowGeometry;
    QByteArray          _historyWindowGeometry;
    QByteArray          _broadcastWindowGeometry;
    QByteArray          _instantMessageWindowGeometry;
    QByteArray          _helpWindowGeometry;
    QByteArray          _publicChatWindowGeometry;
    QByteArray          _chatWindowGeometry;

    QByteArray          _historySplitterGeometry;
    QByteArray          _broadcastSplitterGeometry;
    QByteArray          _instantMessageSplitterGeometry;
    QByteArray          _publicChatHSplitterGeometry;
    QByteArray          _publicChatVSplitterGeometry;
    QByteArray          _chatHSplitterGeometry;
    QByteArray          _chatVSplitterGeometry;

    UserListView        _userListView = ULV_Detailed;
    bool                _sendByEnter = false;

    QString             _userStatus = "Available";
    int                 _userAvatarIndex = 65535;
    QString             _userName;
    QString             _userFirstName;
    QString             _userLastName;
    QString             _userAbout;
    QString             _userNote;

    bool                _autoStart = true;
    bool                _autoShow = true;
    bool                _windowSnapping = true;
    QString             _language = "en_US";

    int                 _refreshInterval = 30;
    int                 _idleTime = 0;

    bool                _sysTray = true;
    bool                _minimizeToTray = false;
    bool                _singleClickTray = false;
    bool                _sysTrayMessages = true;
    bool                _sysTrayNewPublicMessages = true;
    bool                _sysTrayNewMessages = true;
    bool                _sysTrayMinimize = false;
    int                 _defaultNewMessageAction = 1;

    bool                _restoreStatus = false;
    bool                _confirmLeaveChat = true;
    bool                _showEmoticons = true;
    bool                _showMessageTime = true;
    bool                _showMessageDate = false;
    bool                _showLinks = true;
    bool                _showPathsAsLinks = true;
    bool                _trimMessages = true;
    bool                _appendHistory = false;
    bool                _popOnNewMessage = true;
    bool                _popOnNewPublicMessage = false;
    QString             _messagesFontString = QApplication::font().toString();
    QFont               _messagesFont = QApplication::font();
    QString             _messagesColor = QApplication::palette().text().color().name();
    bool                _sendReadNotifs = true;
    bool                _showCharacterCount = true;

    QString             _chatTheme = "Classic";
    QString             _applicationTheme = "native";
    QString             _buttonsTheme = "native";
    QString             _iconTheme = "Default";

    bool                _overrideInMessagesStyle = false;
    bool                _saveHistory = true;
    bool                _defaultHistorySavePath = true;
    QString             _historySavePath;
    bool                _saveFileHistory = true;
    QString             _fileHistorySavePath;

    bool                _autoReceiveFiles = false;
    bool                _openNewTransfers = true;
    bool                _popupNewTransfers = true;
    QString             _fileStoragePath;
    bool                _createIndividualFolders = false;

    bool                _enableAlerts = true;
    bool                _noBusyAlerts = false;
    bool                _noDNDAlerts = true;
    bool                _enableSoundAlerts = false;
    bool                _noBusySounds = false;
    bool                _noDNDSounds = true;

    QString             _connectionName = "auto";
    int                 _connectionTimeout = 15;
    int                 _connectionRetries = 2;
    QString             _multicastAddress = "239.255.100.100";
    QString             _erpAddress = "http://192.168.1.201/erp/";
    int                 _udpPort = 50000;
    int                 _tcpPort = 50000;

    bool                _isConnected = true;

public:

    Globals();
    ~Globals();

    static Globals &getInstance() {
        static Globals globals;
        return globals;
    }

    std::vector<StatusStruct> &getStatuses() { return _statuses; }
    StatusStruct *getStatus(const QString &description, int *index = nullptr);
    StatusTypeEnum getStatusType(const QString &description);
    bool statusExists(const QString &description);

    void loadSettings();
    bool loadSettingsFromConfig(const QString &configFile);
    void syncSettings();

    const QString &version() const;
    void setVersion(const QString &version);
    const QString &fileOpenPath() const;
    void setFileOpenPath(const QString &fileOpenPath);
    const QString &fileSavePath() const;
    void setFileSavePath(const QString &fileSavePath);
    const QByteArray &mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &mainWindowGeometry);
    const QByteArray &transferWindowGeometry() const;
    void setTransferWindowGeometry(const QByteArray &transferWindowGeometry);
    const QByteArray &historyWindowGeometry() const;
    void setHistoryWindowGeometry(const QByteArray &historyWindowGeometry);
    const QByteArray &broadcastWindowGeometry() const;
    void setBroadcastWindowGeometry(const QByteArray &broadcastWindowGeometry);
    QByteArray instantMessageWindowGeometry() const;
    void setInstantMessageWindowGeometry(const QByteArray &instantMessageWindowGeometry);
    const QByteArray &helpWindowGeometry() const;
    void setHelpWindowGeometry(const QByteArray &helpWindowGeometry);
    const QByteArray &publicChatWindowGeometry() const;
    void setPublicChatWindowGeometry(const QByteArray &publicChatWindowGeometry);
    const QByteArray &chatWindowGeometry() const;
    void setChatWindowGeometry(const QByteArray &chatWindowGeometry);
    const QByteArray &historySplitterGeometry() const;
    void setHistorySplitterGeometry(const QByteArray &historySplitterGeometry);
    const QByteArray &broadcastSplitterGeometry() const;
    void setBroadcastSplitterGeometry(const QByteArray &broadcastSplitterGeometry);
    QByteArray instantMessageSplitterGeometry() const;
    void setInstantMessageSplitterGeometry(const QByteArray &instantMessageSplitterGeometry);
    const QByteArray &publicChatHSplitterGeometry() const;
    void setPublicChatHSplitterGeometry(const QByteArray &publicChatHSplitterGeometry);
    const QByteArray &publicChatVSplitterGeometry() const;
    void setPublicChatVSplitterGeometry(const QByteArray &publicChatVSplitterGeometry);
    const QByteArray &chatHSplitterGeometry() const;
    void setChatHSplitterGeometry(const QByteArray &chatHSplitterGeometry);
    const QByteArray &chatVSplitterGeometry() const;
    void setChatVSplitterGeometry(const QByteArray &chatVSplitterGeometry);
    UserListView userListView() const;
    void setUserListView(const UserListView &userListView);
    bool sendByEnter() const;
    void setSendByEnter(bool sendByEnter);
    const QString &userStatus() const;
    void setUserStatus(const QString &userStatus);
    int userAvatarIndex() const;
    void setUserAvatarIndex(int userAvatarIndex);
    const QString &userName() const;
    void setUserName(const QString &userName);
    const QString &userFirstName() const;
    void setUserFirstName(const QString &userFirstName);
    const QString &userLastName() const;
    void setUserLastName(const QString &userLastName);
    const QString &userAbout() const;
    void setUserAbout(const QString &userAbout);
    const QString &userNote() const;
    void setUserNote(const QString &userNote);
    bool autoStart() const;
    void setAutoStart(bool autoStart);
    bool autoShow() const;
    void setAutoShow(bool autoShow);
    bool windowSnapping() const;
    void setWindowSnapping(bool windowSnapping);
    const QString &language() const;
    void setLanguage(const QString &language);
    int refreshInterval() const;
    void setRefreshInterval(int refreshInterval);
    int idleTime() const;
    void setIdleTime(int idleTime);
    bool sysTray() const;
    void setSysTray(bool sysTray);
    bool minimizeToTray() const;
    void setMinimizeToTray(bool minimizeToTray);
    bool singleClickTray() const;
    void setSingleClickTray(bool singleClickTray);
    bool sysTrayMessages() const;
    void setSysTrayMessages(bool sysTrayMessages);
    bool sysTrayNewPublicMessages() const;
    void setSysTrayNewPublicMessages(bool sysTrayNewPublicMessages);
    bool sysTrayNewMessages() const;
    void setSysTrayNewMessages(bool sysTrayNewMessages);
    bool sysTrayMinimize() const;
    void setSysTrayMinimize(bool sysTrayMinimize);
    bool restoreStatus() const;
    void setRestoreStatus(bool restoreStatus);
    bool showEmoticons() const;
    void setShowEmoticons(bool showEmoticons);
    bool showMessageTime() const;
    void setShowMessageTime(bool showMessageTime);
    bool showMessageDate() const;
    void setShowMessageDate(bool showMessageDate);
    bool showLinks() const;
    void setShowLinks(bool showLinks);
    bool showPathsAsLinks() const;
    void setShowPathsAsLinks(bool showPathsAsLinks);
    bool trimMessages() const;
    void setTrimMessages(bool trimMessages);
    bool appendHistory() const;
    void setAppendHistory(bool appendHistory);
    bool popOnNewMessage() const;
    void setPopOnNewMessage(bool popOnNewMessage);
    bool popOnNewPublicMessage() const;
    void setPopOnNewPublicMessage(bool popOnNewPublicMessage);
    const QString &messagesFontString() const;
    const QFont &messagesFont() const;
    void setMessagesFont(const QString &messagesFontString);
    const QString &messagesColor() const;
    void setMessagesColor(const QString &messagesColor);
    const QString &chatTheme() const;
    void setChatTheme(const QString &chatTheme);
    const QString &applicationTheme() const;
    void setApplicationTheme(const QString &applicationTheme);
    const QString &buttonsTheme() const;
    void setButtonsTheme(const QString &buttonsTheme);
    const QString &iconTheme() const;
    void setIconTheme(const QString &iconTheme);
    bool overrideInMessagesStyle() const;
    void setOverrideInMessagesStyle(bool overrideInMessagesStyle);
    bool showCharacterCount() const;
    void setShowCharacterCount(bool showCharacterCount);
    bool saveHistory() const;
    void setSaveHistory(bool saveHistory);
    bool defaultHistorySavePath() const;
    void setDefaultHistorySavePath(bool defaultHistorySavePath);
    QString historySavePath();
    QString historyFile(const QDate &date);
    void setHistorySavePath(const QString &historySavePath);
    bool saveFileHistory() const;
    void setSaveFileHistory(bool saveFileHistory);
    QString fileHistorySavePath() const;
    void setFileHistorySavePath(const QString &fileHistorySavePath);
    bool autoReceiveFiles() const;
    void setAutoReceiveFile(bool autoReceiveFiles);
    bool openNewTransfers() const;
    void setOpenNewTransfers(bool openNewTransfers);
    bool popupNewTransfers() const;
    void setPopupNewTransfers(bool popupNewTransfers);
    QString fileStoragePath(const QString &sender = QString()) const;
    void setFileStoragePath(const QString &fileStoragePath);
    bool createIndividualFolders() const;
    void setCreateIndividualFolders(bool createIndividualFolders);
    bool enableAlerts() const;
    void setEnableAlerts(bool enableAlerts);
    bool noBusyAlerts() const;
    void setNoBusyAlerts(bool noBusyAlerts);
    bool noDNDAlerts() const;
    void setNoDNDAlerts(bool noDNDAlerts);
    bool enableSoundAlerts() const;
    void setEnableSoundAlerts(bool enableSoundAlerts);
    bool noBusySounds() const;
    void setNoBusySounds(bool noBusySounds);
    bool noDNDSounds() const;
    void setNoDNDSounds(bool noDNDSounds);
    const QString &connectionName() const;
    void setConnectionName(const QString &connectionName);
    int connectionTimeout() const;
    void setConnectionTimeout(int connectionTimeout);
    int connectionRetries() const;
    void setConnectionRetries(int connectionRetries);
    const QString &broadcastAddress() const;
    void setBroadcastAddress(const QString &broadcastAddress);
    const QString &multicastAddress() const;
    void setMulticastAddress(const QString &multicastAddress);
    const QString &erpAddress() const;
    void setErpAddress(const QString &erpAddress);
    int udpPort() const;
    void setUdpPort(int udpPort);
    int tcpPort() const;
    void setTcpPort(int tcpPort);
    bool isConnected() const;
    void setIsConnected(bool isConnected);
    int defaultNewMessageAction() const;
    void setDefaultNewMessageAction(int defaultNewMessageAction);
    bool confirmLeaveChat() const;
    void setConfirmLeaveChat(bool confirmLeaveChat);
    bool sendReadNotifs() const;
    void setSendReadNotifs(bool sendReadNotifs);
};

#endif // GLOBALS_H
