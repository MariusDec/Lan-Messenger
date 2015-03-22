#include "sendsinglemessage.h"
#include "ui_sendsinglemessage.h"

SendSingleMessage::SendSingleMessage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendSingleMessage)
{
    ui->setupUi(this);
}

SendSingleMessage::~SendSingleMessage()
{
    delete ui;
}
