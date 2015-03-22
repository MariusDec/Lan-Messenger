#include "instantmessagewindow.h"
#include "ui_instantmessagewindow.h"
#include "messagelog.h"
#include "stdlocation.h"
#include "globals.h"
#include "thememanager.h"

#include <QDesktopWidget>
#include <QFontDialog>
#include <QColorDialog>
#include <QKeyEvent>

InstantMessageWindow::InstantMessageWindow(User *localUser, QWidget *parent) :  QDialog(parent), _localUser(localUser),
    ui(new Ui::InstantMessageWindow)
{
    ui->setupUi(this);
    setProperty("isWindow", true);
    ui->textBoxMessage->setProperty("light", true);
    ui->textBoxMessage->installEventFilter(this);
    ui->frameStatus->hide();
    ui->labelStatusIcon->setPixmap(QPixmap(ThemeManager::getInstance().getAppIcon("warning")));

    _messageLog = new lmcMessageLog (ui->widgetLog);
    ui->widgetLog_layout->addWidget(_messageLog);

    _messageLog->setAcceptDrops(false);
    _messageLog->localId = _localUser->id;
    _messageLog->initMessageLog();

    ui->labelCountChars->setVisible(Globals::getInstance().showCharacterCount());
    ui->labelSendKey->setVisible(Globals::getInstance().showCharacterCount());

    int bottomPanelHeight = ui->textBoxMessage->minimumHeight() +
                            ui->labelDividerBottom->minimumHeight() +
                            ui->labelDividerTop->minimumHeight() +
                            ui->widgetToolBar->minimumHeight();
    QList<int> sizes;
    sizes.append(height() - bottomPanelHeight - ui->hSplitter->handleWidth());
    sizes.append(bottomPanelHeight);
    ui->hSplitter->setSizes(sizes);
    ui->hSplitter->setStyleSheet(QString("QSplitter::handle { image: url(%1); }").arg(ThemeManager::getInstance().getAppIcon(QStringLiteral("vgrip"))));

    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);
}

InstantMessageWindow::~InstantMessageWindow()
{
    delete ui;

    _smileyMenu->clear();
    delete _smileyMenu;

    _emojiMenu->clear();
    delete _emojiMenu;
}

void InstantMessageWindow::init(const QString &peerId, const QString &peerName, MessageType messageType, const XmlMessage &message)
{
    setUIText();
    createSmileyMenu();
    createEmojiMenu();
    createToolbar();

    QFont font;
    font.fromString(Globals::getInstance().messagesFontString());
    _messageColor = QApplication::palette().text().color();
    _messageColor.setNamedColor(Globals::getInstance().messagesColor());

    setMessageFont(font);
    ui->textBoxMessage->setStyleSheet(QString("QTextEdit { color: %1; }").arg(_messageColor.name()));
    ui->textBoxMessage->setFocus();

    _peerId = peerId;
    _peerName = peerName;
    _messageType = messageType;

    if (message.dataExists(XN_MESSAGE))
    {
        if (!Globals::getInstance().instantMessageSplitterGeometry().isEmpty())
            ui->hSplitter->restoreGeometry(Globals::getInstance().instantMessageSplitterGeometry());
        _xmlMessage = message;
        _message = _xmlMessage.data(XN_MESSAGE);

        _messageLog->appendMessageLog(messageType, _peerId, _peerName, _xmlMessage, false, false, false);
    } else {
        ui->widgetMessageLog->hide();
        ui->buttonOpenInChat->hide();
    }

    if (!Globals::getInstance().instantMessageWindowGeometry().isEmpty())
        restoreGeometry(Globals::getInstance().instantMessageWindowGeometry());
    else
        move(50, 50);

    ui->hSplitter->restoreState(Globals::getInstance().chatHSplitterGeometry());

    XmlMessage xmlMessage(message.toString());
    SingleMessage messageItem(messageType, peerId, peerName, xmlMessage, QString::null);

    QString roomId = _localUser->id;
    roomId.append(Helper::getUuid());
    _historySavePath = QDir(StdLocation::getWritableCacheDir()).absoluteFilePath(QString("msg_%1.tmp").arg(roomId));

    lmcMessageLog::saveMessageLog(peerName, peerId, _localUser, QList<QString> () << peerName, QDateTime::currentDateTime(), messageItem, _historySavePath);

    connect (ui->buttonClose, &ThemedButton::clicked, this, &InstantMessageWindow::close);
    connect (ui->buttonOpenInChat, &ThemedButton::clicked, this, &InstantMessageWindow::openInChat);
    connect (ui->textBoxMessage, &QTextEdit::textChanged, this, &InstantMessageWindow::textChanged);

    setWindowTitle(getWindowTitle());
    ThemeManager::getInstance().reloadStyleSheet();
    adjustSize();
    show();
}

