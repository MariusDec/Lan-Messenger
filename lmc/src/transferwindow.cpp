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

#include "transferwindow.h"
#include "thememanager.h"

#include <QDesktopServices>
#include <QtWidgets/QDesktopWidget>
#include <QUrl>

lmcTransferWindow::lmcTransferWindow(QWidget *parent) : QWidget(parent) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.lmcTransferWindow started"));

  ui.setupUi(this);
  setProperty("isWindow", true);

  QRect scr = QApplication::desktop()->screenGeometry();
  move(scr.center() - rect().center());

  connect(ui.listWidgetTransferList, &lmcTransferListView::currentRowChanged, this,
          &lmcTransferWindow::listWidgetTransferList_currentRowChanged);
  connect(ui.listWidgetTransferList, &lmcTransferListView::activated, this,
          &lmcTransferWindow::listWidgetTransferList_activated);
  connect(ui.buttonClear, &ThemedButton::clicked, this, &lmcTransferWindow::buttonClear_clicked);
  connect(ui.buttonClose, &ThemedButton::clicked, this, &lmcTransferWindow::buttonClose_clicked);
  connect(ui.comboBoxSelectedUser, &ThemedComboBox::currentTextChanged, this, &lmcTransferWindow::comboBoxSelectedUser_textChanged);

  ui.listWidgetTransferList->installEventFilter(this);
  ui.buttonClear->installEventFilter(this);
  ui.buttonClose->installEventFilter(this);

  pendingSendList.clear();

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.lmcTransferWindow ended"));
}

lmcTransferWindow::~lmcTransferWindow() {}

void lmcTransferWindow::init() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.init started"));

  setWindowIcon(QIcon(ThemeManager::getInstance ().getAppIcon (QStringLiteral("messenger"))));

  createToolBar();
  setButtonState(FileView::TS_Max);

  pSettings = new lmcSettings();
  restoreGeometry(pSettings->value(IDS_WINDOWTRANSFERS).toByteArray());
  setUIText();

  ui.listWidgetTransferList->loadData(StdLocation::transferHistoryFilePath());
  if (ui.listWidgetTransferList->count() > 0)
    ui.listWidgetTransferList->setCurrentRow(0);

  ui.comboBoxSelectedUser->addItem (tr("All users"), QString());
  QList<QString> users = getUniqueUsers ();
  for (QString user : users)
      ui.comboBoxSelectedUser->addItem (user);

  pSoundPlayer = new lmcSoundPlayer();

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.init ended"));
}

QList<QString> lmcTransferWindow::getUniqueUsers() {
    QList<QString> users;

    for(int index = 0; index < ui.listWidgetTransferList->count (); ++index) {
        FileView *file = ui.listWidgetTransferList->item (index);
        if (!users.contains (file->userName))
            users.append(file->userName);
    }
    return users;
}

void lmcTransferWindow::saveHistory(const FileView *transfer)
{
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.saveHistory started"));

    bool saveHistory =
        pSettings->value(IDS_FILEHISTORY, IDS_FILEHISTORY_VAL).toBool();
    if (saveHistory) {
      QString transferHistoryPath = StdLocation::transferHistoryFilePath();
      if (!transferHistoryPath.isEmpty())
        ui.listWidgetTransferList->saveData(transferHistoryPath, transfer);
    }

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.saveHistory ended"));
}

void lmcTransferWindow::updateList() { clearList(); }

void lmcTransferWindow::stop() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.stop started"));

  //	Cancel all active transfers
  for (int index = 0; index < ui.listWidgetTransferList->count(); index++) {
    FileView *view = ui.listWidgetTransferList->item(index);

    if (view->state < FileView::TS_Complete) {
      int mode = view->mode == FileView::TM_Send ? FM_Send : FM_Receive;
      XmlMessage xmlMessage;
      xmlMessage.addData(XN_MODE, FileModeNames[mode]);
      xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
      xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Cancel]);
      xmlMessage.addData(XN_FILEID, view->id);
      emit messageSent((MessageType)view->type, &view->userId, &xmlMessage);

      view->state = FileView::TS_Cancel;
      saveHistory(view);
    }
  }

  pSettings->setValue(IDS_WINDOWTRANSFERS, saveGeometry());

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.stop ended"));
}

