#ifndef BROADCASTDISPLAYWINDOW_H
#define BROADCASTDISPLAYWINDOW_H

#include "shared.h"
#include "xmlmessage.h"
#include "messagelog.h"
#include "imagepickeraction.h"

#include <QDialog>
#include <QFrame>
#include <QMenu>

namespace Ui {
class InstantMessageWindow;
}

class InstantMessageWindow : public QDialog
{
    Q_OBJECT

    User          *_localUser = nullptr;
    lmcMessageLog *_messageLog = nullptr;
    QString        _peerId;
    QString        _peerName;
    XmlMessage     _xmlMessage;
    QString        _message;
    MessageType    _messageType;

public:
    explicit InstantMessageWindow(User *localUser, QWidget *parent = 0);
    ~InstantMessageWindow();

    void init(const QString &peerId, const QString &peerName, MessageType messageType, const XmlMessage &message);
    void stop();
    void settingsChanged();
    void connectionChanged(bool connected);
    void peerConnectionChanged(bool connected);
    const QString &getPeerId() { return _peerId; }

protected:
    bool eventFilter(QObject* object, QEvent* event);
    void changeEvent(QEvent* event);
    void moveEvent(QMoveEvent *event);
    void closeEvent(QCloseEvent *event);

signals:
    void chatStarting(QString chatRoomId, XmlMessage message, QStringList contacts);
    void messageSent(MessageType type, QString userId, XmlMessage message);
    void closed();

private:
    void setUIText();
    void createSmileyMenu();
    void createEmojiMenu();
    QFrame *createSeparator(QWidget *parent);
    void createToolbar();
    void openInChat();
    void insertSmileyCode(const ImagesStruct &smiley);
    void sendMessage();
    QString getWindowTitle();
    void insertHtmlTag(const QString &beginTag, const QString &endTag);

    void buttonFont_clicked();
    void buttonFontColor_clicked();
    void smileyAction_triggered();
    void emojiAction_triggered();
    void setMessageFont(QFont &font);
    void textChanged();

    Ui::InstantMessageWindow *ui;
    QMenu                    *_smileyMenu = nullptr;
    QMenu                    *_emojiMenu = nullptr;
    lmcImagePickerAction     *_smileyAction = nullptr;
    lmcImagePickerAction     *_emojiAction = nullptr;
    int                      _nSmiley = -1;
    int                      _nEmoji = -1;
    QColor                   _messageColor;
    QString                  _historySavePath;
    bool                     _peerIsOffline = false;
};

#endif // BROADCASTDISPLAYWINDOW_H
