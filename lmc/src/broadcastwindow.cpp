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

#include "broadcastwindow.h"
#include "thememanager.h"
#include "globals.h"

#include <QMessageBox>
#include <QDesktopWidget>

//	constructor
lmcBroadcastWindow::lmcBroadcastWindow(User *localUser, QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    setProperty("isWindow", true);

    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);

    this->localUser = localUser;

    //	set up the initial default size of splitter panels
    //	left panel takes up 60% of total width, right panel the rest
    QList<int> sizes;
    sizes.append(width() * 0.6);
    sizes.append(width() - width() * 0.6 - ui.splitter->handleWidth());
    ui.splitter->setSizes(sizes);

    connect(ui.buttonSelectAll, &ThemedButton::clicked, this, &lmcBroadcastWindow::buttonSelectAll_clicked);
    connect(ui.buttonSelectNone, &ThemedButton::clicked, this, &lmcBroadcastWindow::buttonSelectNone_clicked);
    connect(ui.treeWidgetUserList, &lmcUserTreeWidget::selectedItemsChanged,
        this, &lmcBroadcastWindow::treeWidgetUserList_selectedItemsChanged);
    connect(ui.buttonSend, &ThemedButton::clicked, this, &lmcBroadcastWindow::buttonSend_clicked);
    connect(ui.buttonCancel, &ThemedButton::clicked, this, &lmcBroadcastWindow::close);

    //	event filters for handling keyboard input
    ui.textBoxMessage->installEventFilter(this);
    ui.treeWidgetUserList->installEventFilter(this);
    ui.buttonSend->installEventFilter(this);
    ui.buttonCancel->installEventFilter(this);
    ui.buttonSelectAll->installEventFilter(this);
    ui.buttonSelectNone->installEventFilter(this);

    ui.labelInfo->setVisible(false);
    infoFlag = IT_Ok;

    parentToggling = false;
    childToggling = false;
}

lmcBroadcastWindow::~lmcBroadcastWindow() {
}

void lmcBroadcastWindow::init(bool connected) {
    createToolBar();

    setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));
    ui.splitter->setStyleSheet(QString("QSplitter::handle { image: url(%1); }").arg(ThemeManager::getInstance().getAppIcon(QStringLiteral("hgrip"))));

    ui.treeWidgetUserList->setIconSize(QSize(16, 16));
    ui.treeWidgetUserList->header()->setDragEnabled (false);
    ui.treeWidgetUserList->header()->setStretchLastSection(false);
    ui.treeWidgetUserList->header()->setSectionResizeMode (0, QHeaderView::Stretch);
    ui.treeWidgetUserList->setCheckable(true);

    //	load settings
    if (!Globals::getInstance().broadcastWindowGeometry().isEmpty())
        restoreGeometry(Globals::getInstance().broadcastWindowGeometry());
    else
        move(50, 50);

    if (!Globals::getInstance().broadcastSplitterGeometry().isEmpty())
        ui.splitter->restoreState(Globals::getInstance().broadcastSplitterGeometry());

    ui.treeWidgetUserList->setView(Globals::getInstance().userListView());

    //	show a message if not connected
    bConnected = connected;
    ui.buttonSend->setEnabled(bConnected);
    if(!bConnected)
        showStatus(IT_Disconnected, true);

    setUIText();

    ui.textBoxMessage->setFocus();
}

void lmcBroadcastWindow::stop() {
    //	save window geometry and splitter panel sizes
    Globals::getInstance().setBroadcastWindowGeometry(saveGeometry());
    Globals::getInstance().setBroadcastSplitterGeometry(ui.splitter->saveState());
}

void lmcBroadcastWindow::show(const QList<QTreeWidgetItem *> &pGroupList, const QStringList &selectedUsers) {
    QWidget::show();

    if(!pGroupList.size())
        return;

    bool userSelected;
    bool allChildrenChecked;

    //	populate the user tree
    ui.treeWidgetUserList->clear();
    for(int index = 0; index < pGroupList.count(); index++) {
        allChildrenChecked = true;
        QTreeWidgetItem* item = pGroupList.value(index);
        for(int childIndex = 0; childIndex < item->childCount(); childIndex++) {
            userSelected = selectedUsers.contains(item->child(childIndex)->data(0, IdRole).toString());
            item->child(childIndex)->setCheckState(0, userSelected ? Qt::Checked : Qt::Unchecked);
            allChildrenChecked = allChildrenChecked && userSelected;
        }
        item->setCheckState(0, allChildrenChecked ? Qt::Checked : Qt::Unchecked);
        ui.treeWidgetUserList->addTopLevelItem(item);
    }
    ui.treeWidgetUserList->expandAll();

    if (selectedUsers.size() < 2)
        buttonSelectAll_clicked();
}