void lmcTransferWindow::createTransfer(MessageType type, FileMode mode,
                                       QString *lpszUserId,
                                       QString *lpszUserName,
                                       XmlMessage *pMessage) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.createTransfer started"));

  FileView fileView(pMessage->data(XN_FILEID));
  fileView.fileSize = pMessage->data(XN_FILESIZE).toLongLong();
  fileView.sizeDisplay = Helper::formatSize(fileView.fileSize);
  fileView.userId = *lpszUserId;
  fileView.userName = *lpszUserName;
  fileView.fileName = pMessage->data(XN_FILENAME);
  fileView.filePath = pMessage->data(XN_FILEPATH);
  fileView.type = type;
  if (mode == FM_Send) {
    fileView.mode = FileView::TM_Send;
    fileView.state = FileView::TS_Send;
  } else {
    fileView.mode = FileView::TM_Receive;
    fileView.state = FileView::TS_Receive;
  }
  fileView.fileDisplay = fileView.fileName + " (" + fileView.sizeDisplay + ")";
  fileView.icon = getIcon(fileView.filePath);
  fileView.startTime = QDateTime::currentDateTime();
  ui.listWidgetTransferList->insertItem(0, &fileView);
  ui.listWidgetTransferList->setCurrentRow(0);

  if (ui.comboBoxSelectedUser->findText(fileView.userName, Qt::MatchFixedString | Qt::MatchCaseSensitive) < 0)
      ui.comboBoxSelectedUser->addItem (fileView.userName);

  QString currentFilter = ui.comboBoxSelectedUser->currentData ().toString ();
  if (!currentFilter.isEmpty () && currentFilter.compare (fileView.userId))
      ui.listWidgetTransferList->setRowHidden (ui.listWidgetTransferList->count (), true);

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.createTransfer ended"));
}

void lmcTransferWindow::receiveMessage(MessageType type, QString *lpszUserId,
                                       XmlMessage *pMessage) {
  Q_UNUSED(type);
  Q_UNUSED(lpszUserId);

  int fileMode =
      Helper::indexOf(FileModeNames, FM_Max, pMessage->data(XN_MODE));
  int fileOp = Helper::indexOf(FileOpNames, FO_Max, pMessage->data(XN_FILEOP));
  QString id = pMessage->data(XN_FILEID);

  FileView *view = NULL;
  FileView::TransferMode transferMode =
      fileMode == FM_Send ? FileView::TM_Send : FileView::TM_Receive;
  int itemIndex = -1;
  QString trayMsg;

  switch (fileOp) {
  case FO_Decline:
    //	receiver has declined
    view = ui.listWidgetTransferList->item(id, FileView::TM_Send);
    if (!view)
      return;
    itemIndex = ui.listWidgetTransferList->itemIndex(id, FileView::TM_Send);
    view->state = FileView::TS_Decline;
    break;
  case FO_Cancel:
    view = ui.listWidgetTransferList->item(id, transferMode);
    if (!view)
      return;
    itemIndex = ui.listWidgetTransferList->itemIndex(id, transferMode);
    view->state = FileView::TS_Cancel;
    break;
  case FO_Progress:
    view = ui.listWidgetTransferList->item(id, transferMode);
    if (!view)
      return;
    itemIndex = ui.listWidgetTransferList->itemIndex(id, transferMode);
    updateProgress(view, pMessage->data(XN_FILESIZE).toLongLong());
    break;
  case FO_Error:
    view = ui.listWidgetTransferList->item(id, transferMode);
    if (!view)
      return;
    itemIndex = ui.listWidgetTransferList->itemIndex(id, transferMode);
    view->state = FileView::TS_Abort;
    break;
  case FO_Abort:
    view = ui.listWidgetTransferList->item(id, transferMode);
    if (!view)
      return;
    itemIndex = ui.listWidgetTransferList->itemIndex(id, transferMode);
    view->state = FileView::TS_Abort;
    break;
  case FO_Complete:
    if (fileMode == FM_Send) {
      view = ui.listWidgetTransferList->item(id, FileView::TM_Send);
      if (!view)
        return;
      itemIndex = ui.listWidgetTransferList->itemIndex(id, FileView::TM_Send);
      view->state = FileView::TS_Complete;
      if (isHidden() || !isActiveWindow()) {
        trayMsg = tr("'%1' has been sent to %2.");
        emit showTrayMessage(TM_Transfer,
                             trayMsg.arg(view->fileName, view->userName), QString(),
                             tr("File Transfer Completed"), TMI_Info);
        pSoundPlayer->play(SE_FileDone);
      }
    } else {
      view = ui.listWidgetTransferList->item(id, FileView::TM_Receive);
      if (!view)
        return;
      itemIndex = ui.listWidgetTransferList->itemIndex(id, FileView::TM_Receive);
      view->filePath = QDir::fromNativeSeparators(pMessage->data(XN_FILEPATH));
      view->icon = getIcon(view->filePath);
      _buttonShowFolder->setEnabled(QFile::exists(view->filePath));
      view->state = FileView::TS_Complete;
      if (isHidden() || !isActiveWindow()) {
        trayMsg = tr("'%1' has been received from %2.");
        emit showTrayMessage(TM_Transfer,
                             trayMsg.arg(view->fileName, view->userName), QString(),
                             tr("File Transfer Completed"), TMI_Info);
        pSoundPlayer->play(SE_FileDone);
      }
    }
    saveHistory(view);
    break;
  }

  ui.listWidgetTransferList->itemChanged(itemIndex);

  FileView *current = ui.listWidgetTransferList->currentItem();
  setButtonState(current->state);
}

