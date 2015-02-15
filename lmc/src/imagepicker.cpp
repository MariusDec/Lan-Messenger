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

#include "uidefinitions.h"
#include "imagepicker.h"
#include "loggermanager.h"
#include "qxttooltip.h"
#include "thememanager.h"
#include "chathelper.h"

#include <QMenu>
#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <qmath.h>
#include <QLabel>
#include <QMovie>
#include <QToolTip>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QBitmap>

lmcImagePicker::lmcImagePicker(QWidget *parent,
                               std::vector<ImagesStruct *> source,
                               std::vector<unsigned> sourceIndexes, int picWidth, int picHeight,
                               int columns, int *selected, bool showTooltips, bool smoothScaleImages)
    : QTableWidget(parent), _selected(selected),
      _columns(columns) {
  LoggerManager::getInstance().writeInfo(
      QString("lmcImagePicker started-|- source size: %1").arg(source.size()));
  setMouseTracking(true);

  if (!_selected)
      _selected = new int;

  _hoveredCell.setX(-1);
  _hoveredCell.setY(-1);

  setBackgroundRole(QPalette::Window);
  setIconSize(QSize(picWidth, picWidth));
  setFrameShape(QFrame::NoFrame);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setSelectionMode(QAbstractItemView::NoSelection);
  setShowGrid(false);
  horizontalHeader()->setVisible(false);
  verticalHeader()->setVisible(false);
  setStyleSheet("QTableWidget { padding: 4px }"); // padding around table

  if (_columns <= 0)
      _columns = 4;

  int max_row = qCeil(source.size() / (qreal)_columns);

  setColumnCount(_columns);
  setRowCount(max_row);

  int cellWidth = picWidth + 8;
  int cellHeight = picHeight + 8;
  verticalHeader()->setDefaultSectionSize(cellHeight);
  verticalHeader()->setMinimumSectionSize(cellHeight);
  horizontalHeader()->setDefaultSectionSize(cellWidth);
  horizontalHeader()->setMinimumSectionSize(cellWidth);

  //	set min and max size of table, with padding included
  setMinimumSize(_columns * cellWidth + 8, max_row * cellHeight + 8);
  setMaximumSize(_columns * cellWidth + 8, max_row * cellHeight + 8);

  int currImageHeight;
  int currImageWidth;
  for (int row = 0; row < max_row; row++) {
    for (int column = 0; column < _columns; column++) {
      unsigned index = (row * _columns) + column;

      QLabelSubclass *item = new QLabelSubclass();

      if (index < source.size()) {
        item->setData(sourceIndexes[index]);
        if (!source[index]->icon.isEmpty()) {
            QPixmap px(source[index]->icon);

            currImageWidth = px.width() > picWidth ? picWidth : px.width(); // TODO !!! Reseach what happens if trying to get width/height or scale a null pixmap
            currImageHeight = px.height() > picHeight ? picHeight : px.height();

            item->setPixmap(px.scaled(QSize(currImageWidth, currImageHeight), Qt::KeepAspectRatio,
                                      smoothScaleImages ? Qt::SmoothTransformation : Qt::FastTransformation));
        } else {
            item->setText (source[index]->code);
            item->setFrameStyle (QLabel::StyledPanel | QLabel::Raised);
        }

        if (showTooltips)
          addLabelTooltip(item, source[index]);
      } else
        item->setData(-1);
      setCellWidget(row, column, item);
    }
  }

  connect(this, &lmcImagePicker::cellClicked, this,
          &lmcImagePicker::cellSelected);

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcImagePicker ended"));
}

lmcImagePicker::~lmcImagePicker() {
    for (int index = 0; index < rowCount(); ++index)
        for (int index2 = 0; index2 < columnCount(); ++index2)
            QxtToolTip::removeToolTip(cellWidget(index2, index));
}

void lmcImagePicker::cellSelected(int row, int column) {
  LoggerManager::getInstance().writeInfo(QStringLiteral("ImagePicker.cellSelected started"));

  QLabelSubclass *label = (QLabelSubclass *)cellWidget(row, column);
  if (label->getData().toInt() >= 0) {
    LoggerManager::getInstance().writeInfo(
        QString("ImagePicker.cellSelected-|- selected: %1")
            .arg(label->getData().toString()));

    *_selected = label->getData().toInt();
    emit triggered();
  }
  _hoveredCell.setX(-1);
  _hoveredCell.setY(-1);
  setCurrentIndex(QModelIndex());

  LoggerManager::getInstance().writeInfo(QStringLiteral("ImagePicker.cellSelected ended"));
}

void lmcImagePicker::addLabelTooltip(QLabelSubclass *item,
                                     ImagesStruct *image) {
  QWidget *toolTipWidget = new QWidget(this);
  toolTipWidget->setProperty("isWindow", true);

  toolTipWidget->setObjectName (QStringLiteral("toolTipWidget"));
  QVBoxLayout *layout = new QVBoxLayout(toolTipWidget);
  layout->setObjectName (QStringLiteral("layout"));
  toolTipWidget->setStyleSheet("border: 1px outset gray; border-radius: 4px;");

  if (!image->description.isEmpty()) {
    QLabel *labelDescription = new QLabel(toolTipWidget);
    labelDescription->setObjectName (QStringLiteral("labelDescription"));
    labelDescription->setText(QString("<b>%1</b>").arg(ChatHelper::makeHtmlSafe(image->description)));
    layout->addWidget(labelDescription, 0, Qt::AlignHCenter);
    labelDescription->setStyleSheet("border: none;");
  }

  if (!image->icon.isEmpty ()) {
      QLabel *labelImage = new QLabel(toolTipWidget);
      labelImage->setObjectName (QStringLiteral("labelImage"));
      labelImage->setMovie(new QMovie(image->icon));
      layout->addWidget(labelImage, 0, Qt::AlignHCenter);
      labelImage->setStyleSheet("border: none;");
  }

  if (!image->code.isEmpty()) {
    QLabel *labelCode = new QLabel(toolTipWidget);
    labelCode->setObjectName (QStringLiteral("labelCode"));
    labelCode->setText(image->code);
    layout->addWidget(labelCode, 0, Qt::AlignHCenter);
    labelCode->setStyleSheet("border: none;");
  }
  QxtToolTip::setToolTip(item, toolTipWidget);
}

void QLabelSubclass::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent (e);

    if (_hovered && _data.toInt () >= 0) {
        QPainter painter(this);

        QPen pen(QColor(125, 125, 152));
        pen.setWidth (3);
        painter.setPen(pen);

        painter.drawRect (rect ());
    }
}