void InstantMessageWindow::settingsChanged()
{
    ui->labelSendKey->setText(QString("Send message using %1\t").arg(Globals::getInstance().sendByEnter() ? "Enter" : "Ctrl+Enter"));
}

void InstantMessageWindow::connectionChanged(bool connected)
{
    if (!connected) {
        ui->labelStatus->setText(QStringLiteral("You are offline"));
        ui->frameStatus->show();
    } else if (_peerIsOffline) {
        ui->labelStatus->setText(QString("%1 is offline").arg(_peerName));
        ui->frameStatus->show();
    } else
        ui->frameStatus->hide();

    ui->labelCountChars->setVisible(Globals::getInstance().showCharacterCount());
    ui->labelSendKey->setVisible(Globals::getInstance().showCharacterCount());
    textChanged();
}

void InstantMessageWindow::peerConnectionChanged(bool connected)
{
    if (!connected && ui->frameStatus->isHidden()) {
            ui->labelStatus->setText(QString("%1 is offline").arg(_peerName));
            ui->frameStatus->show();
    } else if (connected && Globals::getInstance().isConnected())
        ui->frameStatus->hide();
}

void InstantMessageWindow::setUIText()
{
    ui->retranslateUi(this);
    ui->labelSendKey->setText(QString("Send message using %1\t").arg(Globals::getInstance().sendByEnter() ? "Enter" : "Ctrl+Enter"));
}

bool InstantMessageWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (object == ui->textBoxMessage) {
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {
          bool sendByEnter = (keyEvent->modifiers() == Qt::NoModifier);
          if (sendByEnter == Globals::getInstance().sendByEnter()) {
            sendMessage();
            return true;
          }
          // The TextEdit widget does not insert new line when Ctrl+Enter is pressed
          // So we insert a new line manually
          if (!sendByEnter)
            ui->textBoxMessage->insertPlainText("\n");
        } else if (keyEvent->key() == Qt::Key_Escape) {
          close();
          return true;
        } else if (keyEvent->key() == Qt::Key_B && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
            insertHtmlTag(QStringLiteral("[b]"), QStringLiteral("[/b]"));
            return true;
        } else if (keyEvent->key() == Qt::Key_U && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
            insertHtmlTag(QStringLiteral("[u]"), QStringLiteral("[/u]"));
            return true;
        } else if (keyEvent->key() == Qt::Key_I && (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
            insertHtmlTag(QStringLiteral("[i]"), QStringLiteral("[/i]"));
            return true;
        }
      } else {
        if (keyEvent->key() == Qt::Key_Escape) {
          close();
          return true;
        }
      }
    }

    return false;
}

void InstantMessageWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ActivationChange:
      if (isActiveWindow()) {
        ui->textBoxMessage->setFocus();
      }
      break;
    case QEvent::LanguageChange:
      setUIText();
      break;
    default:
      break;
    }

    QWidget::changeEvent(event);
}

void InstantMessageWindow::moveEvent(QMoveEvent *event)
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

void InstantMessageWindow::closeEvent(QCloseEvent *event)
{
    Globals::getInstance().setInstantMessageWindowGeometry(saveGeometry());
    if (_xmlMessage.dataExists(XN_MESSAGE))
        Globals::getInstance().setInstantMessageSplitterGeometry(ui->hSplitter->saveGeometry());

    emit closed();
    QDialog::closeEvent(event);
}

void InstantMessageWindow::createSmileyMenu() {
  _smileyMenu = new QMenu(this);

  _smileyAction = new lmcImagePickerAction(
      _smileyMenu, ImagesList::getInstance().getSmileys(),
      ImagesList::getInstance().getTabs(ImagesList::Smileys), 39, 39, 10,
      &_nSmiley, true);
  connect(_smileyAction, &lmcImagePickerAction::imageSelected, this,
          &InstantMessageWindow::smileyAction_triggered);

  _smileyMenu->addAction(_smileyAction);
}

void InstantMessageWindow::createEmojiMenu() {
  _emojiMenu = new QMenu(this);

  _emojiAction = new lmcImagePickerAction(
      _emojiMenu, ImagesList::getInstance().getEmojis(),
      ImagesList::getInstance().getTabs(ImagesList::Emojis), 78, 39, 8, &_nEmoji,
      true);
  connect(_emojiAction, &lmcImagePickerAction::imageSelected, this,
          &InstantMessageWindow::emojiAction_triggered);

  _emojiMenu->addAction(_emojiAction);
}

