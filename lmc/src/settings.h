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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QtWidgets/QApplication>
#include <QFont>
#include <QPalette>
#include <QDir>
#include "shared.h"
#include "uidefinitions.h"

//	Application settings definitions and default values
#define IDS_VERSION				"Application/Version"
#define IDS_VERSION_VAL			"1.3.3"
#define IDS_OPENPATH			"Application/OpenPath"
#define IDS_SAVEPATH			"Application/SavePath"
#define IDS_WINDOWMAIN			"Window/Main"
#define IDS_WINDOWTRANSFERS		"Window/Transfers"
#define IDS_WINDOWHISTORY		"Window/History"
#define IDS_WINDOWBROADCAST		"Window/Broadcast"
#define IDS_WINDOWINSTANTMSG	"Window/InstantMessage"
#define IDS_WINDOWHELP			"Window/Help"
#define IDS_WINDOWPUBLICCHAT	"Window/PublicChat"
#define IDS_WINDOWCHATROOM  	"Window/ChatRoom"
#define IDS_SPLITTERHISTORY		"Splitter/History"
#define IDS_SPLITTERBROADCAST	"Splitter/Broadcast"
#define IDS_SPLITTERINSTANTMSG	"Splitter/InstantMessage"
#define IDS_SPLITTERPUBLICCHATH	"Splitter/PublicChatH"
#define IDS_SPLITTERPUBLICCHATV	"Splitter/PublicChatV"
#define IDS_SPLITTERCHATROOMH	"Splitter/ChatRoomH"
#define IDS_SPLITTERCHATROOMV	"Splitter/ChatRoomV"
#define IDS_AUTOSTART			"AutoStart"
#define IDS_AUTOSTART_DEFAULT_VAL true
#define IDS_AUTOSHOW			"AutoShow"
#define IDS_AUTOSHOW_DEFAULT_VAL true
#define IDS_WINDOWSNAPPING		"WindowSnapping"
#define IDS_WINDOWSNAPPING_VAL  true
#define IDS_LANGUAGE			"Locale/Language"
#define IDS_LANGUAGE_DEFAULT_VAL "en_US"
#define IDS_SYSTRAY				"SystemTray/SysTray"
#define IDS_SYSTRAY_VAL			true
#define IDS_MINIMIZETRAY		"SystemTray/MinimizeTray"
#define IDS_MINIMIZETRAY_VAL	false
#define IDS_SINGLECLICKTRAY		"SystemTray/SingleClickTray"
#define IDS_SINGLECLICKTRAY_VAL	false
#define IDS_SYSTRAYMSG			"SystemTray/SysTrayMsg"
#define IDS_SYSTRAYMSG_VAL		true
#define IDS_SYSTRAYPUBNEWMSG	"SystemTray/SysTrayPubNewMsg"
#define IDS_SYSTRAYPUBNEWMSG_VAL true
#define IDS_SYSTRAYNEWMSG		"SystemTray/SysTrayNewMsg"
#define IDS_SYSTRAYNEWMSG_VAL	true
#define IDS_ALLOWSYSTRAYMIN		"SystemTray/AllowMinimize"
#define IDS_ALLOWSYSTRAYMIN_VAL	false
#define IDS_DEFMSGACTION		"SystemTray/DefaultNewMessageAction"
#define IDS_DEFMSGACTION_VAL	1
#define IDS_REFRESHTIME			"RefreshInterval"
#define IDS_REFRESHTIME_VAL		30
#define IDS_RESTORESTATUS		"RestoreStatus"
#define IDS_RESTORESTATUS_VAL	false
#define IDS_IDLETIME			"IdleTime"
#define IDS_IDLETIME_VAL		0
#define IDS_EMOTICON			"Messages/Emoticon"
#define IDS_EMOTICON_VAL		true
#define IDS_CONFIRMLEAVECHAT    "Messages/ConfirmLeaveChat"
#define IDS_CONFIRMLEAVECHAT_VAL true
#define IDS_MESSAGETIME			"Messages/MessageTime"
#define IDS_MESSAGETIME_VAL		true
#define IDS_MESSAGEDATE			"Messages/MessageDate"
#define IDS_MESSAGEDATE_VAL		false
#define IDS_ALLOWLINKS			"Messages/AllowLinks"
#define IDS_ALLOWLINKS_VAL		true
#define IDS_PATHTOLINK			"Messages/PathToLink"
#define IDS_PATHTOLINK_VAL		true
#define IDS_TRIMMESSAGE			"Messages/Trim"
#define IDS_TRIMMESSAGE_VAL		true
#define IDS_APPENDHISTORY       "Messages/AppendHistory"
#define IDS_APPENDHISTORY_VAL    false
#define IDS_MESSAGEPOP			"Messages/MessagePop"
#define IDS_MESSAGEPOP_VAL		true
#define IDS_PUBMESSAGEPOP		"Messages/PubMessagePop"
#define IDS_PUBMESSAGEPOP_VAL	false
#define IDS_INFORMREAD  		"Messages/InformReadMessage"
#define IDS_INFORMREAD_VAL	    true
#define IDS_FONT				"Messages/Font"
#define IDS_FONT_VAL			QApplication::font().toString()
#define IDS_COLOR				"Messages/Color"
#define IDS_COLOR_VAL			QApplication::palette().text().color().name()
#define IDS_OVERRIDEINMSG		"Messages/OverrideIncomming"
#define IDS_OVERRIDEINMSG_VAL	false
#define IDS_SHOWCHARCOUNT       "Messages/ShowCharacterCount"
#define IDS_SHOWCHARCOUNT_VAL   true
#define IDS_HISTORY				"History/History"
#define IDS_HISTORY_VAL			true
#define IDS_SYSHISTORYPATH		"History/SysHistoryPathDef"
#define IDS_SYSHISTORYPATH_VAL	true
#define IDS_HISTORYPATH			"History/HistoryPath"
#define IDS_HISTORYPATH_VAL		""
#define IDS_FILEHISTORY			"History/FileHistory"
#define IDS_FILEHISTORY_VAL		true
#define IDS_FILEHISTORYPATH		"History/FileHistoryPath"
#define IDS_FILEHISTORYPATH_VAL	""
#define IDS_ALERT				"Alerts/Alert"
#define IDS_ALERT_VAL			true
#define IDS_NOBUSYALERT			"Alerts/NoBusyAlert"
#define IDS_NOBUSYALERT_VAL		false
#define IDS_NODNDALERT			"Alerts/NoDNDAlert"
#define IDS_NODNDALERT_VAL		true
#define IDS_SOUND				"Alerts/Sound"
#define IDS_SOUND_VAL			false
#define IDS_NOBUSYSOUND			"Alerts/NoBusySound"
#define IDS_NOBUSYSOUND_VAL		false
#define IDS_NODNDSOUND			"Alerts/NoDNDSound"
#define IDS_NODNDSOUND_VAL		true

