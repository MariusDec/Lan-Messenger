﻿/****************************************************************************
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


#include "application.h"
#include "lmc.h"
#include "globals.h"
#include "loggermanager.h"

#include <QResource>
#include <QMessageBox>
#include <QImageReader>


//	Define a unique application id. This is a combination of two GUIDs
const QString appId = "93fab548-2cf5-4a1e-8758-a416a5ec2120-6fc5009f-84e8-4489-a444-7f934bcf9166";

int showSwitches() {
    QString msg =	"Usage:	lmc [switches]\n" \
                    "	All switches are optional.\n" \
                    "\n" \
                    "/loopback - Allows loopback communication with local machine.\n" \
                    "/nohistory - Deletes existing history.\n" \
                    "/nofilehistory - Deletes existing file transfer history.\n" \
                    "/noconfig - Resets preferences to default values.\n" \
                    "/sync - Synchronizes application settings and their external dependencies.\n" \
                    "/unsync - Removes all external application dependencies.\n" \
                    "/term - Closes an instance which is already running.\n" \
                    "/inst - Returns a value indicating whether an instance is already running.\n" \
                    "/quit - Closes once the command line parameters are processed.\n" \
                    "/? - Display this help.\n" \
                    "\n" \
                    "Example:\n" \
                    "Start in loopback mode with default settings: lmc /loopback /noconfig\n" \
                    "\n" \
                    "Some command line switches are mutually exclusive. If multiple switches\n" \
                    "are specified, they will take precedence in the order given above.\n" \
                    "Copyright (C) 2010-2012 Qualia Digital Solutions.\n";
    QMessageBox::information(NULL, IDA_TITLE, msg, QMessageBox::Ok);
    return 0;
}

int main(int argc, char *argv[]) {
    Application application(argc, argv);

    QDir::setCurrent(QApplication::applicationDirPath());

#ifdef Q_OS_MAC
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("Plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#else
    QApplication::setLibraryPaths(QStringList(QApplication::applicationDirPath()));
#endif

    QApplication::setApplicationName(IDA_PRODUCT);
    QApplication::setOrganizationName(IDA_COMPANY);
    QApplication::setOrganizationDomain(IDA_DOMAIN);
    QApplication::setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

    LoggerManager::getInstance ().writeSimpleMessage (QString("============= Application started - %1 =================\n").arg (QDateTime::currentDateTime ().toString ()));

    Globals::getInstance().loadSettings();

    QString messageList;
    QStringList arguments = QApplication::arguments();

    for(int index = 0; index < arguments.count(); index++) {
        if(arguments.at(index).compare("/?", Qt::CaseInsensitive) == 0)
            return showSwitches();
        else if(arguments.at(index).compare("/inst", Qt::CaseInsensitive) == 0)
            return application.isRunning() ? 1 : 0;
        else
            messageList += arguments.at(index) + "\n";
    }

    if(application.sendMessage(messageList)) {
        LoggerManager::getInstance ().writeInfo (QString("main -|- Application ended. MessageList: %1\tArguments: %2").arg (messageList, arguments.join (", ")));
        return 0;
    }
    LoggerManager::getInstance ().writeInfo (QStringLiteral("main -|- Connected to socket"));

    application.loadTranslations(StdLocation::translationsPath());

    //	Enable tracing for Windows and Mac
#ifndef Q_OS_LINUX
    messageList += "/trace\n";
#endif
    LoggerManager::getInstance ().writeInfo (QString("main -|- Application started. MessageList: %1\tArguments: %2").arg (messageList, arguments.join (", ")));

    lmcCore core;
    //	handle command line args if this is first instance
    //	some args are handled when the application is initializing
    core.init(messageList);
    messageList += "/new\n";	//	indicates this is a new instance
    //	the remaining args are handled after initializing all the layers
    if(!core.receiveAppMessage(messageList))
        return 0;
    if(!core.start())
        return 1;

    QObject::connect(&application, &Application::messageReceived, &core, &lmcCore::receiveAppMessage);
    QObject::connect(&application, &Application::aboutToQuit, &core, &lmcCore::aboutToExit);

    return application.exec();
}
