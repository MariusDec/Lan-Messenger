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

#include "historywindow.h"
#include "thememanager.h"
#include "loggermanager.h"

#include <QDesktopWidget>

lmcHistoryWindow::lmcHistoryWindow(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.lmcHistoryWindow started"));

    ui.setupUi(this);

    setProperty("isWindow", true);

    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);

    pMessageLog = new lmcMessageLog(ui.frameMessageLog);
    ui.frameMessageLog_layout->addWidget(pMessageLog);
    pMessageLog->setAcceptDrops(false);

    QList<int> sizes;
    sizes.append(width() * 0.35);
    sizes.append(width() - width() * 0.35 - ui.splitter->handleWidth());
    ui.splitter->setSizes(sizes);
    QRect scr = QApplication::desktop()->screenGeometry();
    move(scr.center() - rect().center());

    connect(ui.treeWidgetMsgList, &QTreeWidget::currentItemChanged,
        this, &lmcHistoryWindow::treeWidgetMsgList_currentItemChanged);
    connect(ui.buttonClearHistory, &ThemedButton::clicked, this, &lmcHistoryWindow::buttonClearHistory_clicked);
    connect(ui.buttonClose, &ThemedButton::clicked, this, &lmcHistoryWindow::buttonClose_clicked);
    connect(ui.comboBoxHistoryFilter, &ThemedComboBox::currentTextChanged, this, &lmcHistoryWindow::comboBoxHistoryFilter_textChanged);

    ui.treeWidgetMsgList->installEventFilter(this);
    pMessageLog->installEventFilter(this);
    ui.buttonClearHistory->installEventFilter(this);
    ui.buttonClose->installEventFilter(this);

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.lmcHistoryWindow ended"));
}

lmcHistoryWindow::~lmcHistoryWindow() {
}

void lmcHistoryWindow::init() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.init started"));

    setWindowIcon(QIcon(ThemeManager::getInstance ().getAppIcon (QStringLiteral("messenger"))));
    ui.splitter->setStyleSheet(QString("QSplitter::handle { image: url(%1); }").arg (ThemeManager::getInstance ().getAppIcon (QStringLiteral("hgrip"))));

    pMessageLog->setAutoScroll(false);

    pSettings = new lmcSettings();
    restoreGeometry(pSettings->value(IDS_WINDOWHISTORY).toByteArray());
    ui.splitter->restoreState(pSettings->value(IDS_SPLITTERHISTORY).toByteArray());
    setUIText();

    displayList();

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.init ended"));
}

void lmcHistoryWindow::updateList() {
    displayList();
}

void lmcHistoryWindow::stop() {
    pSettings->setValue(IDS_WINDOWHISTORY, saveGeometry());
    pSettings->setValue(IDS_SPLITTERHISTORY, ui.splitter->saveState());
}

void lmcHistoryWindow::settingsChanged() {
}

void lmcHistoryWindow::setUserFilter(const QString &userName) {
    LoggerManager::getInstance().writeInfo(
        QString("lmcMainWindow.setUserFilter started-|- user: %1").arg(userName));

    int itemIndex;
    if ((itemIndex = ui.comboBoxHistoryFilter->findText(userName, Qt::MatchFixedString | Qt::MatchCaseSensitive)) < 0)
        return;

    if (!userName.compare(QStringLiteral("All users"))) {
        for(int index = 0; index < ui.treeWidgetMsgList->topLevelItemCount (); ++index)
            ui.treeWidgetMsgList->topLevelItem (index)->setHidden (false);

    } else {
        for(int index = 0; index < ui.treeWidgetMsgList->topLevelItemCount (); ++index) {
            QTreeWidgetItem *item = ui.treeWidgetMsgList->topLevelItem (index);
            QString itemUserName = item->text(0);
            if (!userName.compare (itemUserName))
                item->setHidden (false);
            else {
                item->setHidden (true);
            }
        }
    }

    ui.comboBoxHistoryFilter->setCurrentIndex(itemIndex);

    LoggerManager::getInstance().writeInfo(
                QStringLiteral("lmcMainWindow.setUserFilter ended"));
}

bool lmcHistoryWindow::eventFilter(QObject* pObject, QEvent* pEvent) {
    Q_UNUSED(pObject);
    if(pEvent->type() == QEvent::KeyPress) {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        if(pKeyEvent->key() == Qt::Key_Escape) {
            close();
            return true;
        }
    }

    return false;
}

void lmcHistoryWindow::changeEvent(QEvent* pEvent) {
    switch(pEvent->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QWidget::changeEvent(pEvent);
}

void lmcHistoryWindow::treeWidgetMsgList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.treeWidgetMsgList_currentItemChanged started"));

    Q_UNUSED(previous);

    if(current) {
        QString filePath = current->data(0, DataRole).toString();
        QString tstamp = current->data(1, DataRole).toString();
        QString data = History::getMessage(filePath, tstamp, false);

        pMessageLog->setHtml(data);
    }

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.treeWidgetMsgList_currentItemChanged ended"));
}

void lmcHistoryWindow::buttonClearHistory_clicked() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.buttonClearHistory_clicked started"));

    QDir dir(History::historyFilesDir());
    dir.setNameFilters(QStringList() << "*.xml");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
        dir.remove(dirFile);

    displayList();

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.buttonClearHistory_clicked ended"));
}

void lmcHistoryWindow::buttonClose_clicked()
{
    close ();
}

void lmcHistoryWindow::comboBoxHistoryFilter_textChanged(const QString &text)
{
    setUserFilter (text);
}

void lmcHistoryWindow::setUIText() {
    ui.retranslateUi(this);

    setWindowTitle(tr("Message History"));
}

void lmcHistoryWindow::displayList() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.displayList started"));

    pMessageLog->setHtml("<html></html>");
    ui.treeWidgetMsgList->clear();
    msgList.clear();

    msgList = History::getList();

    for(const MsgInfo &msg : msgList) {
        lmcHistoryTreeWidgetItem* pItem = new lmcHistoryTreeWidgetItem();
        pItem->setText(0, msg.name);
        pItem->setToolTip(0, msg.name);
        pItem->setData(0, DataRole, msg.fileName);
        pItem->setText(1, msg.date.toString(Qt::SystemLocaleDate));
        pItem->setData(1, DataRole, msg.tstamp);
        pItem->setSizeHint(0, QSize(0, 20));
        ui.treeWidgetMsgList->addTopLevelItem(pItem);
    }

    ui.treeWidgetMsgList->sortByColumn(1, Qt::DescendingOrder);

    if(ui.treeWidgetMsgList->topLevelItemCount() > 0)
        ui.treeWidgetMsgList->setCurrentItem(ui.treeWidgetMsgList->topLevelItem(0));

    ui.comboBoxHistoryFilter->clear ();
    ui.comboBoxHistoryFilter->addItem (tr("All users"), QString());
    QList<QString> users = getUniqueUsers ();
    for (QString user : users)
        ui.comboBoxHistoryFilter->addItem (user);

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcMainWindow.displayList ended"));
}

QList<QString> lmcHistoryWindow::getUniqueUsers()
{
    QList<QString> users;

    for (const MsgInfo &message : msgList) {
        if (!users.contains (message.name))
            users.append (message.name);
    }

    return users;
}
