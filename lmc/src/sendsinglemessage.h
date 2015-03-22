#ifndef SENDSINGLEMESSAGE_H
#define SENDSINGLEMESSAGE_H

#include <QDialog>

namespace Ui {
class SendSingleMessage;
}

class SendSingleMessage : public QDialog
{
    Q_OBJECT

public:
    explicit SendSingleMessage(QWidget *parent = 0);
    ~SendSingleMessage();

private:
    Ui::SendSingleMessage *ui;
};

#endif // SENDSINGLEMESSAGE_H