void lmcTransferWindow::settingsChanged() { pSoundPlayer->settingsChanged(); }

void lmcTransferWindow::setUserFilter(const QString &userName)
{
    int dataIndex;
    if ((dataIndex = ui.comboBoxSelectedUser->findText (userName, Qt::MatchFixedString | Qt::MatchCaseSensitive)) < 0)
        return;

    if (!userName.compare(QStringLiteral("All users"))) {
        for(int index = 0; index < ui.listWidgetTransferList->count (); ++index)
            ui.listWidgetTransferList->setRowHidden (index, false);
    } else {
        for(int index = 0; index < ui.listWidgetTransferList->count (); ++index) {
            FileView *file = ui.listWidgetTransferList->item (index);
            if (!userName.compare (file->userName))
                ui.listWidgetTransferList->setRowHidden (index, false);
            else
                ui.listWidgetTransferList->setRowHidden (index, true);
        }
    }

    ui.comboBoxSelectedUser->setCurrentText(userName);
}

bool lmcTransferWindow::eventFilter(QObject *pObject, QEvent *pEvent) {
  Q_UNUSED(pObject);
  if (pEvent->type() == QEvent::KeyPress) {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);
    if (pKeyEvent->key() == Qt::Key_Escape) {
      close();
      return true;
    }
  }

  return false;
}

void lmcTransferWindow::changeEvent(QEvent *pEvent) {
  switch (pEvent->type()) {
  case QEvent::LanguageChange:
    setUIText();
    break;
  default:
    break;
  }

  QWidget::changeEvent(pEvent);
}

void lmcTransferWindow::listWidgetTransferList_currentRowChanged(int currentRow) {
  if (currentRow < 0) {
    setButtonState(FileView::TS_Max);
    return;
  }
  // TODO check if row index changes when there are hidden rows
  FileView *pFileView = ui.listWidgetTransferList->item(currentRow);
  setButtonState(pFileView->state);
  _buttonShowFolder->setEnabled(QFile::exists(pFileView->filePath));
}