QFrame *InstantMessageWindow::createSeparator(QWidget *parent) {
    QFrame *separator = new QFrame(parent);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(2);
    separator->setFixedHeight(30);

    return separator;
}

void InstantMessageWindow::createToolbar()
{
    ui->widgetToolBar->setProperty("isToolbar", true);

   ThemedButton *buttonFont = new ThemedButton(ui->widgetToolBar);
   buttonFont->setToolButtonStyle(Qt::ToolButtonIconOnly);
   buttonFont->setAutoRaise(true);
   buttonFont->setIconSize(QSize(17, 17));
   buttonFont->setFixedWidth(32);
   buttonFont->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                          QStringLiteral("font"))));

   connect (buttonFont, &ThemedButton::clicked, this, &InstantMessageWindow::buttonFont_clicked);

   ThemedButton *buttonFontColor = new ThemedButton(ui->widgetToolBar);
   buttonFontColor->setToolButtonStyle(Qt::ToolButtonIconOnly);
   buttonFontColor->setAutoRaise(true);
   buttonFontColor->setIconSize(QSize(17, 17));
   buttonFontColor->setFixedWidth(32);
   buttonFontColor->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                          QStringLiteral("fontcolor"))));

   connect (buttonFontColor, &ThemedButton::clicked, this, &InstantMessageWindow::buttonFontColor_clicked);

   ThemedButton *buttonSmiley = new ThemedButton(ui->widgetToolBar);
   buttonSmiley->setPopupMode(QToolButton::InstantPopup);
   buttonSmiley->setToolButtonStyle(Qt::ToolButtonIconOnly);
   buttonSmiley->setAutoRaise(true);
   buttonSmiley->setIconSize(QSize(17, 17));
   buttonSmiley->setFixedWidth(32);
   buttonSmiley->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                          QStringLiteral("smiley"))));
   buttonSmiley->setMenu(_smileyMenu);

   ThemedButton *buttonEmoji = new ThemedButton(ui->widgetToolBar);
   buttonEmoji->setPopupMode(QToolButton::InstantPopup);
   buttonEmoji->setToolButtonStyle(Qt::ToolButtonIconOnly);
   buttonEmoji->setAutoRaise(true);
   buttonEmoji->setIconSize(QSize(17, 17));
   buttonEmoji->setFixedWidth(32);
   buttonEmoji->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(
                                          QStringLiteral("emoji"))));
   buttonEmoji->setMenu(_emojiMenu);

   ui->toolBarLayout->addWidget(buttonFont);
   ui->toolBarLayout->addWidget(buttonFontColor);
   ui->toolBarLayout->addWidget(createSeparator(ui->widgetToolBar));
   ui->toolBarLayout->addWidget(buttonSmiley);
   ui->toolBarLayout->addWidget(buttonEmoji);
   ui->toolBarLayout->addStretch();

   buttonFont->setText(tr("Change Font..."));
   buttonFont->setToolTip(tr("Change message font"));
   buttonFontColor->setText(tr("Change Color..."));
   buttonFontColor->setToolTip(tr("Change message text color"));
   buttonSmiley->setText(tr("Insert Smiley"));
   buttonSmiley->setToolTip(tr("Insert a smiley into the message"));
   buttonEmoji->setText(tr("Insert Emoji"));
   buttonEmoji->setToolTip(tr("Insert an emoji into the message"));


   ui->labelDividerTop->setBackgroundRole(QPalette::Light);
   ui->labelDividerTop->setAutoFillBackground(true);
   ui->labelDividerBottom->setBackgroundRole(QPalette::Dark);
   ui->labelDividerBottom->setAutoFillBackground(true);
}

void InstantMessageWindow::buttonFont_clicked() {
  bool ok;
  QFont font = ui->textBoxMessage->font();
  font.setPointSize(ui->textBoxMessage->fontPointSize());
  QFont newFont = QFontDialog::getFont(&ok, font, this, tr("Select Font"));
  if (ok)
    setMessageFont(newFont);
}

void InstantMessageWindow::buttonFontColor_clicked() {
  QColor color = QColorDialog::getColor(_messageColor, this, tr("Select Color"));
  if (color.isValid()) {
    _messageColor = color;
    ui->textBoxMessage->setStyleSheet("QTextEdit {color: " +
                                     _messageColor.name() + ";}");
  }
}

void InstantMessageWindow::setMessageFont(QFont &font) {
  ui->textBoxMessage->setFont(font);
  ui->textBoxMessage->setFontPointSize(font.pointSize());
}

