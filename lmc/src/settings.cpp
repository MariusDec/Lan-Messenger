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


#include "settings.h"
#include "stdlocation.h"

lmcSettingsBase::lmcSettingsBase() : QSettings() {
}

lmcSettingsBase::lmcSettingsBase(const QString& fileName, Format format) :
    QSettings(fileName, format) {
}

lmcSettingsBase::lmcSettingsBase(Format format, Scope scope, const QString& organization, const QString& application) :
    QSettings(format, scope, organization, application) {
}

lmcSettingsBase::~lmcSettingsBase() {
}

void lmcSettingsBase::setValue(const QString& key, const QVariant& value) {
    QSettings::setValue(key, value);
}

//	migrate settings from older versions to new format
//	Returns false if existing settings cannot be migrated, else true
lmcSettings::lmcSettings() : lmcSettingsBase(QSettings::IniFormat, QSettings::UserScope, IDA_COMPANY, IDA_PRODUCT) {
    QString filePath = fileName();

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
        saveDefaults();
}

//	Load settings from the specified config file and overwrite corresponding
//	application settings
bool lmcSettings::loadFromConfig(const QString& configFile) {
    if(!QFile::exists(configFile))
        return false;

    if(!Helper::copyFile(configFile, StdLocation::tempConfigFile()))
        return false;

    QVariant value;
    QSettings extSettings(StdLocation::tempConfigFile(), QSettings::IniFormat);

    value = extSettings.value(IDS_VERSION);
    if(value.isValid())	setValue(IDS_VERSION, value);
    value = extSettings.value(IDS_OPENPATH);
    if(value.isValid())	setValue(IDS_OPENPATH, value);
    value = extSettings.value(IDS_SAVEPATH);
    if(value.isValid())	setValue(IDS_SAVEPATH, value);
    value = extSettings.value(IDS_WINDOWMAIN);
    if(value.isValid())	setValue(IDS_WINDOWMAIN, value);
    value = extSettings.value(IDS_WINDOWTRANSFERS);
    if(value.isValid())	setValue(IDS_WINDOWTRANSFERS, value);
    value = extSettings.value(IDS_WINDOWHISTORY);
    if(value.isValid())	setValue(IDS_WINDOWHISTORY, value);
    value = extSettings.value(IDS_WINDOWBROADCAST);
    if(value.isValid())	setValue(IDS_WINDOWBROADCAST, value);
    value = extSettings.value(IDS_WINDOWINSTANTMSG);
    if(value.isValid())	setValue(IDS_WINDOWINSTANTMSG, value);
    value = extSettings.value(IDS_WINDOWHELP);
    if(value.isValid())	setValue(IDS_WINDOWHELP, value);
    value = extSettings.value(IDS_WINDOWPUBLICCHAT);
    if(value.isValid())	setValue(IDS_WINDOWPUBLICCHAT, value);
    value = extSettings.value(IDS_WINDOWCHATROOM);
    if(value.isValid())	setValue(IDS_WINDOWCHATROOM, value);
    value = extSettings.value(IDS_SPLITTERHISTORY);
    if(value.isValid())	setValue(IDS_SPLITTERHISTORY, value);
    value = extSettings.value(IDS_SPLITTERBROADCAST);
    if(value.isValid())	setValue(IDS_SPLITTERBROADCAST, value);
    value = extSettings.value(IDS_SPLITTERINSTANTMSG);
    if(value.isValid())	setValue(IDS_SPLITTERINSTANTMSG, value);
    value = extSettings.value(IDS_SPLITTERPUBLICCHATH);
    if(value.isValid())	setValue(IDS_SPLITTERPUBLICCHATH, value);
    value = extSettings.value(IDS_SPLITTERPUBLICCHATV);
    if(value.isValid())	setValue(IDS_SPLITTERPUBLICCHATV, value);
    value = extSettings.value(IDS_SPLITTERCHATROOMH);
    if(value.isValid())	setValue(IDS_SPLITTERCHATROOMH, value);
    value = extSettings.value(IDS_SPLITTERCHATROOMV);
    if(value.isValid())	setValue(IDS_SPLITTERCHATROOMV, value);

    value = extSettings.value(IDS_AUTOSTART);
    if(value.isValid())	setValue(IDS_AUTOSTART, value);
    value = extSettings.value(IDS_AUTOSHOW);
    if(value.isValid())	setValue(IDS_AUTOSHOW, value);
    value = extSettings.value(IDS_WINDOWSNAPPING);
    if(value.isValid())	setValue(IDS_WINDOWSNAPPING, value);
    value = extSettings.value(IDS_DEFMSGACTION);
    if(value.isValid())	setValue(IDS_DEFMSGACTION, value);

    value = extSettings.value(IDS_SYSTRAY);
    if(value.isValid())	setValue(IDS_SYSTRAY, value);
    value = extSettings.value(IDS_MINIMIZETRAY);
    if(value.isValid())	setValue(IDS_MINIMIZETRAY, value);
    value = extSettings.value(IDS_SINGLECLICKTRAY);
    if(value.isValid())	setValue(IDS_SINGLECLICKTRAY, value);
    value = extSettings.value(IDS_SYSTRAYMSG);
    if(value.isValid())	setValue(IDS_SYSTRAYMSG, value);
    value = extSettings.value(IDS_SYSTRAYPUBNEWMSG);
    if(value.isValid())	setValue(IDS_SYSTRAYPUBNEWMSG, value);
    value = extSettings.value(IDS_SYSTRAYNEWMSG);
    if(value.isValid())	setValue(IDS_SYSTRAYNEWMSG, value);
    value = extSettings.value(IDS_ALLOWSYSTRAYMIN);
    if(value.isValid())	setValue(IDS_ALLOWSYSTRAYMIN, value);
    value = extSettings.value(IDS_DEFMSGACTION);
    if(value.isValid())	setValue(IDS_DEFMSGACTION, value);
    value = extSettings.value(IDS_RESTORESTATUS);
    if (value.isValid()) setValue(IDS_RESTORESTATUS, value);
    value = extSettings.value(IDS_IDLETIME);
    if (value.isValid()) setValue(IDS_IDLETIME, value);
    value = extSettings.value(IDS_LANGUAGE);
    if(value.isValid())	setValue(IDS_LANGUAGE, value);

    value = extSettings.value(IDS_USERNAME);
    if(value.isValid())	setValue(IDS_USERNAME, value);
    value = extSettings.value(IDS_USERFIRSTNAME);
    if(value.isValid())	setValue(IDS_USERFIRSTNAME, value);
    value = extSettings.value(IDS_USERLASTNAME);
    if(value.isValid())	setValue(IDS_USERLASTNAME, value);
    value = extSettings.value(IDS_USERABOUT);
    if(value.isValid())	setValue(IDS_USERABOUT, value);
    value = extSettings.value(IDS_REFRESHTIME);
    if(value.isValid())	setValue(IDS_REFRESHTIME, value);

    value = extSettings.value(IDS_MESSAGEPOP);
    if(value.isValid())	setValue(IDS_MESSAGEPOP, value);
    value = extSettings.value(IDS_PUBMESSAGEPOP);
    if(value.isValid())	setValue(IDS_PUBMESSAGEPOP, value);
    value = extSettings.value(IDS_EMOTICON);
    if(value.isValid())	setValue(IDS_EMOTICON, value);
    value = extSettings.value(IDS_CONFIRMLEAVECHAT);
    if(value.isValid())	setValue(IDS_CONFIRMLEAVECHAT, value);
    value = extSettings.value(IDS_MESSAGETIME);
    if(value.isValid())	setValue(IDS_MESSAGETIME, value);
    value = extSettings.value(IDS_MESSAGEDATE);
    if(value.isValid())	setValue(IDS_MESSAGEDATE, value);
    value = extSettings.value(IDS_ALLOWLINKS);
    if(value.isValid())	setValue(IDS_ALLOWLINKS, value);
    value = extSettings.value(IDS_PATHTOLINK);
    if(value.isValid())	setValue(IDS_PATHTOLINK, value);
    value = extSettings.value(IDS_TRIMMESSAGE);
    if(value.isValid())	setValue(IDS_TRIMMESSAGE, value);
    value = extSettings.value(IDS_APPENDHISTORY);
    if(value.isValid())	setValue(IDS_APPENDHISTORY, value);
    value = extSettings.value(IDS_FONT);
    if(value.isValid())	setValue(IDS_FONT, value);
    value = extSettings.value(IDS_COLOR);
    if(value.isValid())	setValue(IDS_COLOR, value);
    value = extSettings.value(IDS_OVERRIDEINMSG);
    if(value.isValid())	setValue(IDS_OVERRIDEINMSG, value);
    value = extSettings.value(IDS_SHOWCHARCOUNT);
    if(value.isValid())	setValue(IDS_SHOWCHARCOUNT, value);
    value = extSettings.value(IDS_INFORMREAD);
    if(value.isValid())	setValue(IDS_INFORMREAD, value);

    value = extSettings.value(IDS_HISTORY);
    if(value.isValid())	setValue(IDS_HISTORY, value);
    value = extSettings.value(IDS_SYSHISTORYPATH);
    if(value.isValid())	setValue(IDS_SYSHISTORYPATH, value);
    value = extSettings.value(IDS_HISTORYPATH);
    if(value.isValid())	setValue(IDS_HISTORYPATH, value);
    value = extSettings.value(IDS_FILEHISTORY);
    if(value.isValid())	setValue(IDS_FILEHISTORY, value);
    value = extSettings.value(IDS_FILEHISTORYPATH);
    if(value.isValid())	setValue(IDS_FILEHISTORYPATH, value);

    value = extSettings.value(IDS_ALERT);
    if(value.isValid())	setValue(IDS_ALERT, value);
    value = extSettings.value(IDS_NOBUSYALERT);
    if(value.isValid())	setValue(IDS_NOBUSYALERT, value);
    value = extSettings.value(IDS_NODNDALERT);
    if(value.isValid())	setValue(IDS_NODNDALERT, value);
    value = extSettings.value(IDS_SOUND);
    if(value.isValid())	setValue(IDS_SOUND, value);
    value = extSettings.value(IDS_NOBUSYSOUND);
    if(value.isValid())	setValue(IDS_NOBUSYSOUND, value);
    value = extSettings.value(IDS_NODNDSOUND);
    if(value.isValid())	setValue(IDS_NODNDSOUND, value);

    value = extSettings.value(IDS_CONNECTION);
    if(value.isValid())	setValue(IDS_CONNECTION, value);
    value = extSettings.value(IDS_TIMEOUT);
    if(value.isValid())	setValue(IDS_TIMEOUT, value);
    value = extSettings.value(IDS_MAXRETRIES);
    if(value.isValid())	setValue(IDS_MAXRETRIES, value);
    QStringList broadcastList;
    int size = extSettings.beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        extSettings.setArrayIndex(index);
        broadcastList.append(extSettings.value(IDS_BROADCAST).toString());
    }
    extSettings.endArray();
    if(size > 0) {
        beginWriteArray(IDS_BROADCASTHDR);
        for(int index = 0; index < size; index++) {
            setArrayIndex(index);
            setValue(IDS_BROADCAST, broadcastList.at(index));
        }
        endArray();
    }
    value = extSettings.value(IDS_MULTICAST);
    if(value.isValid())	setValue(IDS_MULTICAST, value);
    value = extSettings.value(IDS_UDPPORT);
    if(value.isValid())	setValue(IDS_UDPPORT, value);
    value = extSettings.value(IDS_TCPPORT);
    if(value.isValid())	setValue(IDS_TCPPORT, value);
    value = extSettings.value(IDS_ERPADDRESS);
    if (value.isValid()) setValue(IDS_ERPADDRESS, value);

    value = extSettings.value(IDS_AUTOFILE);
    if(value.isValid())	setValue(IDS_AUTOFILE, value);
    value = extSettings.value(IDS_AUTOSHOWFILE);
    if(value.isValid())	setValue(IDS_AUTOSHOWFILE, value);
    value = extSettings.value(IDS_FILETOP);
    if(value.isValid())	setValue(IDS_FILETOP, value);

    value = extSettings.value(IDS_CHATTHEME);
    if(value.isValid())	setValue(IDS_CHATTHEME, value);
    value = extSettings.value(IDS_APPTHEME);
    if(value.isValid())	setValue(IDS_APPTHEME, value);
    value = extSettings.value(IDS_BUTTONTHEME);
    if(value.isValid())	setValue(IDS_BUTTONTHEME, value);
    value = extSettings.value(IDS_USERLISTVIEW);
    if(value.isValid())	setValue(IDS_USERLISTVIEW, value);

    value = extSettings.value(IDS_SENDBYENTER);
    if(value.isValid())	setValue(IDS_SENDBYENTER, value);

    setValue(IDS_VERSION, IDA_VERSION);
    sync();

    QFile::remove(StdLocation::tempConfigFile());

    return true;
}

