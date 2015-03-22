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

#include "usertreewidget.h"

#include <QLabel>
#include <QMimeData>
#include <QMovie>
#include <QDesktopWidget>

lmcUserTreeWidgetItem::lmcUserTreeWidgetItem() : QTreeWidgetItem(UserType + 1) {
    //	make item not user checkable
    setFlags(flags() & ~Qt::ItemIsUserCheckable);
}

QRect lmcUserTreeWidgetItem::checkBoxRect(const QRect& itemRect) {
    QRect checkBoxRect(0, 0, 0, 0);

    if(data(0, TypeRole).toString() == "Group")
        checkBoxRect = QRect(itemRect.left(), itemRect.top() + ((itemRect.height() - 12) / 2), 0, 0);
    else if(data(0, TypeRole).toString() == "User")
        checkBoxRect = QRect(itemRect.left(), itemRect.top() + 4, 0, 0);

    lmcUserTreeWidget* treeWidget = static_cast<lmcUserTreeWidget*>(this->treeWidget());
    if(treeWidget->checkable()) {
        checkBoxRect.setSize(QSize(12, 12));
        checkBoxRect.moveLeft(checkBoxRect.left() + 3);
    }

    return checkBoxRect;
}

void lmcUserTreeWidgetGroupItem::addChild(QTreeWidgetItem* child) {
    lmcUserTreeWidget* treeWidget = static_cast<lmcUserTreeWidget *>(this->treeWidget());
    child->setSizeHint(0, QSize(0, itemViewHeight[treeWidget->view()]));
    QTreeWidgetItem::addChild(child);
}

bool lmcUserTreeWidgetUserItem::operator < (const QTreeWidgetItem& other) const {
    int column = treeWidget()->sortColumn();
    if(column == 0) {
        //	sort based on IP and username
        if (data(column, DataRole).toInt() < other.data(column, DataRole).toInt())
            return true;
        else if (data(column, DataRole).toInt() > other.data(column, DataRole).toInt())
            return false;
        else
            return text(column).toLower() < other.text(column).toLower();
    }

    return text(column).toLower() < other.text(column).toLower();
}

void lmcUserTreeWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();

    QPalette palette = QApplication::palette();
    QRect itemRect = option.rect;
    lmcUserTreeWidgetItem* pItem = static_cast<lmcUserTreeWidgetItem*>(index.internalPointer());
    lmcUserTreeWidget* pTreeWidget = static_cast<lmcUserTreeWidget*>(pItem->treeWidget());
    QString type = pItem->data(0, TypeRole).toString();
    QString name = pItem->data(0, Qt::DisplayRole).toString();

    int padding = 2;

    if(type == "Group") {
        //	fill the entire item area with window background color (usually white)
        painter->fillRect(itemRect, palette.window());

        //	Fill the background of the item with highlight color, and draw a border with a darker shade
        QColor fillColor = palette.color(QPalette::Highlight);
        QColor borderColor = fillColor.darker(130);
        painter->setPen(QPen(borderColor));
        painter->setBrush(QBrush(fillColor));
        itemRect.adjust(1, 1, -2, -1);
        painter->drawRect(itemRect);

        //	Draw checkbox if needed
        QRect checkBoxRect = pItem->checkBoxRect(itemRect);
        if(pTreeWidget->checkable())
            drawCheckBox(painter, palette, checkBoxRect, pItem->checkState(0));

        //	Draw the expand/collapse arrow
        painter->setBrush(QBrush(palette.color(QPalette::HighlightedText)));
        painter->setPen(QPen(palette.color(QPalette::HighlightedText)));
        QPoint points[3];
        if(option.state.testFlag(QStyle::State_Children) && option.state.testFlag(QStyle::State_Open)) {
            points[0] = QPoint(checkBoxRect.right() + 4, itemRect.top() + ((itemRect.height() - 4) / 2));
            points[1] = QPoint(checkBoxRect.right() + 12, itemRect.top() + ((itemRect.height() - 4) / 2));
            points[2] = QPoint(checkBoxRect.right() + 8, itemRect.top() + ((itemRect.height() - 4) / 2) + 4);
        } else {
            points[0] = QPoint(checkBoxRect.right() + 6, itemRect.top() + ((itemRect.height() - 7) / 2));
            points[1] = QPoint(checkBoxRect.right() + 10, itemRect.top() + ((itemRect.height() - 7) / 2) + 4);
            points[2] = QPoint(checkBoxRect.right() + 6, itemRect.top() + ((itemRect.height() - 7) / 2) + 8);
        }
        painter->drawPolygon(points, 3);

        //	Draw the text
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);
        int textFlags = Qt::AlignLeft | Qt::AlignVCenter;
        //	Leave a padding of 5px on left and right
        int leftPad = checkBoxRect.width() > 0 ? checkBoxRect.right() + 16 : 16;
        QRect textRect = itemRect.adjusted(leftPad, padding, -5, -padding);
        painter->setPen(QPen(palette.color(QPalette::HighlightedText)));
        QString text = elidedText(painter->fontMetrics(), textRect.width(), Qt::ElideRight, name);
        painter->drawText(textRect, textFlags, text);
    } else if(type == "User") {
        QColor fillColor, borderColor;
        if(index.row() % 2 == 1)
            borderColor = fillColor = palette.color(QPalette::AlternateBase);
        else
            borderColor = fillColor = palette.color(QPalette::Base);

        if(option.state.testFlag(QStyle::State_Active) && option.state.testFlag(QStyle::State_Enabled)
                && option.state.testFlag(QStyle::State_Selected)) {
            borderColor = palette.color(QPalette::Shadow);
            fillColor = palette.color(QPalette::Shadow).lighter(200);
        }

        painter->setPen(QPen(borderColor));
        painter->setBrush(QBrush(fillColor));
        itemRect.adjust(1, 0, -2, -1);
        painter->drawRect(itemRect);

        //	Draw checkbox if needed
        QRect checkBoxRect = pItem->checkBoxRect(itemRect);
        if(pTreeWidget->checkable())
            drawCheckBox(painter, palette, checkBoxRect, pItem->checkState(0));

        //	Draw the status image
        QPixmap statusImage = pItem->icon(0).pixmap(QSize(16, 16));
        int leftPad = checkBoxRect.width() > 0 ? checkBoxRect.right() + 5 : 5;
        QRect statusRect = itemRect.adjusted(leftPad, padding, 0, 0);
        statusRect.setSize(statusImage.size());
        painter->drawPixmap(statusRect, statusImage);

        //	Draw the avatar image
        QRect avatarRect = itemRect.adjusted(itemRect.width(), padding, 0, 0);
        if(pTreeWidget->view() == ULV_Detailed) {
            QVariant avatar = pItem->data(0, AvatarRole);
            if(!avatar.isNull()) {
                QPixmap avatarImage = ((QIcon)pItem->data(0, AvatarRole).value<QIcon>()).pixmap(40, 40);
                avatarRect.setLeft(avatarRect.right() - avatarImage.width() - padding);
                avatarRect.setSize(avatarImage.size());
                painter->drawPixmap(avatarRect, avatarImage);
            }
        }

        //	Draw the text
        painter->setPen(QPen(palette.color(QPalette::WindowText)));
        int textFlags = Qt::AlignLeft;
        textFlags |= (pTreeWidget->view() == ULV_Detailed ? Qt::AlignTop : Qt::AlignVCenter);
        //	Leave a padding of 5px on left and right
        QRect textRect = itemRect.adjusted(statusRect.right() + 5, padding, -(5 + avatarRect.width() + padding), -padding);
        QString text = elidedText(painter->fontMetrics(), textRect.width(), Qt::ElideRight, name);
        painter->drawText(textRect, textFlags, text);

        //	Draw sub text
        if(pTreeWidget->view() == ULV_Detailed) {
            QVariant note = pItem->data(0, SubtextRole);
            if(!note.isNull()) {
                QString userNote = note.toString();
                painter->setPen(QPen(GRAY_TEXT_COLOR));
                textFlags = Qt::AlignLeft | Qt::AlignBottom;
                text = elidedText(painter->fontMetrics(), textRect.width(), Qt::ElideRight, userNote);
                painter->drawText(textRect, textFlags, text);
            }
        }
    }

    painter->restore();
}

