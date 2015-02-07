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


#ifndef STRINGS_H
#define STRINGS_H

#include <QStringList>
#include <QCoreApplication>

class lmcStrings {
    Q_DECLARE_TR_FUNCTIONS(lmcStrings)

public:
    lmcStrings();
    ~lmcStrings();

    static void retranslate();
    static const QString appName();
    static const QString appDesc();
    static const QString autoConn();
    static const QStringList soundDesc();
    static const QStringList awayTimeDesc();
    static const QStringList userListView();

private:
    static QString m_appName;
    static QString m_appDesc;
    static QString m_autoConn;
    static QStringList m_fontSize;
    static QStringList m_statusDesc;
    static QStringList m_soundDesc;
    static QStringList m_awayTimeDesc;
    static QStringList m_userListView;
};

#endif // STRINGS_H
