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

#include "userinfowindow.h"
#include "globals.h"
#include "imageslist.h"
#include "thememanager.h"

#include <QDesktopWidget>

lmcUserInfoWindow::lmcUserInfoWindow(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
    setProperty("isWindow", true);

    //	set fixed size
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    //	remove the help button from window button group
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect (ui.btnClose, &ThemedButton::clicked, this, &lmcUserInfoWindow::close);
}

lmcUserInfoWindow::~lmcUserInfoWindow() {
}

void lmcUserInfoWindow::init() {
    setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

    QFont font = ui.labelUserName->font();
    int fontSize = ui.labelUserName->fontInfo().pixelSize();
    fontSize += (fontSize * 0.1);
    font.setPixelSize(fontSize);
    font.setBold(true);
    ui.labelUserName->setFont(font);

    setUIText();
}

void lmcUserInfoWindow::setInfo(const XmlMessage &message) {
    userInfo.setContent(message.toString());
    setUIText();
    ui.tabWidget->setCurrentIndex(0);
}

void lmcUserInfoWindow::settingsChanged() {
}

void lmcUserInfoWindow::changeEvent(QEvent* pEvent) {
    switch(pEvent->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QDialog::changeEvent(pEvent);
}

void lmcUserInfoWindow::moveEvent(QMoveEvent *event)
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

void lmcUserInfoWindow::setUIText() {
    ui.retranslateUi(this);
    setWindowTitle(tr("User Information"));

    if(userInfo.header(XN_TYPE).isNull())
        return;

    QString filePath = userInfo.data(XN_AVATAR);
    //	if image not found, use the default avatar image for this user
    if(!QFile::exists(filePath))
        filePath = ImagesList::getInstance().getDefaultAvatar();
    ui.lblAvatar->setPixmap(QPixmap(filePath).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui.labelUserName->setText(userInfo.data(XN_NAME));

    StatusStruct *status = Globals::getInstance ().getStatus (userInfo.data(XN_STATUS));
    if (status)
        ui.labelStatus->setText(status->uiDescription);
    else
        ui.labelStatus->setText(tr("Not Available"));

    QString data = userInfo.data(XN_FIRSTNAME);
    if(!data.isNull() && data.compare("N/A") != 0)
        ui.txtFirstName->setText(data);
    data = userInfo.data(XN_LASTNAME);
    if(!data.isNull() && data.compare("N/A") != 0)
        ui.txtLastName->setText(data);
    data = userInfo.data(XN_ABOUT);
    if(!data.isNull() && data.compare("N/A") != 0)
        ui.txtAbout->setPlainText(data);

    ui.lblIPAddress->setText(userInfo.data(XN_ADDRESS));
    ui.lblLogonName->setText(userInfo.data(XN_LOGON));
    ui.lblComputerName->setText(userInfo.data(XN_HOST));
    ui.lblOSName->setText(userInfo.data(XN_OS));
    ui.lblVersion->setText(userInfo.data(XN_VERSION));
}
