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

#include "userselectdialog.h"
#include "thememanager.h"

lmcUserSelectDialog::lmcUserSelectDialog(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
    setProperty("isWindow", true);

    //	remove the help button from window button group
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui.buttonOK, &ThemedButton::clicked, this, &lmcUserSelectDialog::buttonOK_clicked);
    connect(ui.buttonCancel, &ThemedButton::clicked, this, &lmcUserSelectDialog::buttonCancel_clicked);
    connect(ui.treeWidgetUserList, &lmcUserTreeWidget::selectedItemsChanged,
        this, &lmcUserSelectDialog::treeWidgetUserList_selectedItemsChanged);
}

lmcUserSelectDialog::~lmcUserSelectDialog() {
}

void lmcUserSelectDialog::init(QList<QTreeWidgetItem *> *pContactsList) {
    ui.treeWidgetUserList->setIconSize(QSize(16, 16));
    ui.treeWidgetUserList->header()->setDragEnabled (false);
    ui.treeWidgetUserList->header()->setStretchLastSection(false);
    ui.treeWidgetUserList->header()->setSectionResizeMode (0, QHeaderView::Stretch);
    ui.treeWidgetUserList->setCheckable(true);

    ui.treeWidgetUserList->clear();
    for(int index = 0; index < pContactsList->count(); index++) {
        QTreeWidgetItem* pItem = pContactsList->value(index)->clone();
        pItem->setCheckState(0, Qt::Unchecked);
        for(int childIndex = 0; childIndex < pItem->childCount(); childIndex++)
            pItem->child(childIndex)->setCheckState(0, Qt::Unchecked);
        ui.treeWidgetUserList->addTopLevelItem(pItem);
    }
    ui.treeWidgetUserList->expandAll();

    //	load settings
    pSettings = new lmcSettings();
    int viewType = pSettings->value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt();
    ui.treeWidgetUserList->setView((UserListView)viewType);

    ui.buttonOK->setEnabled(false);

    selectedContacts.clear();
    setUIText();
}

void lmcUserSelectDialog::changeEvent(QEvent* pEvent) {
    switch(pEvent->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QWidget::changeEvent(pEvent);
}

void lmcUserSelectDialog::buttonOK_clicked() {
    selectedContacts.clear();

    for(int index = 0; index < ui.treeWidgetUserList->topLevelItemCount(); index++) {
        for(int childIndex = 0; childIndex < ui.treeWidgetUserList->topLevelItem(index)->childCount(); childIndex++) {
            QTreeWidgetItem* item = ui.treeWidgetUserList->topLevelItem(index)->child(childIndex);
            if(item->checkState(0) == Qt::Checked) {
                QString szUserId = item->data(0, IdRole).toString();
                selectedContacts.append(szUserId);
            }
        }
    }
    accept();
}

void lmcUserSelectDialog::buttonCancel_clicked()
{
    reject();
}

//	event called when the user checks/unchecks a tree item
void lmcUserSelectDialog::treeWidgetUserList_selectedItemsChanged(unsigned count) {
    ui.buttonOK->setEnabled(count > 0);
}

void lmcUserSelectDialog::setUIText() {
    ui.retranslateUi(this);

    setWindowTitle(tr("Select Contacts"));
}