void lmcTransferWindow::listWidgetTransferList_activated(const QModelIndex &index) {
  FileView *view = ui.listWidgetTransferList->item(index.row());

  QDesktopServices::openUrl(QUrl::fromLocalFile(view->filePath));
}

void lmcTransferWindow::buttonCancel_clicked() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.buttonCancel_clicked started"));

  FileView *view = ui.listWidgetTransferList->currentItem();

  int mode = view->mode == FileView::TM_Send ? FM_Send : FM_Receive;
  XmlMessage xmlMessage;
  xmlMessage.addData(XN_MODE, FileModeNames[mode]);
  xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Normal]);
  xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Cancel]);
  xmlMessage.addData(XN_FILEID, view->id);
  emit messageSent((MessageType)view->type, &view->userId, &xmlMessage);

  view->state = FileView::TS_Cancel;

  ui.listWidgetTransferList->itemChanged(ui.listWidgetTransferList->currentRow());
  setButtonState(view->state);

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.buttonCancel_clicked ended"));
}

void lmcTransferWindow::buttonRemove_clicked() {
  FileView *view = ui.listWidgetTransferList->currentItem();

  if (view->state < FileView::TS_Complete)
    return;

  ui.listWidgetTransferList->removeItem(ui.listWidgetTransferList->currentRow());
}

void lmcTransferWindow::buttonClear_clicked() {
  QFile::remove(StdLocation::transferHistoryFilePath());
  clearList();
}

void lmcTransferWindow::buttonClose_clicked()
{
    close ();
}

void lmcTransferWindow::comboBoxSelectedUser_textChanged(const QString &text)
{
    setUserFilter (text);
}

void lmcTransferWindow::buttonShowFolder_clicked() {
  FileView *view = ui.listWidgetTransferList->currentItem();

  QString path = QFileInfo(view->filePath).dir().path();
  QUrl url;
  // hack to make sure qdesktopservices open a network path
  if (path.startsWith("\\\\") || path.startsWith("//"))
    url.setUrl(QDir::toNativeSeparators(path));
  else
    url = QUrl::fromLocalFile(path);

  QDesktopServices::openUrl(url);
}

QFrame *lmcTransferWindow::createSeparator(QWidget *parent) {
    QFrame *separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(2);
    separator->setFixedHeight(30);

    return separator;
}

void lmcTransferWindow::createToolBar() {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.createToolBar started"));

   ui.widgetToolBarContainer->setProperty("isToolbar", true);

  _buttonCancel = new ThemedButton(ui.widgetToolBar);
  _buttonCancel->setToolButtonStyle (Qt::ToolButtonIconOnly);
  _buttonCancel->setAutoRaise (true);
  _buttonCancel->setIconSize (QSize(24, 24));
  _buttonCancel->setFixedWidth(32);
  _buttonCancel->setIcon (QIcon(ThemeManager::getInstance ().getAppIcon (QStringLiteral("stop"))));

  connect(_buttonCancel, &ThemedButton::clicked, this, &lmcTransferWindow::buttonCancel_clicked);

  _buttonShowFolder = new ThemedButton(ui.widgetToolBar);
  _buttonShowFolder->setToolButtonStyle (Qt::ToolButtonIconOnly);
  _buttonShowFolder->setAutoRaise (true);
  _buttonShowFolder->setIconSize (QSize(24, 24));
  _buttonShowFolder->setFixedWidth(32);
  _buttonShowFolder->setIcon (QIcon(ThemeManager::getInstance ().getAppIcon (QStringLiteral("folder"))));

  connect(_buttonShowFolder, &ThemedButton::clicked, this, &lmcTransferWindow::buttonShowFolder_clicked);

  _buttonRemove = new ThemedButton(ui.widgetToolBar);
  _buttonRemove->setToolButtonStyle (Qt::ToolButtonIconOnly);
  _buttonRemove->setAutoRaise (true);
  _buttonRemove->setIconSize (QSize(24, 24));
  _buttonRemove->setFixedWidth(32);
  _buttonRemove->setIcon (QIcon(ThemeManager::getInstance ().getAppIcon (QStringLiteral("close"))));

  connect(_buttonRemove, &ThemedButton::clicked, this, &lmcTransferWindow::buttonRemove_clicked);

  ui.widgetToolbar_layout->addWidget(_buttonCancel);
  ui.widgetToolbar_layout->addWidget(createSeparator(ui.widgetToolBar));
  ui.widgetToolbar_layout->addWidget(_buttonShowFolder);
  ui.widgetToolbar_layout->addWidget(_buttonRemove);

  LoggerManager::getInstance().writeInfo(
      QStringLiteral("lmcTransferWindow.createToolBar ended"));
}

