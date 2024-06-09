#include "triggerLine.h"
#include "ui_triggerLine.h"

triggerLine::triggerLine(QString name, uint8_t channel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::triggerLine)
{
    ui->setupUi(this);

    ui->nameLabel->setText(name);
    ui->channelLabel->setText("Ch: "+ QString::number(channel,10));

    _channel = channel;
}

void triggerLine::setName(QString name)
{
    ui->nameLabel->setText(name);
}

triggerLine::~triggerLine()
{
    delete ui;
}

void triggerLine::on_triggerButton_clicked()
{
    emit trigger(_channel);
}

uint8_t triggerLine::channel() const
{
    return _channel;
}
