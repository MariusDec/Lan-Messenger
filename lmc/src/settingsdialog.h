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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtWidgets/QDialog>
#include <QListWidget>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QDesktopServices>
#include <QSound>
#include "ui_settingsdialog.h"
#include "shared.h"
#include "settings.h"
#include "history.h"
#include "stdlocation.h"
#include "application.h"
#include "messagelog.h"
#include "soundplayer.h"

class lmcSettingsDialog : public QDialog {
  Q_OBJECT

public:
  lmcSettingsDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~lmcSettingsDialog();

  void init();
  void settingsChanged();

signals:
  void historyCleared();
  void fileHistoryCleared();

protected:
  void changeEvent(QEvent* pEvent);

private slots:
  void listWidgetCategories_currentRowChanged(int currentRow);
  void buttonOk_clicked();
  void buttonCancel_clicked();
  void checkBoxMessageTime_toggled(bool checked);
  void checkBoxAllowLinks_toggled(bool checked);
  void checkBoxSysTrayMsg_toggled(bool checked);
  void radioButtonSysHistoryPath_toggled(bool checked);
  void buttonHistoryPath_clicked();
  void buttonFilePath_clicked();
  void buttonClearHistory_clicked();
  void buttonClearFileHistory_clicked();
  void checkBoxSound_toggled(bool checked);
  void checkBoxAutoShowFile_toggled(bool checked);
  void buttonViewFiles_clicked();
  void buttonFont_clicked();
  void buttonColor_clicked();
  void buttonReset_clicked();
  void comboBoxApplicationTheme_currentIndexChanged(int index);
  void comboBoxIconTheme_currentIndexChanged(int index);
  void comboBoxButtonTheme_currentIndexChanged(int index);
  void comboBoxChatTheme_currentIndexChanged(int index);
  void listWidgetBroadcasts_currentRowChanged(int index);
  void textBoxBroadcast_textEdited(const QString& text);
  void buttonAddBroadcast_clicked();
  void buttonDeleteBroadcast_clicked();
  void listWidgetSounds_currentRowChanged(int index);
  void buttonPlaySound_clicked();
  void buttonSoundPath_clicked();
  void buttonResetSounds_clicked();

private:
  void setPageHeaderStyle(QLabel* pLabel);
  void setUIText();
  void loadSettings();
  void saveSettings();
  QString getFontStyle(const QFont &font);

  Ui::SettingsDialog ui;
  lmcSettings* pSettings = nullptr;
  int fontSize;
  QFont font;
  QColor color;
  QIntValidator* pPortValidator;
  QRegExp	ipRegExp;
  QRegExpValidator* pIpValidator;
  lmcMessageLog* pMessageLog;
};

#endif // SETTINGSDIALOG_H
