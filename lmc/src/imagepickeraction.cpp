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

#include "imagepickeraction.h"
#include <QMenu>

lmcImagePickerAction::lmcImagePickerAction(QObject *parent,
                                           std::vector<ImagesStruct> source,
                                           QStringList tabs, int picWidth, int picHeight,
                                           int columns, int *selected, bool showTooltips)
    : QWidgetAction(parent), _picWidth(picWidth), _picHeight(picHeight), _columns(columns),
      _selected(selected), _showTooltips(showTooltips) {
  this->_source.swap(source);
  this->_tabs.swap(tabs);
}

lmcImagePickerAction::~lmcImagePickerAction() {}

void lmcImagePickerAction::releaseWidget(QWidget *widget) {
  widget->deleteLater();
}

QWidget *lmcImagePickerAction::createWidget(QWidget *parent) {
  QWidget *widget = nullptr;

  if (_tabs.size() <= 1) {
    std::vector<unsigned> sourceIndexes;
    std::vector<ImagesStruct *> sourceGrouped;

    for (unsigned index = 0; index < _source.size(); ++index) {
        sourceIndexes.push_back(index);
        sourceGrouped.push_back(&_source[index]);
    }

    lmcImagePicker *imagePicker = new lmcImagePicker(
        parent, sourceGrouped, sourceIndexes, _picWidth, _picHeight, _columns, _selected, _showTooltips);
    connect(imagePicker, &lmcImagePicker::triggered, this,
            &lmcImagePickerAction::imageSelected_slot);

    widget = imagePicker;
  } else {
      QTabWidget *tabWidget = new QTabWidget(parent);
      std::vector<ImagesStruct *> sourceGrouped;
      std::vector<unsigned> sourceIndexes;
      for (const QString &group : _tabs) {
          sourceGrouped.clear();
          sourceIndexes.clear();
          for (unsigned index = 0; index < _source.size(); ++index) {
              if (!_source[index].group.compare(group)) {
                  sourceGrouped.push_back(&_source[index]);
                  sourceIndexes.push_back(index);
              }
          }

          lmcImagePicker *widgetPage =
                  new lmcImagePicker(parent, sourceGrouped, sourceIndexes, _picWidth, _picHeight,
                                     _columns, _selected, _showTooltips, !group.compare(QStringLiteral("Tuzki bunny")));

          connect(widgetPage, &lmcImagePicker::triggered, this,
                  &lmcImagePickerAction::imageSelected_slot);

          tabWidget->addTab(widgetPage, group);

          widget = tabWidget;
      }
  }

  return widget;
}

void lmcImagePickerAction::imageSelected_slot() {
    if (*_selected >= 0) {
        emit imageSelected();
        QMenu *menu = (QMenu *)parent();
        menu->hide();
    }
}
