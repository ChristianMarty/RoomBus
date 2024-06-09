#include "eventSignalLine.h"
#include "ui_eventSignalLine.h"

eventSignalLine::eventSignalLine(eventSlot eventSlot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::eventSignalLine)
{
    ui->setupUi(this);

    update(eventSlot);
}

eventSignalLine::~eventSignalLine()
{
    delete ui;
}

void eventSignalLine::update(eventSlot eventSlot)
{
    ui->nameLabel->setText(eventSlot.name);
    ui->channelLabel->setText("Ch: "+ QString::number(eventSlot.channel,10));
    ui->timeoutLabel->setText("Timeout: "+ QString::number(eventSlot.timeout,10)+"ms");

    _channel = eventSlot.channel;
}

uint8_t eventSignalLine::channel() const
{
    return _channel;
}

void eventSignalLine::on_checkBox_stateChanged(int arg1)
{
    bool active = (arg1 == true);
    emit eventActive(_channel,active);
}
