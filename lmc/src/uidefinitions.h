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


#ifndef UIDEFINITIONS_H
#define UIDEFINITIONS_H

#include <QString>
#include "definitions.h"
#include "strings.h"

//	Resource name definitions
// TODO remove resource names definitions

#define IDR_LICENSETEXT		":/text/license"
#define IDR_THANKSTEXT		":/text/thanks"
#define IDR_LANG			":/lang"

//	item data role definitions
enum ItemDataRole {
    IdRole = Qt::UserRole + 1,	// Unique Id of the item
    TypeRole,	//	Whether item represents a Group or a User
    DataRole,	//	Custom data associated with the item
    StatusRole,	//	Status of the User
    AvatarRole,	//	Avatar image of the User
    SubtextRole,//	Subtext to be displayed on the item
    CapsRole    //  The capabilities of the user
};

//	Sound events definitions
enum SoundEvent {
    SE_NewMessage = 0,
    SE_UserOnline,
    SE_UserOffline,
    SE_NewFile,
    SE_FileDone,
    SE_NewPubMessage,
    SE_Max
};

#define SND_NEWMESSAGE		"./sounds/newmessage.wav"
#define SND_USERONLINE		"./sounds/useronline.wav"
#define SND_USEROFFLINE		"./sounds/useroffline.wav"
#define SND_NEWFILE			"./sounds/newfile.wav"
#define SND_FILEDONE		"./sounds/filedone.wav"
#define SND_NEWPUBMESSAGE	SND_NEWMESSAGE

#define SE_COUNT	6
const QString soundFile[] = {SND_NEWMESSAGE, SND_USERONLINE, SND_USEROFFLINE, SND_NEWFILE, SND_FILEDONE, SND_NEWPUBMESSAGE};

#define AT_COUNT	8
const int awayTimeVal[] = {5, 10, 15, 20, 30, 45, 60, 0};

//	User list views
enum UserListView {
    ULV_Detailed = 0,
    ULV_Compact,
    ULV_Max
};

#define ULV_COUNT	2
const int itemViewHeight[] = {36, 20};

#define RTL_LAYOUT			"RTL"

#ifdef Q_OS_MAC
#define GRAY_TEXT_COLOR     QApplication::palette().color(QPalette::Shadow).darker(175)
#else
#define GRAY_TEXT_COLOR     QApplication::palette().color(QPalette::Shadow)
#endif

#endif // UIDEFINITIONS_H