void lmcBroadcastWindow::connectionStateChanged(bool connected) {
    bConnected = connected;
    ui.buttonSend->setEnabled(bConnected);
    bConnected ? showStatus(IT_Disconnected, false) : showStatus(IT_Disconnected, true);
}

void lmcBroadcastWindow::settingsChanged() {
    ui.treeWidgetUserList->setView(Globals::getInstance().userListView());
}

//	this method receives keyboard events and check if Enter key or Escape key were pressed
//	if so, corresponding functions are called
bool lmcBroadcastWindow::eventFilter(QObject* object, QEvent* event) {
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
        if(pKeyEvent->key() == Qt::Key_Escape) {
            close();
            return true;
        } else if((pKeyEvent->key() == Qt::Key_Return || pKeyEvent->key() == Qt::Key_Enter) && object == ui.textBoxMessage) {
            bool sendByEnter = ((pKeyEvent->modifiers() & Qt::ControlModifier) == Qt::NoModifier);
            if(sendByEnter == Globals::getInstance().sendByEnter()) {
                sendMessage();
                return true;
            }
            // The TextEdit widget does not insert new line when Ctrl+Enter is pressed
            // So we insert a new line manually
            if(!sendByEnter)
                ui.textBoxMessage->insertPlainText("\n");
        }
    }

    return false;
}

void lmcBroadcastWindow::changeEvent(QEvent* event) {
    switch(event->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QWidget::changeEvent(event);
}

void lmcBroadcastWindow::closeEvent(QCloseEvent* event) {
    ui.textBoxMessage->clear();
    buttonSelectNone_clicked();

    QWidget::closeEvent(event);
}

void lmcBroadcastWindow::moveEvent(QMoveEvent *event)
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

//	insert a smiley into the text box
void lmcBroadcastWindow::smileyAction_triggered() {
    //	nSmiley contains index of smiley
   insertSmileyCode(ImagesList::getInstance().getSmileys()[nSmiley]);
}

//	insert a smiley into the text box
void lmcBroadcastWindow::emojiAction_triggered() {
    insertSmileyCode(ImagesList::getInstance().getEmojis()[nEmoji]);
}

void lmcBroadcastWindow::insertSmileyCode(const ImagesStruct &smiley) {
    QString emojiCode = "%1";

    QString text = ui.textBoxMessage->toPlainText();
    QTextCursor cursor = ui.textBoxMessage->textCursor();
    if (!text.isEmpty()) {
        if (cursor.anchor() != 0 && text.at(cursor.anchor() - 1) != ' ')
            emojiCode.prepend(' ');
    }

    bool moveCursor = true;
    if (text.length() <= cursor.position() || text.at(cursor.position()) != ' ') {
        moveCursor = false;
        emojiCode.append(' ');
    }

    ui.textBoxMessage->insertPlainText(emojiCode.arg(smiley.code));

    if (moveCursor) {
        cursor.movePosition(QTextCursor::Right); // Move cursor to the right by 1 unit
        ui.textBoxMessage->setTextCursor(cursor);
    }
    ui.textBoxMessage->setFocus();
}

//	select all users in the user tree
void lmcBroadcastWindow::buttonSelectAll_clicked() {
    for(int index = 0; index < ui.treeWidgetUserList->topLevelItemCount(); index++)
        ui.treeWidgetUserList->topLevelItem(index)->setCheckState(0, Qt::Checked);
}

//	deselect all users in the user tree
void lmcBroadcastWindow::buttonSelectNone_clicked() {
    for(int index = 0; index < ui.treeWidgetUserList->topLevelItemCount(); index++)
        ui.treeWidgetUserList->topLevelItem(index)->setCheckState(0, Qt::Unchecked);
}


void lmcBroadcastWindow::treeWidgetUserList_selectedItemsChanged(unsigned count) {
    ui.buttonSend->setEnabled(count > 0);
}

void lmcBroadcastWindow::buttonSend_clicked() {
    sendMessage();
}

//	create toolbar and add buttons
void lmcBroadcastWindow::createToolBar() {
    //	create the toolbar
    _toolbar = new QToolBar(ui.toolBarWidget);
    _toolbar->setStyleSheet("QToolBar { border: 0px; background: transparent; padding: 0px; margin: 0px }");
    _toolbar->setIconSize(QSize(26, 26));
    ui.toolBarLayout->addWidget(_toolbar);

    QMenu* smileyMenu = new QMenu(this);
    lmcImagePickerAction* smileyAction = new lmcImagePickerAction(smileyMenu, ImagesList::getInstance ().getSmileys (), ImagesList::getInstance ().getTabs (ImagesList::Smileys), 39, 39, 10, &nSmiley, true);
    connect(smileyAction, &lmcImagePickerAction::imageSelected, this, &lmcBroadcastWindow::smileyAction_triggered);
    smileyMenu->addAction(smileyAction);

    QMenu* emojiMenu = new QMenu(this);
    lmcImagePickerAction* emojiAction = new lmcImagePickerAction(
        emojiMenu, ImagesList::getInstance().getEmojis(),
        ImagesList::getInstance().getTabs(ImagesList::Emojis), 78, 39, 8, &nEmoji,
        true);
    connect(emojiAction, &lmcImagePickerAction::imageSelected, this,
            &lmcBroadcastWindow::emojiAction_triggered);
    emojiMenu->addAction(emojiAction);

    //	create the smiley tool button
    _buttonSmiley = new ThemedButton(_toolbar);
    _buttonSmiley->setPopupMode(ThemedButton::InstantPopup);
    _buttonSmiley->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _buttonSmiley->setIconFitSize(true);
    _buttonSmiley->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("smiley"))));
    _buttonSmiley->setMenu(smileyMenu);
    _toolbar->addWidget(_buttonSmiley);

    _buttonEmoji = new ThemedButton(_toolbar);
    _buttonEmoji->setPopupMode(QToolButton::InstantPopup);
    _buttonEmoji->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _buttonEmoji->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("emoji"))));
    _buttonEmoji->setIconFitSize(true);
    _buttonEmoji->setMenu(emojiMenu);
    _toolbar->addWidget(_buttonEmoji);
}