// THESE
#define IDS_SOUNDEVENTHDR		"SoundEvents"
#define IDS_SOUNDEVENT			"Event"
#define IDS_SOUNDEVENT_VAL		Qt::Checked
#define IDS_SOUNDFILEHDR		"SoundFiles"
#define IDS_SOUNDFILE			"File"

#define IDS_CONNECTION			"Connection/Connection" // TODO !!! Not loaded
#define IDS_CONNECTION_VAL		AUTO_CONNECTION
#define IDS_TIMEOUT				"Connection/Timeout"
#define IDS_TIMEOUT_VAL			15
#define IDS_MAXRETRIES			"Connection/MaxRetries"
#define IDS_MAXRETRIES_VAL		2
#define IDS_MULTICAST			"Connection/Multicast"
#define IDS_MULTICAST_VAL		"239.255.100.100"
#define IDS_UDPPORT				"Connection/UDPPort"
#define IDS_UDPPORT_VAL			50000
#define IDS_TCPPORT				"Connection/TCPPort"
#define IDS_TCPPORT_VAL			50000
#define IDS_ERPADDRESS			"Connection/ERPAddress"
#define IDS_ERPADDRESS_VAL		"http://192.168.1.201/erp/"
#define IDS_AUTOFILE			"FileTransfer/AutoFile"
#define IDS_AUTOFILE_VAL		false
#define	IDS_AUTOSHOWFILE		"FileTransfer/AutoShow"
#define	IDS_AUTOSHOWFILE_VAL	true
#define IDS_FILETOP				"FileTransfer/FileTop"
#define IDS_FILETOP_VAL			true
#define IDS_FILESTORAGEPATH		"FileTransfer/StoragePath" // TODO !!! Not loaded
#define IDS_FILESTORAGEPATH_VAL	""
#define IDS_STORAGEUSERFOLDER	"FileTransfer/UserFolder" // TODO !!! Not loaded
#define IDS_STORAGEUSERFOLDER_VAL	false
#define IDS_CHATTHEME			"Appearance/ChatTheme"
#define IDS_CHATTHEME_VAL		"Classic"
#define IDS_APPTHEME		    "Appearance/ApplicationTheme"
#define IDS_APPTHEME_VAL		"native"
#define IDS_APPICONTHEME		"Appearance/IconTheme" // TODO !!! Not loaded
#define IDS_APPICONTHEME_VAL	"Default"
#define IDS_BUTTONTHEME		    "Appearance/ButtonTheme"
#define IDS_BUTTONTHEME_VAL		"native"
#define IDS_USERLISTVIEW		"Appearance/UserListView"
#define IDS_USERLISTVIEW_VAL	ULV_Detailed
#define IDS_SENDBYENTER			"Hotkeys/SendByEnter"
#define IDS_SENDBYENTER_VAL		true
#define IDS_STATUS				"User/Status" // TODO !!! Not loaded
#define IDS_STATUS_VAL			"Available"
#define IDS_AVATAR				"User/Avatar" // TODO !!! Not loaded
#define IDS_AVATAR_VAL			65535	//	this should be a number bigger than AVT_COUNT, 65535 set arbitrarily
#define IDS_USERNAME			"User/Name"
#define IDS_USERNAME_VAL		""
#define IDS_USERFIRSTNAME		"User/FirstName"
#define IDS_USERFIRSTNAME_VAL	""
#define IDS_USERLASTNAME		"User/LastName"
#define IDS_USERLASTNAME_VAL	""
#define IDS_USERABOUT			"User/About"
#define IDS_USERABOUT_VAL		""
#define IDS_NOTE				"User/Note" // TODO !!! Not loaded
#define IDS_NOTE_VAL			""

#define IDS_GROUPHDR			"Groups"
#define IDS_GROUP				"Group"
#define IDS_GROUPNAME			"GroupName"
#define IDS_GROUPEXPHDR			"GroupExp"
#define IDS_GROUPMAPHDR			"GroupMap"
#define IDS_USER				"User"
#define IDS_BROADCASTHDR		"BroadcastList"
#define IDS_BROADCAST			"Broadcast"

class lmcSettingsBase : public QSettings {
public:
    lmcSettingsBase();
    lmcSettingsBase(const QString& fileName, Format format);
    lmcSettingsBase(Format format, Scope scope, const QString& organization, const QString& application);
    ~lmcSettingsBase();

    using QSettings::setValue;
    void setValue(const QString& key, const QVariant& value);
};

class lmcSettings : public lmcSettingsBase {
public:
    lmcSettings();
    ~lmcSettings() {}

    bool loadFromConfig(const QString& configFile);

    static void setAutoStart(bool on);
    void saveDefaults();
};

#endif // SETTINGS_H
