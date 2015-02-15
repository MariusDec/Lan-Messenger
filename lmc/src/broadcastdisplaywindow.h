#ifndef BROADCASTDISPLAYWINDOW_H
#define BROADCASTDISPLAYWINDOW_H

#include "shared.h"
#include "xmlmessage.h"

#include <QDialog>

namespace Ui {
class broadcastDisplayWindow;
}

class broadcastDisplayWindow : public QDialog
{
    Q_OBJECT

    User *_localUser = nullptr;
    QString _peerId;
    XmlMessage _xmlMessage;
    QString _message;

public:
    explicit broadcastDisplayWindow(User *localUser, QWidget *parent = 0);
    ~broadcastDisplayWindow();

    void init(const QString &peerId, const QString &peerName, XmlMessage &message);

signals:
    void chatStarting(QString lpszThreadId, XmlMessage message, QStringList contacts);

private:
    void setUIText();
    void reply();

    Ui::broadcastDisplayWindow *ui;
};

#endif // BROADCASTDISPLAYWINDOW_H