void InstantMessageWindow::smileyAction_triggered() {
  //	nSmiley contains index of smiley
    if (_nSmiley >= 0)
        insertSmileyCode(ImagesList::getInstance().getSmileys()[_nSmiley]);
}

void InstantMessageWindow::emojiAction_triggered() {
    if (_nEmoji >= 0)
        insertSmileyCode(ImagesList::getInstance().getEmojis()[_nEmoji]);
}

void InstantMessageWindow::insertSmileyCode(const ImagesStruct &smiley) {
    QString smileyCode = "%1";

    QString text = ui->textBoxMessage->toPlainText();
    QTextCursor cursor = ui->textBoxMessage->textCursor();
    if (!text.isEmpty()) {
        int cursorPos = cursor.anchor();
        if (cursorPos != 0 && text.at(cursorPos - 1) != ' ')
            smileyCode.prepend(' ');
    }

    bool moveCursor = true;
    if (text.length() <= cursor.position() || text.at(cursor.position()) != ' ') {
        moveCursor = false;
        smileyCode.append(' ');
    }

    ui->textBoxMessage->insertPlainText(smileyCode.arg(smiley.code));

    if (moveCursor) {
        cursor.movePosition(QTextCursor::Right); // Move cursor to the right by 1 unit
        ui->textBoxMessage->setTextCursor(cursor);
    }
    ui->textBoxMessage->setFocus();
}

void InstantMessageWindow::insertHtmlTag(const QString &beginTag, const QString &endTag) {
    QTextCursor cursor = ui->textBoxMessage->textCursor();
    QString selectedText = cursor.selectedText();

    bool moveCursor = selectedText.isEmpty();

    selectedText = QString("%1%2%3").arg(beginTag, selectedText, endTag);

    ui->textBoxMessage->insertPlainText(selectedText);

    if (moveCursor) {
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, endTag.length());
        ui->textBoxMessage->setTextCursor(cursor);
    }
    ui->textBoxMessage->setFocus();
}

void InstantMessageWindow::sendMessage()
{
    if (ui->textBoxMessage->document()->isEmpty())
      return;

    if (Globals::getInstance().isConnected() && !_peerIsOffline) {
      QString message(ui->textBoxMessage->toPlainText());

      if (!_message.isEmpty())
          message.append(QString("[quote][b] %1:[/b]\n%2").arg(_peerName, _message));

      QFont font = ui->textBoxMessage->font();
      font.setPointSize(ui->textBoxMessage->fontPointSize());

      XmlMessage xmlMessage;
      xmlMessage.addHeader(XN_TIME, QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
      xmlMessage.addData(XN_MESSAGE, message);
      xmlMessage.addData(XN_STATUS, _localUser->status);
      xmlMessage.addData(XN_NAME, _localUser->name);
      xmlMessage.addData(XN_FONT, font.toString());
      xmlMessage.addData(XN_COLOR, _messageColor.name());

      SingleMessage messageItem(_messageType, _localUser->id, _localUser->name, xmlMessage, QString::null);
      lmcMessageLog::saveMessageLog(_localUser->name, _localUser->id, _localUser, QList<QString> () << _localUser->name, QDateTime::currentDateTime(), messageItem, _historySavePath);

      emit messageSent(_messageType, _peerId, xmlMessage);
      close();
    } else
    //  appendMessageLog(MT_Error, NULL, NULL, NULL);

    // close if no MT_Failed is sent from messageLog

    ui->textBoxMessage->setFocus();
}

QString InstantMessageWindow::getWindowTitle()
{
    if (_messageType == MT_Broadcast)
        return QString("%1").arg(tr("%1 sent you a broadcast")).arg(_peerName);
    else {
        if (_xmlMessage.dataExists(XN_MESSAGE))
            return QString("%1").arg(tr("%1 sent you a message")).arg(_peerName);
        else
            return QString("%1").arg(tr("Send message to %1")).arg(_peerName);
    }
}

void InstantMessageWindow::openInChat()
{
    QString chatRoomId = _localUser->id;
    chatRoomId.append(Helper::getUuid());

    emit chatStarting(chatRoomId, _xmlMessage, QStringList() << _peerId);
    close();
}

void InstantMessageWindow::textChanged() {
    if (Globals::getInstance().showCharacterCount()) {
        QString text = ui->textBoxMessage->toPlainText();
        text = text.remove(QRegularExpression("(\\[/?(b|u|i|q|quote)\\])", QRegularExpression::DontCaptureOption));
        ui->labelCountChars->setText(QString("%1 character%2").arg(QString::number(text.length()), text.length() != 1 ? "s" : ""));
    }
}