void lmcUserTreeWidgetDelegate::drawCheckBox(QPainter *painter, const QPalette& palette,
                                        const QRect &checkBoxRect, Qt::CheckState checkState) const {
    painter->setPen(QPen(palette.color(QPalette::Shadow)));
    painter->setBrush(palette.base());
    painter->drawRect(checkBoxRect);

    if(checkState == Qt::Checked) {
        painter->setPen(QPen(palette.text(), 1));
        painter->drawLine(checkBoxRect.left() + 2, checkBoxRect.top() + 6, checkBoxRect.left() + 4, checkBoxRect.top() + 8);
        painter->drawLine(checkBoxRect.left() + 2, checkBoxRect.top() + 7, checkBoxRect.left() + 4, checkBoxRect.top() + 9);
        painter->drawLine(checkBoxRect.left() + 2, checkBoxRect.top() + 8, checkBoxRect.left() + 4, checkBoxRect.top() + 10);
        painter->drawLine(checkBoxRect.left() + 4, checkBoxRect.top() + 8, checkBoxRect.left() + 10, checkBoxRect.top() + 2);
        painter->drawLine(checkBoxRect.left() + 4, checkBoxRect.top() + 9, checkBoxRect.left() + 10, checkBoxRect.top() + 3);
        painter->drawLine(checkBoxRect.left() + 4, checkBoxRect.top() + 10, checkBoxRect.left() + 10, checkBoxRect.top() + 4);
    }
}

lmcUserTreeWidget::lmcUserTreeWidget(QWidget* parent) : QTreeWidget(parent), dragGroup(false), dragUser(false), dragFile(false) {
    itemDelegate = new lmcUserTreeWidgetDelegate();
    setItemDelegate(itemDelegate);
    setMouseTracking(true);

    isCheckable = false;
    viewType = ULV_Detailed;

    connect(this, &lmcUserTreeWidget::itemChanged, this, &lmcUserTreeWidget::handleItemChanged);
}

lmcUserTreeWidget::~lmcUserTreeWidget() {
    for (lmcUserTreeWidgetItem *item : _tooltips.keys()) {
        delete _tooltips[item];
        _tooltips.remove(item);
    }
}

bool lmcUserTreeWidget::checkable() {
    return isCheckable;
}

void lmcUserTreeWidget::setCheckable(bool enable) {
    isCheckable = enable;
}

UserListView lmcUserTreeWidget::view() {
    return viewType;
}

void lmcUserTreeWidget::setView(UserListView view) {
    viewType = view;

    //	Set the item heights for the selected view type
    for(int index = 0; index < topLevelItemCount(); index++) {
        QTreeWidgetItem* item = topLevelItem(index);
        for(int childIndex = 0; childIndex < item->childCount(); childIndex++)
            item->child(childIndex)->setSizeHint(0, QSize(0, itemViewHeight[viewType]));
    }
}

void lmcUserTreeWidget::setItemTooltip(lmcUserTreeWidgetItem *item, QWidget *tooltip)
{
    tooltip->setAttribute(Qt::WA_ShowWithoutActivating);
    tooltip->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint);
    if (!_tooltips.contains(item))
        _tooltips.insert(item, tooltip);
    else {
        delete _tooltips[item];
        _tooltips[item] = tooltip;
    }
}