void lmcSettings::setAutoStart(bool on) {
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    if(on)
        settings.setValue(IDA_TITLE, QDir::toNativeSeparators(QApplication::applicationFilePath()));
    else
        settings.remove(IDA_TITLE);
#endif

#ifdef Q_OS_MAC
    Q_UNUSED(on);
#endif

#ifdef Q_OS_LINUX
    //  get the path of .desktop file
    QString autoStartDir;
    char* buffer = getenv("XDG_CONFIG_HOME");
    if(buffer) {
        autoStartDir = QString(buffer);
        autoStartDir.append("/autostart");
    } else {
        buffer = getenv("HOME");
        autoStartDir = QString(buffer);
        autoStartDir.append("/.config/autostart");
    }
    QDir dir(autoStartDir);
    QString fileName = dir.absoluteFilePath("lmc.desktop");
    //	delete the file if autostart is set to false
    if(!on) {
        QFile::remove(fileName);
        return;
    }

    if(!dir.exists())
        dir.mkpath(dir.absolutePath());
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(false);
    stream << "[Desktop Entry]\n";
    stream << "Encoding=UTF-8\n";
    stream << "Type=Application\n";
    stream << "Name=" << IDA_TITLE << "\n";
    stream << "Comment=Send and receive instant messages\n";
    stream << "Icon=lmc\n";
    stream << "Exec=sh " << qApp->applicationDirPath() << "/lmc.sh\n";
    stream << "Terminal=false\n";
    file.close();
#endif
}