void lmcTransferWindow::setUIText() {
  ui.retranslateUi(this);

  setWindowTitle(tr("File Transfers"));

  _buttonCancel->setText (tr("Cancel"));
  _buttonCancel->setToolTip (tr("Cancel"));

  _buttonShowFolder->setText (tr("Show In Folder"));
  _buttonShowFolder->setToolTip (tr("Show In Folder"));

  _buttonRemove->setText (tr("Remove From List"));
  _buttonRemove->setToolTip (tr("Remove From List"));
}

void lmcTransferWindow::setButtonState(FileView::TransferState state) {
  switch (state) {
  case FileView::TS_Send:
  case FileView::TS_Receive:
    _buttonCancel->setEnabled(true);
    _buttonRemove->setEnabled(false);
    break;
  case FileView::TS_Decline:
  case FileView::TS_Complete:
  case FileView::TS_Cancel:
  case FileView::TS_Abort:
    _buttonCancel->setEnabled(false);
    _buttonRemove->setEnabled(true);
    break;
  default:
    _buttonCancel->setEnabled(false);
    _buttonShowFolder->setEnabled(false);
    _buttonRemove->setEnabled(false);
    break;
  }
}

QPixmap lmcTransferWindow::getIcon(QString filePath) {
  QFileIconProvider iconProvider;
  QFileInfo fileInfo(filePath);
  QPixmap icon;

  if (QFile::exists(filePath))
    icon = iconProvider.icon(fileInfo).pixmap(32, 32);
  else {
    QString fileName = fileInfo.fileName();
    QString path = QDir::temp().absoluteFilePath(fileName);
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.close();
    icon = iconProvider.icon(QFileInfo(path)).pixmap(32, 32);
    QFile::remove(path);
  }

  return icon;
}

QString lmcTransferWindow::formatTime(qint64 size, qint64 speed) {
  int d = 86400;
  int h = 3600;
  int m = 60;

  if (speed == 0)
    return tr("Calculating time");

  int time = size / speed;
  QString s;
  if (time > d) {
    s.append(QString("%1d ").arg(time / d));
    time %= d;
  }
  if (time > h) {
    s.append(QString("%1h ").arg(time / h));
    time %= h;
  }
  if (time > m) {
    s.append(QString("%1m ").arg(time / m));
    time %= m;
  }
  if (time >= 0)
    s.append(QString("%1s ").arg(time));

  return s.trimmed();
}

void lmcTransferWindow::clearList() {
  for (int index = 0; index < ui.listWidgetTransferList->count(); index++) {
    FileView *view = ui.listWidgetTransferList->item(index);
    if (view->state < FileView::TS_Complete)
      continue;

    ui.listWidgetTransferList->removeItem(index);
    index--;
  }
  ui.comboBoxSelectedUser->clear ();
}

void lmcTransferWindow::updateProgress(FileView *view, qint64 currentPos) {
  view->position = currentPos;
  view->posDisplay = Helper::formatSize(view->position);
  qint64 timeSpan =
      view->startTime.msecsTo(QDateTime::currentDateTime()) / 1000;
  qint64 speed = (timeSpan > 0) ? view->position / timeSpan : 0;
  view->speed = (speed + view->speed) / 2;
  view->speedDisplay = Helper::formatSize(view->speed) + tr("/sec");
  view->timeDisplay = formatTime(view->fileSize - view->position, view->speed);
}
