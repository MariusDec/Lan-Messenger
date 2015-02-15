#include "broadcastdisplaywindow.h"
#include "ui_broadcastdisplaywindow.h"
#include "messagelog.h"
#include "settings.h"
#include "stdlocation.h"

broadcastDisplayWindow::broadcastDisplayWindow(User *localUser, QWidget *parent) :  QDialog(parent), _localUser(localUser),
    ui(new Ui::broadcastDisplayWindow)
{
    ui->setupUi(this);
    setProperty("isWindow", true);

    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);
}

broadcastDisplayWindow::~broadcastDisplayWindow()
{
    delete ui;
}

void broadcastDisplayWindow::init(const QString &peerId, const QString &peerName, XmlMessage &message)
{
    setUIText();

    _peerId = peerId;

    lmcSettings settings;
    bool trim = settings.value(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL).toBool();
    bool allowLinks =
        settings.value(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL).toBool();
    bool pathToLink =
        settings.value(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL).toBool();
    bool showSmiley = settings.value(IDS_EMOTICON, IDS_EMOTICON_VAL).toBool();

    _xmlMessage = message;
    _message = _xmlMessage.data(XN_BROADCAST);
    lmcMessageLog::decodeMessage(&_message, trim, allowLinks, pathToLink, showSmiley);

    setWindowTitle(QString("%1 sent you a broadcast").arg(peerName));
    ui->labelTitle->setText(QString("Broadcast received from <b>%1</b>:").arg(peerName));
    ui->labelMessage->setText(_message);

    XmlMessage xmlMessage = message.clone();
    SingleMessage messageItem(MT_Broadcast, peerId, peerName, xmlMessage, QString::null);

    QString roomId = _localUser->id;
    roomId.append(Helper::getUuid());
    QString savePath = QDir(StdLocation::getWritableCacheDir()).absoluteFilePath(QString("msg_%1.tmp").arg(roomId));

    lmcMessageLog::saveMessageLog(peerName, peerId, _localUser, QList<QString> () << peerName, QDateTime::currentDateTime(), messageItem, savePath);

    connect (ui->buttonClose, &ThemedButton::clicked, this, &broadcastDisplayWindow::close);
    connect (ui->buttonReply, &ThemedButton::clicked, this, &broadcastDisplayWindow::reply);

    adjustSize();
    show();
}

void broadcastDisplayWindow::setUIText()
{
    ui->retranslateUi(this);
}

void broadcastDisplayWindow::reply()
{
    QString chatRoomId = _localUser->id;
    chatRoomId.append(Helper::getUuid());

    emit chatStarting(chatRoomId, _xmlMessage, QStringList() << _peerId);
    close();
}