void lmcSettings::saveDefaults()
{
    setValue(IDS_VERSION, IDA_VERSION);

    setValue(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL);
    setValue(IDS_AUTOSHOW, IDS_AUTOSHOW_DEFAULT_VAL);
    setValue(IDS_WINDOWSNAPPING, IDS_WINDOWSNAPPING_VAL);
    setValue(IDS_SYSTRAY, IDS_SYSTRAY_VAL);
    setValue(IDS_MINIMIZETRAY, IDS_MINIMIZETRAY_VAL);
    setValue(IDS_SINGLECLICKTRAY, IDS_SINGLECLICKTRAY_VAL);
    setValue(IDS_SYSTRAYMSG, IDS_SYSTRAYMSG_VAL);
    setValue(IDS_SYSTRAYNEWMSG, IDS_SYSTRAYNEWMSG_VAL);
    setValue(IDS_SYSTRAYPUBNEWMSG, IDS_SYSTRAYPUBNEWMSG_VAL);
    setValue(IDS_ALLOWSYSTRAYMIN, IDS_ALLOWSYSTRAYMIN_VAL);
    setValue(IDS_DEFMSGACTION, IDS_DEFMSGACTION_VAL);
    setValue(IDS_RESTORESTATUS, IDS_RESTORESTATUS_VAL);
    setValue(IDS_LANGUAGE, IDS_LANGUAGE_DEFAULT_VAL);

    setValue(IDS_USERNAME, IDS_USERNAME_VAL);
    setValue(IDS_USERFIRSTNAME, IDS_USERFIRSTNAME_VAL);
    setValue(IDS_USERLASTNAME, IDS_USERLASTNAME_VAL);
    setValue(IDS_USERABOUT, IDS_USERABOUT_VAL);
    setValue(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL);

    setValue(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL);
    setValue(IDS_PUBMESSAGEPOP, IDS_PUBMESSAGEPOP_VAL);
    setValue(IDS_EMOTICON, IDS_EMOTICON_VAL);
    setValue(IDS_CONFIRMLEAVECHAT, IDS_CONFIRMLEAVECHAT_VAL);
    setValue(IDS_MESSAGETIME, IDS_MESSAGETIME_VAL);
    setValue(IDS_MESSAGEDATE, IDS_MESSAGEDATE_VAL);
    setValue(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL);
    setValue(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL);
    setValue(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL);
    setValue(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL);
    setValue(IDS_FONT, IDS_FONT_VAL);
    setValue(IDS_COLOR, IDS_COLOR_VAL);
    setValue(IDS_OVERRIDEINMSG, IDS_OVERRIDEINMSG_VAL);
    setValue(IDS_SHOWCHARCOUNT, IDS_SHOWCHARCOUNT_VAL);
    setValue(IDS_INFORMREAD, IDS_INFORMREAD_VAL);

    setValue(IDS_HISTORY, IDS_HISTORY_VAL);
    setValue(IDS_SYSHISTORYPATH, IDS_SYSHISTORYPATH_VAL);
    setValue(IDS_HISTORYPATH, IDS_HISTORYPATH_VAL);
    setValue(IDS_FILEHISTORY, IDS_FILEHISTORY_VAL);

    setValue(IDS_ALERT, IDS_ALERT_VAL);
    setValue(IDS_NOBUSYALERT, IDS_NOBUSYALERT_VAL);
    setValue(IDS_NODNDALERT, IDS_NODNDALERT_VAL);
    setValue(IDS_SOUND, IDS_SOUND_VAL);

    setValue(IDS_NOBUSYSOUND, IDS_NOBUSYSOUND_VAL);
    setValue(IDS_NODNDSOUND, IDS_NODNDSOUND_VAL);

    setValue(IDS_TIMEOUT, IDS_TIMEOUT_VAL);
    setValue(IDS_MAXRETRIES, IDS_MAXRETRIES_VAL);

    setValue(IDS_MULTICAST, IDS_MULTICAST_VAL);
    setValue(IDS_UDPPORT, IDS_UDPPORT_VAL);
    setValue(IDS_TCPPORT, IDS_TCPPORT_VAL);
    setValue(IDS_ERPADDRESS, IDS_ERPADDRESS_VAL);

    setValue(IDS_AUTOFILE, IDS_AUTOFILE_VAL);
    setValue(IDS_AUTOSHOWFILE, IDS_AUTOSHOWFILE_VAL);
    setValue(IDS_FILETOP, IDS_FILETOP_VAL);
    setValue(IDS_FILESTORAGEPATH, IDS_FILESTORAGEPATH_VAL);
    setValue(IDS_STORAGEUSERFOLDER, IDS_STORAGEUSERFOLDER_VAL);

    setValue(IDS_CHATTHEME, IDS_CHATTHEME_VAL);
    setValue(IDS_APPTHEME, IDS_APPTHEME_VAL);
    setValue(IDS_APPICONTHEME, IDS_APPICONTHEME_VAL);
    setValue(IDS_BUTTONTHEME, IDS_BUTTONTHEME_VAL);

    setValue(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL);

    setValue(IDS_SENDBYENTER, IDS_SENDBYENTER_VAL);

    sync();
}