void lmcBroadcastWindow::setUIText() {
    ui.retranslateUi(this);

    setWindowTitle(tr("Send Broadcast Message"));

    _buttonSmiley->setText(tr("Smiley"));
    _buttonSmiley->setToolTip(tr("Insert Smiley"));
    _buttonEmoji->setText(tr("Emoji"));
    _buttonEmoji->setToolTip(tr("Insert Emoji"));
}

//	send the broadcast message to all selected users
void lmcBroadcastWindow::sendMessage() {
    //	return if text box is empty
    if(ui.textBoxMessage->document()->isEmpty())
        return;

    //	send only if connected
    if(bConnected) {
        QString szMessage(ui.textBoxMessage->toPlainText());

        //	send broadcast
        int sendCount = 0;
        XmlMessage xmlMessage;
        xmlMessage.addHeader(XN_TIME, QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
        xmlMessage.addData(XN_MESSAGE, szMessage);
        xmlMessage.addData(XN_STATUS, localUser->status);
        xmlMessage.addData(XN_NAME, localUser->name);
        xmlMessage.addData(XN_FONT, Globals::getInstance().messagesFontString());
        xmlMessage.addData(XN_COLOR, Globals::getInstance().messagesColor());

        for(int index = 0; index < ui.treeWidgetUserList->topLevelItemCount(); index++) {
            for(int childIndex = 0; childIndex < ui.treeWidgetUserList->topLevelItem(index)->childCount(); childIndex++) {
                QTreeWidgetItem* item = ui.treeWidgetUserList->topLevelItem(index)->child(childIndex);
                if(item->checkState(0) == Qt::Checked) {
                    QString szUserId = item->data(0, IdRole).toString();
                    emit messageSent(MT_Broadcast, szUserId, xmlMessage);
                    sendCount++;
                }
            }
        }

        if(sendCount == 0) {
            QMessageBox::warning(this, tr("No recipient selected"),
                tr("Please select at least one recipient to send a broadcast."));
            return;
        }

        ui.textBoxMessage->clear();
        close();
    }
}

//	show a message depending on the connection state
void lmcBroadcastWindow::showStatus(int flag, bool add) {
    infoFlag = add ? infoFlag | flag : infoFlag & ~flag;

    ui.labelInfo->setStyleSheet("QLabel { background-color:white;font-size:9pt; }");
    if(infoFlag & IT_Disconnected) {
        ui.labelInfo->setText("<span style='color:rgb(192, 0, 0);'>" +
            tr("You are no longer connected. Broadcast message cannot be sent.") + "</span>");
        ui.labelInfo->setVisible(true);
    } else {
        ui.labelInfo->setText(QString::null);
        ui.labelInfo->setVisible(false);
    }
}
