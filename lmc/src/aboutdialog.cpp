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

#include "aboutdialog.h"
#include "thememanager.h"
#include "globals.h"

#include <QDesktopWidget>
#include <QFile>

//	constructor
lmcAboutDialog::lmcAboutDialog(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
    ui.setupUi(this);
    setProperty("isWindow", true);

    //	set minimum size
    layout()->setSizeConstraint(QLayout::SetMinimumSize);
    //	remove the help button from window button group
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui.btnClose, &ThemedButton::clicked, this, &lmcAboutDialog::close);
}

lmcAboutDialog::~lmcAboutDialog() {
}

void lmcAboutDialog::init() {
    setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

    setUIText();

    ui.tabWidget->setCurrentIndex(0);
}

void lmcAboutDialog::settingsChanged() {
}

void lmcAboutDialog::changeEvent(QEvent* pEvent) {
    switch(pEvent->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QDialog::changeEvent(pEvent);
}

void lmcAboutDialog::moveEvent(QMoveEvent *event)
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

void lmcAboutDialog::setUIText() {
    ui.retranslateUi(this);

    QString title = tr("About %1");
    setWindowTitle(title.arg(lmcStrings::appName()));

    ui.lblTitle->setText(lmcStrings::appName() + "\n" IDA_VERSION);
    ui.lblLogoSmall->setPixmap(QPixmap(ThemeManager::getInstance().getAppIcon("logosmall")));

    QString description(lmcStrings::appDesc() + "\n\n");
    description.append(IDA_COPYRIGHT "\n" IDA_DOMAIN);
    ui.lblDescription->setText(description);

    QFile thanks(IDR_THANKSTEXT);
    if(thanks.open(QIODevice::ReadOnly)) {
        ui.txtThanks->setPlainText(QString(thanks.readAll().constData()));
        thanks.close();
    }

    QFile license(IDR_LICENSETEXT);
    if(license.open(QIODevice::ReadOnly)) {
        ui.txtLicense->setPlainText(QString(license.readAll().constData()));
        license.close();
    }
}