void lmcUserTreeWidget::setItemTooltipAvatar(lmcUserTreeWidgetItem *item, const QString &avatarPath)
{
    if (item && _tooltips.contains(item)){
        _tooltips[item].data()->hide();

        QLabel *label = _tooltips[item].data()->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label)
            label->setPixmap(QPixmap(avatarPath).scaled(64, 64, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

void lmcUserTreeWidget::setItemTooltipDetails(lmcUserTreeWidgetItem *item, const QString &details)
{
    if (item && _tooltips.contains(item)){
        _tooltips[item].data()->hide();
        QLabel *label = _tooltips[item].data()->findChild<QLabel *>(QStringLiteral("labelDescription"));
        if (label)
            label->setText(details);
    }
}

void lmcUserTreeWidget::removeItemTooltip(lmcUserTreeWidgetItem *item)
{
    if (_tooltips.contains(item)) {
        delete _tooltips[item];
        _tooltips.remove(item);
    }
}

void lmcUserTreeWidget::mousePressEvent(QMouseEvent* event) {
    if (_hoveredItem && _tooltips.contains(_hoveredItem)){
        _tooltips[_hoveredItem].data()->hide();
      /*
        QLabel *label = _tooltips[_hoveredItem].data()->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label && label->movie ())
            label->movie ()->stop();
      */
    }

    if(event->button() == Qt::LeftButton) {
        QTreeWidgetItem* item = itemAt(event->pos());

        dragGroup = false;
        dragUser = false;
        dragFile = false;
        dragItem = nullptr;
        parentId = QString::null;
        expanded = false;

        if(item) {
            dragItem = item;
            if(dynamic_cast<lmcUserTreeWidgetGroupItem*>(item)) {
                dragGroup = true;
                expanded = dragItem->isExpanded();
            }
            else if(dynamic_cast<lmcUserTreeWidgetUserItem*>(item)) {
                dragUser = true;
                parentId = dragItem->parent()->data(0, IdRole).toString();
            }
        }
    }

    QTreeWidget::mousePressEvent(event);
}

void lmcUserTreeWidget::dragMoveEvent(QDragMoveEvent* event) {
    QTreeWidget::dragMoveEvent(event);

    QTreeWidgetItem* item = itemAt(event->pos());
    bool accept = false;

    if(dragUser) {
        if(item && dynamic_cast<lmcUserTreeWidgetGroupItem *>(item) && visualItemRect(item).contains(event->pos(), true))
            accept = true;
    }
    else if(dragGroup) {
        if(!item || (dynamic_cast<lmcUserTreeWidgetGroupItem *>(item) && !visualItemRect(item).contains(event->pos(), true)))
            accept = true;
    } else if (dragFile) {
        if(item || (dynamic_cast<lmcUserTreeWidgetGroupItem *>(item) && visualItemRect(item).contains(event->pos(), true)))
            accept = true;
    }

    accept ? event->accept() : event->ignore();
}

void lmcUserTreeWidget::dropEvent(QDropEvent* event) {
    QTreeWidget::dropEvent(event);

    QStringList fileNames;
    if(dragUser) {
        if(!dragItem->parent()) {
        //	user item dragged to group level. revert
            for(int index = 0; index < topLevelItemCount(); index++) {
                QTreeWidgetItem* parentItem = topLevelItem(index);
                if(parentItem->data(0, IdRole).toString().compare(parentId) == 0) {
                    takeTopLevelItem(indexOfTopLevelItem(dragItem));
                    parentItem->addChild(dragItem);
                    return;
                }
            }
        }
    }
    else if(dragGroup) {
        dragItem->setExpanded(expanded);
    } else if (dragFile) {
        dragItem = itemAt(event->pos());
        foreach(const QUrl &url, event->mimeData()->urls())
        {
            QString filename = url.toLocalFile();
            if (filename.isEmpty())
                continue;

            fileNames.append (filename);
        }
    }

    if(dragItem) {
        if (dragFile)
            emit fileDragDropped (dragItem, fileNames);
        else
            emit itemDragDropped(dragItem);
    }
}

void lmcUserTreeWidget::contextMenuEvent(QContextMenuEvent* event) {
    QTreeWidget::contextMenuEvent(event);

    QTreeWidgetItem* item = itemAt(event->pos());
    QPoint pos = event->globalPos();
    if(item && event->reason() != QContextMenuEvent::Mouse) {
        QRect itemRect = visualItemRect(item);
        pos = itemRect.bottomLeft();
        pos.ry() += itemRect.height();
        pos = mapToGlobal(pos);
    }

    emit itemContextMenu(item, pos);
}

void lmcUserTreeWidget::mouseReleaseEvent(QMouseEvent* event) {
    QTreeWidget::mouseReleaseEvent(event);

    QPoint pos = event->pos();
    lmcUserTreeWidgetItem* item = static_cast<lmcUserTreeWidgetItem*>(itemAt(pos));
    if(item && checkable() && (item->data(0, TypeRole).toString().compare("Group") != 0 || item->checkBoxRect(visualItemRect(item)).contains(pos))) {
        if(item->checkState(0) == Qt::Checked)
            item->setCheckState(0, Qt::Unchecked);
        else
            item->setCheckState(0, Qt::Checked);
    }
    dragUser = false;
    dragGroup = false;
    dragFile = false;
    dragItem = nullptr;
}

void lmcUserTreeWidget::handleItemChanged(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);

    if(item->data(0, TypeRole).toString().compare("Group") == 0) {
        for(int index = 0; index < item->childCount(); index++) {
            bool oldState = blockSignals(true);

            item->child(index)->setCheckState(0, item->checkState(0));
            item->checkState(0) ? ++_selectedCount : --_selectedCount;

            blockSignals(oldState);
        }
    } else if(item->data(0, TypeRole).toString().compare("User") == 0) {
        int nChecked = 0;
        QTreeWidgetItem* parent = item->parent();
        for(int index = 0; index < parent->childCount(); index++)
            if(parent->child(index)->checkState(0))
                nChecked++;
        Qt::CheckState check = (nChecked == parent->childCount()) ? Qt::Checked : Qt::Unchecked;
        bool oldState = blockSignals(true);

        parent->setCheckState(0, check);
        item->checkState(0) ? ++_selectedCount : --_selectedCount;

        blockSignals(oldState);
    }

    emit selectedItemsChanged(_selectedCount);
}

void lmcUserTreeWidget::keyPressEvent(QKeyEvent* event) {
    QTreeWidget::keyPressEvent(event);

    if (_hoveredItem && _tooltips.contains(_hoveredItem)){
        _tooltips[_hoveredItem].data()->hide();
      /*
        QLabel *label = _tooltips[_hoveredItem].data()->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label && label->movie ())
            label->movie ()->stop();
      */
    }

    if(event->key() == Qt::Key_Space && selectedItems().count() > 0) {
        lmcUserTreeWidgetItem* item = static_cast<lmcUserTreeWidgetItem*>(selectedItems().at(0));
        if(item && checkable()) {
            if(item->checkState(0) == Qt::Checked)
                item->setCheckState(0, Qt::Unchecked);
            else
                item->setCheckState(0, Qt::Checked);
        }
    }
}

void lmcUserTreeWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    if(!dragGroup && !dragUser && ev->mimeData()->hasUrls())
    {
        QString urls;
        for (QUrl url : ev->mimeData()->urls())
            urls.append(url.toString());

        dragFile = true;
        ev->acceptProposedAction();
    } else if (dragGroup || dragUser)
        ev->acceptProposedAction();
}

void lmcUserTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (dragGroup || dragFile || dragUser)
        return;

    lmcUserTreeWidgetItem *item = static_cast<lmcUserTreeWidgetItem *> (itemAt(event->pos()));

    if (item && _tooltips.contains(item)) {
        if (!_hoveredItem) {
            // TODO !!! show the tooltip only after the timeout expires (150ms ? 250ms ?)
        }

        QPoint pos = event->globalPos() + QPoint(12, 15);
        QWidget *tooltip = _tooltips[item].data();

        const QRect screen = QApplication::desktop()->availableGeometry(this);
        if (pos.x() + tooltip->width() > screen.right())
            pos.setX(screen.right() - tooltip->width() + 1);
        if (pos.y() + tooltip->height() > screen.bottom())
            pos.setY(event->globalPos().y() - tooltip->height());

        if (_hoveredItem && _hoveredItem != item && _tooltips.contains(_hoveredItem)) {
            _tooltips[_hoveredItem].data()->hide();
        }

        _hoveredItem = item;
        tooltip->move(pos);
        tooltip->show();

      /*
        QLabel *label = tooltip->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label && label->movie ())
            label->movie ()->start ();
      */
    } else if (_hoveredItem && _tooltips.contains(_hoveredItem)){
        _tooltips[_hoveredItem].data()->hide();
      /*
        QLabel *label = _tooltips[_hoveredItem].data()->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label && label->movie ())
            label->movie ()->stop();
      */

        _hoveredItem = nullptr;
    }

    QTreeWidget::mouseMoveEvent(event);
}

void lmcUserTreeWidget::leaveEvent(QEvent  *event)
{if (_hoveredItem && _tooltips.contains(_hoveredItem)){
        _tooltips[_hoveredItem].data()->hide();
      /*
        QLabel *label = _tooltips[_hoveredItem].data()->findChild<QLabel *>(QStringLiteral("labelAvatar"));
        if (label && label->movie ())
            label->movie ()->stop();
      */
    }

    QTreeWidget::leaveEvent(event);
}
