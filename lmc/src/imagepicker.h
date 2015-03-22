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

#ifndef IMAGEPICKER_H
#define IMAGEPICKER_H

#include <QTableWidget>
#include <imageslist.h>
#include <QLabel>
#include "qxttooltip.h"

// Subclassed to display animated tooltips
class QLabelSubclass : public QLabel {
    QWidget *_tooltipWidget = nullptr;
    QVariant _data;
    bool _hovered = false;

public:
    QLabelSubclass(QWidget *parent = 0, Qt::WindowFlags f = 0) : QLabel(parent, f) {
        setMouseTracking(true);
        setAlignment(Qt::AlignCenter);
    }
    QLabelSubclass(const QString &text, QWidget *parent = 0, Qt::WindowFlags f = 0) : QLabel(text, parent, f) {}

    void setData(QVariant data) { _data = data; }
    const QVariant &getData() { return _data; }

protected:
    void enterEvent (QEvent *) { _hovered = true; update(); }
    void leaveEvent (QEvent *) { _hovered = false; update(); }
    void paintEvent (QPaintEvent *);
};

class lmcImagePicker : public QTableWidget {
  Q_OBJECT

public:
  lmcImagePicker(QWidget *parent, std::vector<ImagesStruct *> source, std::vector<unsigned> sourceIndexes,
                 int picWidth, int picHeight, int columns, int *_selected,
                 bool showTooltips = false, bool smoothScaleImages = false);
  ~lmcImagePicker();

protected:
  void cellSelected(int row, int column);

  void addLabelTooltip(QLabelSubclass *item, ImagesStruct *image);

private:
  int *_selected;
  int _columns;

  QPoint _hoveredCell;

signals:
  void triggered();
};

#endif // IMAGEPICKER_H
